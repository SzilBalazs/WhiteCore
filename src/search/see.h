// WhiteCore is a C++ chess engine
// Copyright (c) 2023-2024 Balázs Szilágyi
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

#include "../chess/board.h"

namespace search {

    /**
     * Determines if evaluation captures forced by a move exceeds a specific threshold.
     *
     * @param board The current board
     * @param move The move to evaluate
     * @param threshold The score threshold
     * @return true if the evaluated score exceeds the threshold
     * @return false otherwise
     */
    bool see(const chess::Board &board, chess::Move move, Score threshold) {

        Square from = move.get_from();
        Square to = move.get_to();

        if (move.is_promo() || move.eq_flag(chess::Move::EP_CAPTURE)) return true;

        Score value = PIECE_VALUES[board.piece_at(to).type] - threshold;

        if (value < 0) return false;

        value -= PIECE_VALUES[board.piece_at(from).type];

        if (value >= 0) return true;

        chess::Bitboard rooks = board.pieces<ROOK>() | board.pieces<QUEEN>();
        chess::Bitboard bishops = board.pieces<BISHOP>() | board.pieces<QUEEN>();
        chess::Bitboard occ = board.occupied() & (~chess::Bitboard(from)) & (~chess::Bitboard(to));

        chess::Bitboard attacker = from;
        chess::Bitboard attackers = chess::get_all_attackers(board, to, occ);

        Color stm = color_enemy(board.piece_at(from).color);

        while (true) {
            attackers &= occ;

            PieceType type;
            attacker = chess::least_valuable_piece(board, attackers, stm, type);

            if (!attacker)
                break;

            value = -value - 1 - PIECE_VALUES[type];
            stm = color_enemy(stm);

            if (value >= 0) {
                if (type == KING && (attackers & board.sides(stm))) {
                    stm = color_enemy(stm);
                }
                break;
            }

            occ ^= attacker;

            if (type == ROOK || type == QUEEN)
                attackers |= attacks_rook(to, occ) & rooks & occ;
            if (type == PAWN || type == BISHOP || type == QUEEN)
                attackers |= attacks_bishop(to, occ) & bishops & occ;
        }

        return stm != board.piece_at(from).color;
    }
} // namespace search