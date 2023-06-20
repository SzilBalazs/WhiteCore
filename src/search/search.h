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
#include "../core/movegen.h"
#include "../eval/eval.h"

class Searcher {
public:
	void load_board(const Board &position) {
		board = position;
	}

	Move search(U64 allocated_nodes) {
		max_nodes = allocated_nodes;
		node_count = 0;
		Move bm;
		for (Depth depth = 1; depth <= MAX_PLY; depth++) {
			Score score = search<true>(depth, -INF_SCORE, INF_SCORE, 0);
			if (left_resources()) {
				bm = root_best_move;
				std::cout << "depth " << int(depth) << " nodes " << node_count << " score " << score << "cp " << bm << std::endl;
			} else {
				break;
			}
		}
		return bm;
	}

private:
	Board board;
	U64 max_nodes = 0;
	U64 node_count = 0;
	Move root_best_move;

	bool left_resources() const {
		return node_count <= max_nodes;
	}

	template<bool root_node>
	Score search(Depth depth, Score alpha, Score beta, Ply ply) {
		if (depth == 0)
			return qsearch(alpha, beta);

		if (!left_resources()) {
			return UNKNOWN_SCORE;
		}

		bool inCheck = bool(Movegen::get_attackers(board, board.pieces<KING>(board.get_stm()).lsb()));
		Move moves[200];
		Move *moves_end = Movegen::gen_moves(board, moves, false);

		if (moves == moves_end) {
			if (inCheck)
				return -MATE_VALUE + ply;
			else
				return 0;
		}

		Move best_move = NULL_MOVE;
		Score best_score = -INF_SCORE;

		for (Move *it = moves; it != moves_end; it++) {
			Move move = *it;

			node_count++;
			board.make_move(move);
			Score score = -search<false>(depth - 1, -beta, -alpha, ply + 1);
			board.undo_move(move);

			if (score == UNKNOWN_SCORE) {
				return UNKNOWN_SCORE;
			}

			if (score >= beta)
				return beta;
			if (score > best_score) {
				best_score = score;
				best_move = move;

				if (score > alpha) {
					alpha = score;
				}
			}
		}

		if constexpr (root_node) {
			root_best_move = best_move;
		}

		return alpha;
	}

	Score qsearch(Score alpha, Score beta) {
		node_count++;

		if (!left_resources()) {
			return UNKNOWN_SCORE;
		}

		Move moves[200];
		Move *moves_end = Movegen::gen_moves(board, moves, true);

		Score static_eval = eval(board);

		if (static_eval >= beta)
			return beta;
		if (static_eval > alpha)
			alpha = static_eval;

		for (Move *it = moves; it != moves_end; it++) {
			Move move = *it;

			node_count++;
			board.make_move(move);
			Score score = -qsearch(-beta, -alpha);
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
