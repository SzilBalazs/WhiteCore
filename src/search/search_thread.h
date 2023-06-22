// WhiteCore is a C++ chess engine
// Copyright (c) 2023 Balázs Szilágyi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "../core/board.h"
#include "../eval/eval.h"
#include "time_manager.h"
#include "move_list.h"
#include "tt.h"
#include "history.h"

#include <atomic>
#include <thread>

struct SharedMemory {
	TimeManager tm;
	TT tt;
	std::atomic<bool> is_searching;
	Move best_move;
	int64_t node_count;
};

class SearchThread {
public:

	SearchThread(SharedMemory &shared_memory, unsigned int thread_id) : shared(shared_memory), id(thread_id) {}

	void load_board(const Board &position) {
		board = position;
	}

	void join() {
		if (th.joinable())
			th.join();
	}

	void start() {
		th = std::thread([this](){search();});
	}

private:
	Board board;
	SharedMemory &shared;
	std::thread th;
	unsigned int id;
	Move pv_array[500][500];
	Ply pv_length[500];
	History history;

	std::string get_pv_line() {
		std::string pv;
		for (int i = 0; i < pv_length[0]; i++) {
			pv += pv_array[0][i].to_uci() + " ";
		}
		return pv;
	}


	void search() {
		history.clear(); // TODO remove this?
		for (Depth depth = 1; depth <= shared.tm.get_max_depth(); depth++) {
			Score score = search<ROOT_NODE>(depth, -INF_SCORE, INF_SCORE, 0);

			if (shared.is_searching && id == 0) {
				int64_t elapsed_time = shared.tm.get_elapsed_time();
				logger.print("info", "depth", int(depth), "nodes", shared.node_count,
						"score", "cp", score, "time", elapsed_time,
						"nps", calculate_nps(elapsed_time, shared.node_count),
						"pv", get_pv_line());
				shared.best_move = pv_array[0][0];
			}

			if (!shared.is_searching) {
				break;
			}
		}
		if (id == 0) {
			shared.is_searching = false;
			logger.print("bestmove", shared.best_move);
		}
	}

	void manage_resources() {
		if (!(shared.tm.time_left() && shared.node_count < shared.tm.get_max_nodes())) {
			shared.is_searching = false;
		}
	}

	template<NodeType node_type>
	Score search(Depth depth, Score alpha, Score beta, Ply ply) {
		constexpr bool root_node = node_type == ROOT_NODE;
		constexpr bool non_root_node = !root_node;

		const Score mate_ply = -MATE_VALUE + ply;
		const bool in_check = bool(Movegen::get_attackers(board, board.pieces<KING>(board.get_stm()).lsb()));

		Move best_move = NULL_MOVE;
		Score best_score = -INF_SCORE;

		if (id == 0)
			pv_length[ply] = ply;

		if ((shared.node_count & 1023) == 0 && !shared.tm.time_left()) {
			manage_resources();
		}

		if (!shared.is_searching) {
			return UNKNOWN_SCORE;
		}

		if (non_root_node) {
			if (board.is_draw()) {
				return 0;
			}
		}

		if (in_check) depth++;

		bool hit = false;
		TTEntry entry = shared.tt.probe(board.get_hash(), hit);
		TTFlag flag = TT_ALPHA;

		if (depth <= 0)
			return qsearch<node_type>(alpha, beta);

		MoveList<false> move_list(board, entry.hash_move, history, ply);

		if (move_list.empty()) {
			return in_check ? mate_ply : 0;
		}

		bool pv_next = true;
		while (!move_list.empty()) {
			Move move = move_list.next_move();

			shared.node_count++;
			board.make_move(move);
			Score score;

			if (pv_next) {
				score = -search<PV_NODE>(depth - 1, -beta, -alpha, ply + 1);
			} else {
				score = -search<NON_PV_NODE>(depth - 1, -alpha-1, -alpha, ply + 1);

				if (score > alpha) {
					score = -search<PV_NODE>(depth - 1, -beta, -alpha, ply + 1);
				}
			}

			board.undo_move(move);

			if (!shared.is_searching) {
				return UNKNOWN_SCORE;
			}

			if (score >= beta) {

				if (move.is_quiet()) {
					history.killer_moves[ply][1] = history.killer_moves[ply][0];
					history.killer_moves[ply][0] = move;
				}

				shared.tt.save(board.get_hash(), depth, beta, TT_BETA, move);
				return beta;
			}

			if (score > best_score) {
				best_score = score;
				best_move = move;
				flag = TT_EXACT;

				if (id == 0) {
					pv_array[ply][ply] = move;
					for (Ply i = ply + 1; i < pv_length[ply + 1]; i++) {
						pv_array[ply][i] = pv_array[ply + 1][i];
					}
					pv_length[ply] = pv_length[ply + 1];
				}


				if (score > alpha) {
					alpha = score;
					pv_next = false;
				}
			}
		}

		shared.tt.save(board.get_hash(), depth, best_score, flag, best_move);
		return alpha;
	}

	template<NodeType node_type>
	Score qsearch(Score alpha, Score beta) {

		if (!shared.is_searching) {
			return UNKNOWN_SCORE;
		}

		MoveList<true> move_list(board, NULL_MOVE, history, 0);

		Score static_eval = eval(board);

		if (static_eval >= beta)
			return beta;
		if (static_eval > alpha)
			alpha = static_eval;

		while (!move_list.empty()) {
			Move move = move_list.next_move();

			shared.node_count++;
			board.make_move(move);
			Score score = -qsearch<node_type>(-beta, -alpha);
			board.undo_move(move);

			if (score == UNKNOWN_SCORE) {
				return UNKNOWN_SCORE;
			}

			if (score >= beta)
				return beta;
			if (score > alpha) {
				alpha = score;
			}
		}

		return alpha;
	}
};
