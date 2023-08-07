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

#include "nnue.h"
#include "../core/board.h"

namespace eval {

    Score get_material_scale(const core::Board &board) {
        Score material_value = 0;
        for (PieceType pt : {PAWN, BISHOP, KNIGHT, ROOK, QUEEN}) {
            material_value += board.pieces(pt).pop_count() * PIECE_VALUES[material_value];
        }

        return 700 + material_value / 32 - board.get_move50() * 5;
    }

    Score evaluate(nn::NNUE &nnue, const core::Board &board) {
        Score base_eval = nnue.forward(board.get_stm());
        Score material_scale = get_material_scale(board);
        return base_eval * material_scale / 1024;
    }
}