// WhiteCore is a C++ chess engine
// Copyright (c) 2022-2023 Balázs Szilágyi
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

// The concept of this class is from https://www.chessprogramming.org/Encoding_Moves

#pragma once

#include "../utils/utilities.h"
#include "constants.h"

namespace core {
    class Move {
    public:

        static constexpr unsigned int PROMO_FLAG = 1 << 3;
        static constexpr unsigned int CAPTURE_FLAG = 1 << 2;
        static constexpr unsigned int SPECIAL1_FLAG = 1 << 1;
        static constexpr unsigned int SPECIAL2_FLAG = 1 << 0;
        static constexpr unsigned int QUIET_MOVE = 0;
        static constexpr unsigned int CAPTURE = CAPTURE_FLAG;
        static constexpr unsigned int DOUBLE_PAWN_PUSH = SPECIAL2_FLAG;
        static constexpr unsigned int EP_CAPTURE = CAPTURE_FLAG | SPECIAL2_FLAG;
        static constexpr unsigned int PROMO_KNIGHT = PROMO_FLAG;
        static constexpr unsigned int PROMO_BISHOP = PROMO_FLAG | SPECIAL2_FLAG;
        static constexpr unsigned int PROMO_ROOK = PROMO_FLAG | SPECIAL1_FLAG;
        static constexpr unsigned int PROMO_QUEEN = PROMO_FLAG | SPECIAL1_FLAG | SPECIAL2_FLAG;
        static constexpr unsigned int PROMO_CAPTURE_KNIGHT = CAPTURE_FLAG | PROMO_FLAG;
        static constexpr unsigned int PROMO_CAPTURE_BISHOP = CAPTURE_FLAG | PROMO_FLAG | SPECIAL2_FLAG;
        static constexpr unsigned int PROMO_CAPTURE_ROOK = CAPTURE_FLAG | PROMO_FLAG | SPECIAL1_FLAG;
        static constexpr unsigned int PROMO_CAPTURE_QUEEN = CAPTURE_FLAG | PROMO_FLAG | SPECIAL1_FLAG | SPECIAL2_FLAG;
        static constexpr unsigned int KING_CASTLE = SPECIAL1_FLAG;
        static constexpr unsigned int QUEEN_CASTLE = SPECIAL1_FLAG | SPECIAL2_FLAG;

        // Initialize the move with a from square, to square, and flags
        constexpr Move(Square from, Square to, unsigned int flags) {
            data = (flags << 12) | (from << 6) | (to);
        }

        // Initialize the move with a from square and to square, default flags
        constexpr Move(Square from, Square to) {
            data = (from << 6) | (to);
        }

        // Default constructor, initialize move as null move
        constexpr Move() = default;

        // Returns the to square of the move
        [[nodiscard]]  constexpr Square get_to() const {
            return Square(data & 0x3f);
        }

        // Returns the from square of the move
        [[nodiscard]] constexpr Square get_from() const {
            return Square((data >> 6) & 0x3f);
        }

        // Returns true if the flag is set in the move
        [[nodiscard]] constexpr bool is_flag(unsigned int flag) const {
            return (data >> 12) & flag;
        }

        // Returns true if the move type is equal to flag
        [[nodiscard]] constexpr bool eq_flag(unsigned int flag) const {
            return (data >> 12) == flag;
        }

        // Returns true if the move is not null
        [[nodiscard]] constexpr bool is_ok() const {
            return data != 0;
        }

        // Returns true if the move is a capture
        [[nodiscard]] constexpr bool is_capture() const {
            return is_flag(CAPTURE_FLAG);
        }

        // Returns true if the move is a promotion
        [[nodiscard]] constexpr bool is_promo() const {
            return is_flag(PROMO_FLAG);
        }

        // Returns true if the move has the SPECIAL1_FLAG set
        [[nodiscard]] constexpr bool is_special_1() const {
            return is_flag(SPECIAL1_FLAG);
        }

        // Returns true if the move has the SPECIAL2_FLAG set
        [[nodiscard]] constexpr bool is_special_2() const {
            return is_flag(SPECIAL2_FLAG);
        }

        // Returns true if the move is quiet - not a capture.
        [[nodiscard]] constexpr bool is_quiet() const {
            return !is_capture();
        }

        // Explicit conversion to bool, returns true if move is not a null move
        constexpr explicit operator bool() const {
            return is_ok();
        }

        // Comparison operator, returns true if moves are equal
        constexpr bool operator==(Move a) const {
            return (data & 0xFFFF) == (a.data & 0xFFFF);
        }

        // Comparison operator, returns true if moves are not equal
        constexpr bool operator!=(Move a) const {
            return (data & 0xFFFF) != (a.data & 0xFFFF);
        }

        // Returns the move in UCI format as a string
        [[nodiscard]]  std::string to_uci() const {
            std::string res;
            if (is_promo()) {
                if (!is_special_1() && !is_special_2())
                    res += "n";
                else if (!is_special_1() && is_special_2())
                    res += "b";
                else if (is_special_1() && !is_special_2())
                    res += "r";
                else
                    res += "q";
            }
            return format_square(get_from()) + format_square(get_to()) + res;
        }

    private:
        uint16_t data = 0;
    };

    constexpr Move NULL_MOVE = Move();

    std::ostream &operator<<(std::ostream &os, const Move &move) {
        os << move.to_uci();
        return os;
    }
} // namespace core