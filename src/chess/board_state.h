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

#include "constants.h"
#include "zobrist.h"

namespace chess {
    struct BoardState {
        Color stm = WHITE;
        Square ep = NULL_SQUARE;
        Zobrist hash = Zobrist();
        Piece piece_captured = NULL_PIECE;
        CastlingRights rights = CastlingRights();
        size_t move50 = 0;
    };
} // namespace chess
