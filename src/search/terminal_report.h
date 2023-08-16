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

#include <iostream>
#include <sstream>

#include "../chess/constants.h"
#include "wdl_model.h"

#pragma once

namespace search::report {

    const std::string ASCII_RESET_COLOR = "\u001b[0m";

    bool pretty_output = false;
    bool show_wdl = false;

    /**
     * @brief Switches the output mode of the search results.
     *
     * @param x Boolean variable to decide if output should be made pretty.
     */
    void set_pretty_output(bool x) {
        pretty_output = x;
    }

    void set_show_wdl(bool x) {
        show_wdl = x;
    }

    /**
     * @brief Converts an integer to its corresponding ASCII color code
     *
     * @param a integer value of the color to be returned
     * @return A string representing the color in ASCII
     */
    std::string get_ascii_color(int a) {
        return "\u001b[38;5;" + std::to_string(a) + "m";
    }

    std::string score_color(Score score) {
        if (WORST_MATE < std::abs(score))
            return get_ascii_color(207);
        else if (300 <= score)
            return get_ascii_color(45);
        else if (score <= -300)
            return get_ascii_color(196);
        else if (10 <= score)
            return get_ascii_color(42);
        else if (score <= -10)
            return get_ascii_color(9);
        else
            return get_ascii_color(255);
    }

    std::string pretty_int(uint64_t n) {
        std::string str;
        if (n < 1000) {
            str = std::to_string(n);
        } else if (n < 10000 * 1000) {
            str = std::to_string(uint64_t(n / 1000)) + "K";
        } else {
            str = std::to_string(uint64_t(n / (1000 * 1000))) + "M";
        }
        return str;
    }

    std::string pretty_milli(uint64_t milli) {
        if (milli < 1000) {
            return std::to_string(milli) + "ms";
        } else if (milli < (1000 * 60)) {
            return std::to_string(milli / 1000) + "s";
        }
        return std::to_string(milli / (1000 * 60)) + "m";
    }

    std::string pretty_score(Score score) {

        std::stringstream res;

        res << (score >= 0 ? "+" : "-");

        Score abs_score = std::abs(score);
        int mate_depth = MATE_VALUE - abs_score;
        if (mate_depth <= MAX_PLY) {
            res << "M" << mate_depth;
        } else {
            float pawn_score = float(std::abs(score)) / 100.0f;

            res << std::fixed << std::setprecision(2) << pawn_score;
        }

        return res.str();
    }

    std::string pretty_wdl(Score score) {
        std::stringstream res;
        auto [w, l] = wdl_model::cp_to_wl(score);
        int d = 1000 - w - l;

        res << "(W" << w / 10 << "% D" << d / 10 << "% L" << l / 10 << "%)";

        return res.str();
    }

    std::string pretty_pv(const chess::Board &board, const std::string &pv, const std::string &line_color) {
        std::stringstream in(pv);
        std::stringstream res;
        res << get_ascii_color(87);

        chess::Board tmp = board;

        int cnt = 0;
        std::string move;
        while (getline(in, move, ' ')) {

            cnt++;

            chess::Move uci_move = chess::move_from_string(tmp, move);
            if (!uci_move.is_ok()) {
                throw std::runtime_error("Invalid PV");
            }

            std::stringstream san_move;

            if (uci_move.eq_flag(chess::Move::KING_CASTLE)) {
                san_move << "O-O";
            } else if (uci_move.eq_flag(chess::Move::QUEEN_CASTLE)) {
                san_move << "O-O-O";
            } else {
                Piece piece = tmp.piece_at(uci_move.get_from());

                if (piece.type != PAWN) {
                    san_move << char_from_piece(Piece(piece.type, WHITE));
                } else if (uci_move.is_capture()) {
                    san_move << char('a' + square_to_file(uci_move.get_from()));
                }

                if (uci_move.is_capture()) {
                    san_move << "x";
                }

                san_move << format_square(uci_move.get_to());

                if (uci_move.is_promo()) {
                    san_move << "=" << char_from_piece(Piece(uci_move.get_promo_type(), WHITE));
                }
            }

            tmp.make_move(uci_move);

            chess::Move buffer[200];
            chess::Move *end_ptr = chess::gen_moves(tmp, buffer, false);
            size_t move_count = end_ptr - buffer;

            if (move_count == 0) {
                if (tmp.is_check()) {
                    san_move << "# ";
                    if (tmp.get_stm() == WHITE) {
                        san_move << "1-0";
                    } else {
                        san_move << "0-1";
                    }
                } else {
                    san_move << " 1/2-1/2";
                }
            } else {
                if (tmp.is_check()) {
                    san_move << "+";
                }
                if (tmp.is_draw()) {
                    san_move << " 1/2-1/2";
                }
            }


            if (cnt % 20 == 0) {
                res << ASCII_RESET_COLOR << "\n │         │           │                     │          │          │          │ " << line_color;
            }

            res << san_move.str() << " " << line_color;
        }
        return res.str();
    }

