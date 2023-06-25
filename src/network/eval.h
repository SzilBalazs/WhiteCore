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

#include "../core/attacks.h"
#include "../core/board.h"
#include "../core/constants.h"

Score eval(const core::Board &board) {
    Score score = 0;
    score += 100 * (board.pieces<WHITE, PAWN>().pop_count() - board.pieces<BLACK, PAWN>().pop_count());
    score += 300 * (board.pieces<WHITE, KNIGHT>().pop_count() - board.pieces<BLACK, KNIGHT>().pop_count());
    score += 350 * (board.pieces<WHITE, BISHOP>().pop_count() - board.pieces<BLACK, BISHOP>().pop_count());
    score += 500 * (board.pieces<WHITE, ROOK>().pop_count() - board.pieces<BLACK, ROOK>().pop_count());
    score += 900 * (board.pieces<WHITE, QUEEN>().pop_count() - board.pieces<BLACK, QUEEN>().pop_count());

    for (Color color : {WHITE, BLACK}) {
        core::Bitboard occ = board.pieces(color) | board.pieces(color_enemy(color), PAWN);
        Score mobility = 0;
        for (PieceType pt : {KNIGHT, BISHOP, ROOK, QUEEN}) {
            core::Bitboard bb = board.pieces(color, pt);
            while (bb) {
                Square sq = bb.pop_lsb();
                mobility += attacks_piece(pt, sq, occ).pop_count();
            }
        }
        if (color == WHITE) score += mobility;
        else
            score -= mobility;
    }

    if (board.get_stm() == WHITE)
        return score;
    else
        return -score;
}
