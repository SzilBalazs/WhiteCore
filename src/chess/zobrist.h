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

#include "../utils/utilities.h"
#include "castling_rights.h"
#include "randoms.h"

namespace core {
    struct Zobrist {

        U64 hash = 0;

        Zobrist() = default;

        explicit Zobrist(U64 hash) : hash(hash) {}

        [[nodiscard]] operator U64() const {
            return hash;
        }

        void xor_stm() {
            hash ^= *rand_table_color;
        }

        void xor_piece(Square square, Piece piece) {
            hash ^= rand_table_pieces[12 * square + 6 * piece.color + piece.type];
        }

        void xor_ep(Square square) {
            hash ^= rand_table_ep[square_to_file(square)];
        }

        void xor_castle(CastlingRights rights) {
            hash ^= rand_table_castling[rights.data];
        }
    };
} // namespace core