    std::string score_to_string(Score score) {
        Score abs_score = std::abs(score);
        int mate_depth = MATE_VALUE - abs_score;
        std::stringstream res;
        if (mate_depth <= MAX_PLY) {
            int mate_ply = score > 0 ? mate_depth / 2 + 1 : -(mate_depth / 2);
            res << "mate " << mate_ply;
        } else {
            res << "cp " << score;
        }

        if (show_wdl) {
            auto [w, l] = wdl_model::cp_to_wl(score);
            int d = 1000 - w - l;
            res << " wdl " << w << " " << d << " " << l;
        }

        return res.str();
    }

    /**
     * Prints the information for the current iteration.
     *
     * @param depth Depth of the iteration.
     * @param seldepth Selective depth of the iteration.
     * @param nodes Total number of nodes searched in the given iteration.
     * @param score Evaluation score of the root position.
     * @param time Amount of time spent so far, in milliseconds.
     * @param nps Nodes per second.
     * @param pv_line The principle variation line. A string of moves separated by spaces.
     */
    void print_iteration(const chess::Board &board, const int depth, const int seldepth, const uint64_t nodes, const Score score,
                         const uint64_t time, const uint64_t nps, const uint64_t hashfull, const std::string &pv_line) {

        if (pretty_output) {

            const std::string line_color = depth & 1 ? get_ascii_color(247) : get_ascii_color(251);

            std::stringstream res;
            std::stringstream ss_depth;

            if (depth == 1) {
                res << " ╭─────────┬───────────┬─────────────────────┬──────────┬──────────┬──────────┬─────────────────────────────────────────────────────────\n";
                res << " │  Depth  │   Score   │         WDL         │   Nodes  │    NPS   │   Time   │ Principal variation                                     \n";
                res << " ├─────────┼───────────┼─────────────────────┼──────────┼──────────┼──────────┼─────────────────────────────────────────────────────────\n";
            } /* else {
                res << " ├─────────┼───────────┼─────────────────────┼──────────┼──────────┼──────────┼─────────────────────────────────────────────────────────\n";
            }*/

            ss_depth << depth << "/" << seldepth;

            res << ASCII_RESET_COLOR << " │ "
                << line_color << std::setw(6) << ss_depth.str() << ASCII_RESET_COLOR << "  │"
                << score_color(score) << std::setw(9) << pretty_score(score) << ASCII_RESET_COLOR << "  │ "
                << line_color << std::setw(18) << pretty_wdl(score) << ASCII_RESET_COLOR << "  │ "
                << line_color << std::setw(7) << pretty_int(nodes) << ASCII_RESET_COLOR << "  │ "
                << line_color << std::setw(7) << pretty_int(nps) << ASCII_RESET_COLOR << "  │ "
                << line_color << std::setw(7) << pretty_milli(time) << ASCII_RESET_COLOR << "  │ "
                << pretty_pv(board, pv_line, line_color) << ASCII_RESET_COLOR << "\n";

            std::cout << "\r" << res.str() << std::flush;

        } else {

            std::stringstream ss;

            ss << "info depth " << depth << " seldepth " << seldepth << " nodes " << nodes
               << " score " << score_to_string(score) << " time " << time << " nps " << nps
               << " hashfull " << hashfull << " pv " << pv_line << "\n";


            std::cout << ss.str() << std::flush;
        }
    }

    /**
     * Prints the best move found in the search for a given position.
     *
     * @param bestmove The best move found by the search.
     */
    void print_bestmove(const chess::Move bestmove) {
        std::stringstream ss;
        ss << "bestmove " << bestmove << "\n";
        std::cout << ss.str() << std::flush;
    }

} // namespace search::report