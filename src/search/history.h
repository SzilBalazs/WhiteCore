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

#include "../core/move.h"

namespace search {
    struct History {
        core::Move killer_moves[MAX_PLY + 10][2];
        Score butterfly[64][64];

        void add_cutoff(core::Move move, Depth depth, Ply ply) {
            killer_moves[ply][1] = killer_moves[ply][0];
            killer_moves[ply][0] = move;
            butterfly[move.get_from()][move.get_to()] += 100 * depth;
        }

        void decrease_history(core::Move move, Depth depth) {
            butterfly[move.get_from()][move.get_to()] -= 100 * depth;
        }

        void clear() {
            for (int i = 0; i < MAX_PLY + 2; i++) {
                killer_moves[i][0] = killer_moves[i][1] = core::NULL_MOVE;
            }
            for (int i = 0; i < 64; i++) {
                for (int j = 0; j < 64; j++) {
                    butterfly[i][j] = 0;
                }
            }
        }
    };
} // namespace search