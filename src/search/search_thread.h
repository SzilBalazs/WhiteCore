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
#include "../network/nnue.h"
#include "history.h"
#include "move_list.h"
#include "pv_array.h"
#include "terminal_report.h"
#include "time_manager.h"
#include "tt.h"

#include <atomic>
#include <thread>

namespace search {

    extern Depth lmr_reductions[200][MAX_PLY + 1];

    void init_lmr() {
        for (int made_moves = 0; made_moves < 200; made_moves++) {
            for (Depth depth = 0; depth < MAX_PLY + 1; depth++) {
                double moves_log = made_moves == 0 ? 0 : std::log(made_moves);
                double depth_log = depth == 0 ? 0 : std::log(depth);
                lmr_reductions[made_moves][depth] = 1.0 + moves_log * depth_log / 2.0;
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
        core::Move excluded_move;
        Score eval;
    };

    class SearchThread {
    public:
        SearchThread(SharedMemory &shared_memory, unsigned int thread_id) : nnue(), shared(shared_memory), id(thread_id) {}

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
        nn::NNUE nnue;
        SharedMemory &shared;
        std::thread th;
        unsigned int id;
        Ply max_ply;
        PVArray pv;

        History history;

        void search() {
            init_search();
            iterative_deepening();
            finish_search();
        }

        void init_search() {
            history.clear();
            shared.best_move = core::NULL_MOVE;
            max_ply = 0;
        }

        void iterative_deepening() {
            Score prev_score = 0;
            for (Depth depth = 1; depth <= shared.tm.get_max_depth() && shared.is_searching; depth++) {
                Score score = prev_score = aspiration_window(depth, prev_score);

                handle_iteration(score, depth);
            }
        }

        void handle_iteration(Score score, Depth depth) {
            if (shared.is_searching && id == 0) {
                handle_uci(score, depth);
                shared.best_move = pv.get_best_move();
                shared.eval = score;
            }
        }

        void handle_uci(Score score, Depth depth) {
            if (shared.uci_mode) {
                int64_t elapsed_time = shared.tm.get_elapsed_time();

                report::print_iteration(depth, max_ply, shared.node_count, score, elapsed_time,
                                        calculate_nps(elapsed_time, shared.node_count), pv.get_line());
            }
        }

        void finish_search() {
            if (id == 0) {
                shared.is_searching = false;
                if (shared.uci_mode) {
                    report::print_bestmove(shared.best_move);
                }
            }
        }

        Score aspiration_window(Depth depth, Score prev_score) {

            static constexpr Score DELTA = 20;
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

                nnue.refresh(board.to_features());
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

            const core::Move last_move = ss->ply >= 1 ? (ss - 1)->move : core::NULL_MOVE;
            const Score mate_ply = -MATE_VALUE + ss->ply;
            const bool in_check = board.is_check();
            const bool is_singular_root = ss->excluded_move.is_ok();

            core::Move best_move = core::NULL_MOVE;
            Score best_score = -INF_SCORE;

            if (id == 0) {
                pv.length[ss->ply] = ss->ply;
                max_ply = std::max(max_ply, ss->ply);
            }

            if (id == 0 && (shared.node_count & 2047) == 0) {
                manage_resources();
            }

            if (!shared.is_searching) {
                return UNKNOWN_SCORE;
            }

            if (non_root_node && board.is_draw()) {
                return 0;
            }

            std::optional<TTEntry> entry = is_singular_root ? std::nullopt : shared.tt.probe(board.get_hash());
            TTFlag flag = TT_ALPHA;
            core::Move hash_move = entry ? entry->hash_move : core::NULL_MOVE;

            if (entry && non_pv_node && entry->depth >= depth && board.get_move50() < 90 &&
                (entry->flag == TT_EXACT || (entry->flag == TT_ALPHA && entry->eval <= alpha) || (entry->flag == TT_BETA && entry->eval >= beta))) {
                return entry->eval;
            }

            if (depth <= 0)
                return qsearch<node_type>(alpha, beta);

            Score static_eval = ss->eval = nnue.evaluate(board.get_stm());
            bool improving = ss->ply >= 2 && ss->eval >= (ss - 2)->eval;

            if (root_node || in_check || is_singular_root)
                goto search_moves;

            if (!entry && non_pv_node && depth >= 4)
                depth--;

            if (depth <= 3 && static_eval + 150 * depth <= alpha) {
                Score score = qsearch<NON_PV_NODE>(alpha, beta);
                if (score <= alpha)
                    return score;
            }

            if (non_pv_node && depth <= 6 && static_eval - (depth - improving) * 70 >= beta && std::abs(beta) < WORST_MATE)
                return static_eval;

            if (non_pv_node && depth >= 3 && static_eval >= beta && board.has_non_pawn()) {
                Depth R = 3 + depth / 3 + std::min(3, (static_eval - beta) / 256);
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
            MoveList<false> move_list(board, hash_move, last_move, history, ss->ply);

            if (move_list.empty()) {
                if (is_singular_root) return alpha;
                else return in_check ? mate_ply : 0;
            }

            history.killer_moves[ss->ply + 1][0] = history.killer_moves[ss->ply + 1][1] = core::NULL_MOVE;

            core::Move quiet_moves[200];
            core::Move *next_quiet_move = quiet_moves;

            bool skip_quiets = false;
            int made_moves = 0;
            while (!move_list.empty()) {
                core::Move move = ss->move = move_list.next_move();

                if ((skip_quiets && move.is_quiet() && !move.is_promo()) || (ss->excluded_move == move)) continue;

                if (non_root_node && non_pv_node && !in_check) {
                    if (depth <= 5 && made_moves >= 5 + depth * depth) {
                        skip_quiets = true;
                    }

                    if (depth <= 5 && static_eval + 33 + depth * 53 + improving * 71 < alpha) {
                        skip_quiets = true;
                    }
                }

                Depth extensions = 0;

                if (in_check) {
                    extensions = 1;
                } else if (non_root_node && depth >= 7 && move == hash_move &&
                           entry->flag == TT_BETA && entry->depth >= depth &&
                           std::abs(entry->eval) < WORST_MATE) {

                    Score singular_beta = entry->eval - depth * 3;
                    Depth singular_depth = (depth - 1) / 2;

                    ss->excluded_move = move;
                    Score score = search<NON_PV_NODE>(singular_depth, singular_beta - 1, singular_beta, ss);
                    ss->excluded_move = core::NULL_MOVE;

                    if (score < singular_beta) {
                        extensions = 1;
                    } else if (singular_beta >= beta) {
                        return singular_beta;
                    }
                }

                Depth new_depth = depth - 1 + extensions;

                shared.node_count++;
                board.make_move(move, &nnue);
                Score score;

                shared.tt.prefetch(board.get_hash());

                if (!in_check && depth >= 3 && made_moves >= 4 && !move.is_promo() && move.is_quiet()) {
                    Depth R = lmr_reductions[depth][made_moves];

                    R -= pv_node;
                    R += !improving;

                    Depth D = std::clamp(new_depth - R, 1, depth - 1);
                    score = -search<NON_PV_NODE>(D, -alpha - 1, -alpha, ss + 1);

                    if (score > alpha && R > 0) {
                        score = -search<NON_PV_NODE>(new_depth, -alpha - 1, -alpha, ss + 1);
                    }
                } else if (non_pv_node || made_moves != 0) {
                    score = -search<NON_PV_NODE>(new_depth, -alpha - 1, -alpha, ss + 1);
                }

                if (pv_node && (made_moves == 0 || (alpha < score && score < beta))) {
                    score = -search<PV_NODE>(new_depth, -beta, -alpha, ss + 1);
                }

                board.undo_move(move, &nnue);

                if (!shared.is_searching) {
                    return UNKNOWN_SCORE;
                }

                if (score >= beta) {

                    if (!is_singular_root) {
                        if (move.is_quiet()) {
                            history.add_cutoff(move, last_move, depth, ss->ply);
                            for (core::Move *current_move = quiet_moves; current_move != next_quiet_move; current_move++) {
                                history.decrease_history(*current_move, depth);
                            }
                        }

                        shared.tt.save(board.get_hash(), depth, beta, TT_BETA, move);
                    }

                    return beta;
                }

                if (score > best_score) {
                    best_score = score;
                    best_move = move;

                    if (id == 0) {
                        pv.update(ss->ply, move);
                    }

                    if (score > alpha) {
                        flag = TT_EXACT;
                        alpha = score;
                    }
                }

                made_moves++;
                if (move.is_quiet()) *next_quiet_move++ = move;
            }

            if (!is_singular_root) {
                shared.tt.save(board.get_hash(), depth, best_score, flag, best_move);
            }
            return alpha;
        }

        template<NodeType node_type>
        Score qsearch(Score alpha, Score beta) {

            if (!shared.is_searching) {
                return UNKNOWN_SCORE;
            }

            Score static_eval = nnue.evaluate(board.get_stm());

            if (static_eval >= beta)
                return beta;
            if (static_eval > alpha)
                alpha = static_eval;

            MoveList<true> move_list(board, core::NULL_MOVE, core::NULL_MOVE, history, 0);

            while (!move_list.empty()) {
                core::Move move = move_list.next_move();

                if (alpha > -WORST_MATE && !see(board, move, 0)) continue;

                shared.node_count++;
                board.make_move(move, &nnue);
                Score score = -qsearch<node_type>(-beta, -alpha);
                board.undo_move(move, &nnue);

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