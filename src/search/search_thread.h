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
#include "../network/eval.h"
#include "history.h"
#include "move_list.h"
#include "time_manager.h"
#include "tt.h"

#include <atomic>
#include <thread>

namespace search {

    extern Depth lmr_reductions[200][MAX_PLY + 1];

    inline void init_lmr() {
        for (int made_moves = 0; made_moves < 200; made_moves++) {
            for (Depth depth = 0; depth < MAX_PLY + 1; depth++) {
                lmr_reductions[made_moves][depth] = 1.0 + std::log(made_moves) * std::log(depth) / 2.0;
            }
        }
    }

    struct SharedMemory {
        TimeManager tm;
        TT tt;
        std::atomic<bool> is_searching;
        bool uci_mode = true;
        core::Move best_move;
        Score eval;
        int64_t node_count;
    };

    struct SearchStack {
        Ply ply;
        core::Move move;
        Score eval;
    };

    class SearchThread {
    public:
        SearchThread(SharedMemory &shared_memory, unsigned int thread_id) : shared(shared_memory), id(thread_id) {}

        void load_board(const core::Board &position) {
            board = position;
        }

        void join() {
            if (th.joinable())
                th.join();
        }

        void start() {
            th = std::thread([this]() { search(); });
        }

    private:
        core::Board board;
        SharedMemory &shared;
        std::thread th;
        unsigned int id;
        core::Move pv_array[500][500];
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
            shared.best_move = core::NULL_MOVE;
            Score prev_score = 0;
            for (Depth depth = 1; depth <= shared.tm.get_max_depth(); depth++) {
                Score score = prev_score = aspiration_window(depth, prev_score);

                if (shared.is_searching && id == 0) {
                    int64_t elapsed_time = shared.tm.get_elapsed_time();

                    if (shared.uci_mode) {
                        logger.print("info", "depth", int(depth), "nodes", shared.node_count,
                                     "score", "cp", score, "time", elapsed_time,
                                     "nps", calculate_nps(elapsed_time, shared.node_count),
                                     "pv", get_pv_line());
                    }

                    shared.best_move = pv_array[0][0];
                    shared.eval = score;
                }

                if (!shared.is_searching) {
                    break;
                }
            }
            if (id == 0) {
                shared.is_searching = false;
                if (shared.uci_mode) {
                    logger.print("bestmove", shared.best_move);
                }
            }
        }

        Score aspiration_window(Depth depth, Score prev_score) {

            static constexpr Score DELTA = 30;
            static constexpr Score BOUND = 1500;

            Score delta = DELTA;
            Score alpha = -INF_SCORE;
            Score beta = INF_SCORE;

            if (depth >= 6) {
                alpha = prev_score - delta;
                beta = prev_score + delta;
            }

            SearchStack stack[MAX_PLY + 10];
            SearchStack *ss = stack + 7;
            for (Ply i = -7; i <= MAX_PLY + 2; i++) {
                (ss + i)->move = core::NULL_MOVE;
                (ss + i)->eval = UNKNOWN_SCORE;
                (ss + i)->ply = i;
            }

            while (true) {
                if (!shared.is_searching) {
                    break;
                }

                if (alpha <= -BOUND) alpha = -INF_SCORE;
                if (beta >= BOUND) beta = INF_SCORE;

                Score score = search<ROOT_NODE>(depth, alpha, beta, ss);

                if (score <= alpha) {
                    beta = (alpha + beta) / 2;
                    alpha = std::max(-BOUND, score - delta);
                } else if (score >= beta) {
                    beta = std::min(BOUND, score + delta);
                } else {
                    return score;
                }

                delta += delta / 2;
            }

            return UNKNOWN_SCORE;
        }

        void manage_resources() {
            if (shared.best_move != core::NULL_MOVE && !(shared.tm.time_left() && shared.node_count < shared.tm.get_max_nodes())) {
                shared.is_searching = false;
            }
        }

