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

#pragma once

#include "../chess/move.h"

namespace search {

    struct SearchStack {
        Ply ply;
        chess::Move move;
        PieceType pt;
        Score eval;
    };

    class History {

    public:
        chess::Move killer_moves[MAX_PLY + 10][2];
        chess::Move counter_moves[64][64];
        Score butterfly[64][64];
        Score conthist[6][64][64][64];

        /**
         * Adds a beta-cutoff to the History.
         *
         * @param move Move played
         * @param last_move Move played last turn
         * @param depth Search depth
         * @param ply Distance from root
         */
        void add_cutoff(chess::Move move, Depth depth, SearchStack *ss) {
            update_killer_moves(move, ss->ply);
            update_history(butterfly[move.get_from()][move.get_to()], depth * 100);

            if ((ss - 1)->move.is_ok()) {
                update_counter_moves(move, (ss - 1)->move);
                update_history(conthist[(ss - 1)->pt][(ss - 1)->move.get_to()][move.get_from()][move.get_to()], depth * 100);
            }
        }

        /**
         * Decreases history of a weak move.
         *
         * @param move The weak move
         * @param depth Search depth
         */
        void decrease_history(chess::Move move, Depth depth, SearchStack *ss) {
            update_history(butterfly[move.get_from()][move.get_to()], -depth * 100);

            if ((ss - 1)->move.is_ok()) {
                update_history(conthist[(ss - 1)->pt][(ss - 1)->move.get_to()][move.get_from()][move.get_to()], -depth * 100);
            }
        }

        Score get_history(chess::Move move, SearchStack *ss) const {
            Score value = butterfly[move.get_from()][move.get_to()];
            if ((ss - 1)->move.is_ok()) {
                value += 2 * conthist[(ss - 1)->pt][(ss - 1)->move.get_to()][move.get_from()][move.get_to()];
            }
            return value;
        }

        void decay() {
            for (int i = 0; i < 64; i++) {
                for (int j = 0; j < 64; j++) {
                    for (int k = 0; k < 64; k++) {
                        for (int pt = 0; pt < 6; pt++) {
                            conthist[pt][i][j][k] /= 2;
                        }
                    }
                    butterfly[i][j] /= 2;
                }
            }
        }

        /**
         * Clears the history.
         */
        void clear() {
            for (int i = 0; i < MAX_PLY + 2; i++) {
                killer_moves[i][0] = killer_moves[i][1] = chess::NULL_MOVE;
            }
            for (int i = 0; i < 64; i++) {
                for (int j = 0; j < 64; j++) {
                    for (int k = 0; k < 64; k++) {
                        for (int pt = 0; pt < 6; pt++) {
                            conthist[pt][i][j][k] = 0;
                        }
                    }

                    butterfly[i][j] = 0;
                    counter_moves[i][j] = chess::NULL_MOVE;
                }
            }
        }

    private:
        void update_killer_moves(chess::Move move, Ply ply) {
            killer_moves[ply][1] = killer_moves[ply][0];
            killer_moves[ply][0] = move;
        }

        void update_counter_moves(chess::Move move, chess::Move last_move) {
            counter_moves[last_move.get_from()][last_move.get_to()] = move;
        }

        static void update_history(Score &entry, Score bonus) {
            int scaled = bonus - entry * std::abs(bonus) / 32768;
            entry += scaled;
        }
    };
} // namespace search