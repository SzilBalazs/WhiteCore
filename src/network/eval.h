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

#pragma once

#include "../chess/board.h"
#include "nnue.h"

namespace eval {

    Score evaluate(const chess::Board &board, nn::NNUE &nnue) {
        const int piece_count = board.occupied().pop_count();

        if (piece_count == 2) {
            return 0;
        } else if (piece_count == 3) {

            const Piece piece = board.piece_at((board.occupied() ^ board.pieces<KING>()).lsb());

            if (piece.type == KNIGHT || piece.type == BISHOP) {
                return 0;
            }
        }
        chess::Bitboard x = threats::get_threats(board) & board.sides(color_enemy(board.get_stm()));
        return nnue.evaluate(board.get_stm(), x) + 20 * (piece_count >= 7);
    }
} // namespace eval
