// WhiteCore is a C++ chess engine
// Copyright (c) 2022-2025 Balázs Szilágyi
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

#include "magic.h"
#include "masks.h"

namespace chess {

    [[nodiscard]] Bitboard attacks_rook(Square square, Bitboard occ) {
        const Magic &m = magic_rook[square];
        return m.ptr[get_magic_index(m, occ)];
    }

    [[nodiscard]] Bitboard attacks_bishop(Square square, Bitboard occ) {
        const Magic &m = magic_bishop[square];
        return m.ptr[get_magic_index(m, occ)];
    }

    [[nodiscard]] Bitboard attacks_queen(Square square, Bitboard occ) {
        return attacks_rook(square, occ) | attacks_bishop(square, occ);
    }

    template<PieceType pt>
    [[nodiscard]] constexpr Bitboard attacks_piece(Square square, Bitboard occupied) {
        assert((pt != PAWN) && (pt != PIECE_EMPTY));
        switch (pt) {
            case KNIGHT:
                return masks_knight[square];
            case BISHOP:
                return attacks_bishop(square, occupied);
            case ROOK:
                return attacks_rook(square, occupied);
            case QUEEN:
                return attacks_queen(square, occupied);
            case KING:
                return masks_king[square];
            default:
                return 0;
        }
    }

    [[nodiscard]] Bitboard attacks_piece(PieceType pt, Square square, Bitboard occupied) {
        assert((pt != PAWN) && (pt != PIECE_EMPTY));
        switch (pt) {
            case KNIGHT:
                return masks_knight[square];
            case BISHOP:
                return attacks_bishop(square, occupied);
            case ROOK:
                return attacks_rook(square, occupied);
            case QUEEN:
                return attacks_queen(square, occupied);
            case KING:
                return masks_king[square];
            default:
                return 0;
        }
    }

    template<Color color>
    [[nodiscard]] Bitboard attacks_piece(PieceType pt, Square square, Bitboard occupied) {
        assert((pt != PAWN) && (pt != PIECE_EMPTY));
        switch (pt) {
            case PAWN:
                return masks_pawn[square][color];
            case KNIGHT:
                return masks_knight[square];
            case BISHOP:
                return attacks_bishop(square, occupied);
            case ROOK:
                return attacks_rook(square, occupied);
            case QUEEN:
                return attacks_queen(square, occupied);
            case KING:
                return masks_king[square];
            default:
                return 0;
        }
    }
} // namespace chess