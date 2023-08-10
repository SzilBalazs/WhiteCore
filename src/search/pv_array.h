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

#include "../chess/move.h"

namespace search {
    struct PVArray {

        chess::Move array[MAX_PLY][MAX_PLY];
        Ply length[MAX_PLY]{};

        /**
        * Returns line as a string of UCI moves separated by spaces.
        *
        * @return String of UCI moves.
        */
        [[nodiscard]] std::string get_line() const {
            std::stringstream line;
            for (int i = 0; i < length[0]; i++) {
                line << array[0][i].to_uci() << " ";
            }
            return line.str();
        }

        /**
        * Returns the best move in PVArray
        *
        * @return Best move
        */
        [[nodiscard]] chess::Move get_best_move() const {
            return array[0][0];
        }

        /**
        * Updates PVArray with new data.
        *
        * @param ply  The ply to update.
        * @param move The new move for the ply.
        */
        void update(Ply ply, chess::Move move) {
            array[ply][ply] = move;
            for (Ply i = ply + 1; i < length[ply + 1]; i++) {
                array[ply][i] = array[ply + 1][i];
            }
            length[ply] = length[ply + 1];
        }
    };
} // namespace search