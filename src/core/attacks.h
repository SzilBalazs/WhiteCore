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

#include "bitboard.h"
#include "magic.h"
#include "masks.h"

inline Bitboard attacks_rook(Square square, Bitboard occ) {
	const Magic &m = magic_rook[square];
	return m.ptr[get_magic_index(m, occ)];
}

inline Bitboard attacks_bishop(Square square, Bitboard occ) {
	const Magic &m = magic_bishop[square];
	return m.ptr[get_magic_index(m, occ)];
}

inline Bitboard attacks_queen(Square square, Bitboard occ) {
	return attacks_rook(square, occ) | attacks_bishop(square, occ);
}

template<PieceType type>
constexpr Bitboard attacks_piece(Square square, Bitboard occupied) {
	assert((type != PAWN) && (type != PIECE_EMPTY));
	switch (type) {
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

inline Bitboard attacks_piece(PieceType type, Square square, Bitboard occupied) {
	assert((type != PAWN) && (type != PIECE_EMPTY));
	switch (type) {
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
inline Bitboard attacks_piece(PieceType type, Square square, Bitboard occupied) {
	assert((type != PAWN) && (type != PIECE_EMPTY));
	switch (type) {
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