        template<NodeType node_type>
        Score search(Depth depth, Score alpha, Score beta, SearchStack *ss) {
            constexpr bool root_node = node_type == ROOT_NODE;
            constexpr bool non_root_node = !root_node;
            constexpr bool pv_node = node_type != NON_PV_NODE;
            constexpr bool non_pv_node = !pv_node;

            const Score mate_ply = -MATE_VALUE + ss->ply;
            const bool in_check = board.is_check();

            core::Move best_move = core::NULL_MOVE;
            Score best_score = -INF_SCORE;

            if (id == 0)
                pv_length[ss->ply] = ss->ply;

            if ((shared.node_count & 1023) == 0) {
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

            std::optional<TTEntry> entry = shared.tt.probe(board.get_hash());
            TTFlag flag = TT_ALPHA;
            core::Move hash_move = entry ? entry->hash_move : core::NULL_MOVE;

            if (depth <= 0)
                return qsearch<node_type>(alpha, beta);

            Score static_eval = ss->eval = nn::eval(board);

            if (root_node || in_check)
                goto search_moves;

            if (!entry && depth >= 5)
                depth--;

            if (non_pv_node && depth <= 6 && static_eval - depth * 100 >= beta && std::abs(beta) < WORST_MATE)
                return static_eval;

            if (non_pv_node && depth >= 3 && static_eval >= beta && board.has_non_pawn()) {
                Depth R = 3 + std::min(3, depth / 4);
                ss->move = core::NULL_MOVE;

                board.make_null_move();
                Score score = -search<NON_PV_NODE>(depth - R, -beta, -beta + 1, ss + 1);
                board.undo_null_move();

                if (score >= beta) {
                    if (std::abs(score) > WORST_MATE)
                        return beta;
                    return score;
                }
            }

        search_moves:
            MoveList<false> move_list(board, hash_move, history, ss->ply);

            if (move_list.empty()) {
                return in_check ? mate_ply : 0;
            }

            bool skip_quiets = false;
            int made_moves = 0;
            while (!move_list.empty()) {
                core::Move move = ss->move = move_list.next_move();

                if (skip_quiets && move.is_quiet() && !move.is_promo()) continue;

                if (non_root_node && non_pv_node && !in_check && depth <= 5 && made_moves >= 5 + depth * depth) {
                    skip_quiets = true;
                }

                shared.node_count++;
                board.make_move(move);
                Score score;

                if (!in_check && depth >= 4 && made_moves >= 4 && !move.is_promo() && move.is_quiet()) {
                    Depth R = lmr_reductions[depth][made_moves];
                    Depth new_depth = std::clamp(depth - R, 1, depth - 1);
                    score = -search<NON_PV_NODE>(new_depth, -alpha - 1, -alpha, ss + 1);

                    if (score > alpha && R > 0) {
                        score = -search<NON_PV_NODE>(depth - 1, -alpha - 1, -alpha, ss + 1);
                    }
                } else if (non_pv_node || made_moves != 0) {
                    score = -search<NON_PV_NODE>(depth - 1, -alpha - 1, -alpha, ss + 1);
                }

                if (pv_node && (made_moves == 0 || (alpha < score && score < beta))) {
                    score = -search<PV_NODE>(depth - 1, -beta, -alpha, ss + 1);
                }

                board.undo_move(move);

                if (!shared.is_searching) {
                    return UNKNOWN_SCORE;
                }

                if (score >= beta) {

                    if (move.is_quiet()) {
                        history.add_cutoff(move, depth, ss->ply);
                    }

                    shared.tt.save(board.get_hash(), depth, beta, TT_BETA, move);
                    return beta;
                }

                if (score > best_score) {
                    best_score = score;
                    best_move = move;
                    flag = TT_EXACT;

                    if (id == 0) {
                        pv_array[ss->ply][ss->ply] = move;
                        for (Ply i = ss->ply + 1; i < pv_length[ss->ply + 1]; i++) {
                            pv_array[ss->ply][i] = pv_array[ss->ply + 1][i];
                        }
                        pv_length[ss->ply] = pv_length[ss->ply + 1];
                    }


                    if (score > alpha) {
                        alpha = score;
                    }
                }

                made_moves++;
            }

            shared.tt.save(board.get_hash(), depth, best_score, flag, best_move);
            return alpha;
        }

        template<NodeType node_type>
        Score qsearch(Score alpha, Score beta) {

            if (!shared.is_searching) {
                return UNKNOWN_SCORE;
            }

            MoveList<true> move_list(board, core::NULL_MOVE, history, 0);

            Score static_eval = nn::eval(board);

            if (static_eval >= beta)
                return beta;
            if (static_eval > alpha)
                alpha = static_eval;

            while (!move_list.empty()) {
                core::Move move = move_list.next_move();

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
} // namespace search