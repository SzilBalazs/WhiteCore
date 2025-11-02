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

#include "constants.h"

#include <bit>
#include <cassert>
#include <immintrin.h>

#ifdef _MSC_VER
#include <intrin.h>
#include <nmmintrin.h>
#endif

namespace chess {

    // Used for referencing the 64 squares of a board using a 64-bit number
    struct Bitboard {

        uint64_t bb = 0;

        constexpr Bitboard(uint64_t value) {
            bb = value;
        }

        Bitboard(Square square);

        constexpr Bitboard() = default;

        // Returns the square's value.
        [[nodiscard]] constexpr bool get(Square square) const {
            return (bb >> square) & 1;
        }

        // Sets the square's value to 1.
        constexpr void set(Square square) {
            bb |= 1ULL << square;
        }

        // Clears the square's value.
        constexpr void clear(Square square) {
            bb &= ~(1ULL << square);
        }

        // Returns the number of bits set to 1.
        [[nodiscard]] constexpr int pop_count() const {
            return std::popcount(bb);
        }

        // Returns the square with the lowest index, that is set to 1.
        [[nodiscard]] constexpr Square lsb() const {
            return Square(std::countr_zero(bb));
        }

        // Clears the square with the lowest index, that is set to 1.
        [[nodiscard]] constexpr Square pop_lsb() {
            Square square = lsb();
            bb &= bb - 1;
            return square;
        }

        [[nodiscard]] constexpr Bitboard operator*(Bitboard a) const {
            return bb * a.bb;
        }

        [[nodiscard]] constexpr bool operator==(Bitboard a) const {
            return bb == a.bb;
        }

        [[nodiscard]] constexpr bool operator!=(Bitboard a) const {
            return bb != a.bb;
        }

        [[nodiscard]] constexpr Bitboard operator+(Bitboard a) const {
            return bb + a.bb;
        }

        [[nodiscard]] constexpr Bitboard operator-(Bitboard a) const {
            return bb - a.bb;
        }

        [[nodiscard]] constexpr Bitboard operator&(Bitboard a) const {
            return bb & a.bb;
        }

        [[nodiscard]] constexpr Bitboard operator|(Bitboard a) const {
            return bb | a.bb;
        }

        [[nodiscard]] constexpr Bitboard operator^(Bitboard a) const {
            return bb ^ a.bb;
        }

        [[nodiscard]] constexpr Bitboard operator~() const {
            return ~bb;
        }

        [[nodiscard]] constexpr Bitboard operator<<(const unsigned int a) const {
            return bb << a;
        }

        [[nodiscard]] constexpr Bitboard operator>>(const unsigned int a) const {
            return bb >> a;
        }

        constexpr void operator&=(Bitboard a) {
            bb &= a.bb;
        }

        constexpr void operator|=(Bitboard a) {
            bb |= a.bb;
        }

        constexpr void operator^=(Bitboard a) {
            bb ^= a.bb;
        }

        constexpr void operator<<=(const unsigned int a) {
            bb <<= a;
        }

        constexpr void operator>>=(const unsigned int a) {
            bb >>= a;
        }

        [[nodiscard]] constexpr explicit operator bool() const {
            return bb;
        }

        [[nodiscard]] constexpr explicit operator uint64_t() const {
            return bb;
        }
    };

    // Constant values generated at compile time.
    constexpr Bitboard FILE_A = 0x101010101010101ULL;
    constexpr Bitboard FILE_B = FILE_A << 1;
    constexpr Bitboard FILE_C = FILE_A << 2;
    constexpr Bitboard FILE_D = FILE_A << 3;
    constexpr Bitboard FILE_E = FILE_A << 4;
    constexpr Bitboard FILE_F = FILE_A << 5;
    constexpr Bitboard FILE_G = FILE_A << 6;
    constexpr Bitboard FILE_H = FILE_A << 7;

    constexpr Bitboard NOT_FILE_A = ~FILE_A;
    constexpr Bitboard NOT_FILE_H = ~FILE_H;

    constexpr Bitboard RANK_1 = 0xff;
    constexpr Bitboard RANK_2 = RANK_1 << (1 * 8);
    constexpr Bitboard RANK_3 = RANK_1 << (2 * 8);
    constexpr Bitboard RANK_4 = RANK_1 << (3 * 8);
    constexpr Bitboard RANK_5 = RANK_1 << (4 * 8);
    constexpr Bitboard RANK_6 = RANK_1 << (5 * 8);
    constexpr Bitboard RANK_7 = RANK_1 << (6 * 8);
    constexpr Bitboard RANK_8 = RANK_1 << (7 * 8);

    constexpr Bitboard masks_side[2] = {RANK_1 | RANK_2 | RANK_3 | RANK_4, RANK_5 | RANK_6 | RANK_7 | RANK_8};

    constexpr Bitboard WK_CASTLE_SAFE = 0x70ULL;
    constexpr Bitboard WK_CASTLE_EMPTY = 0x60ULL;
    constexpr Bitboard WQ_CASTLE_SAFE = 0x1cULL;
    constexpr Bitboard WQ_CASTLE_EMPTY = 0xeULL;

    constexpr Bitboard BK_CASTLE_SAFE = 0x7000000000000000ULL;
    constexpr Bitboard BK_CASTLE_EMPTY = 0x6000000000000000ULL;
    constexpr Bitboard BQ_CASTLE_SAFE = 0x1c00000000000000ULL;
    constexpr Bitboard BQ_CASTLE_EMPTY = 0xe00000000000000ULL;

    extern Bitboard masks_bit[64];

    Bitboard::Bitboard(Square square) {
        bb = masks_bit[square].bb;
    }

    template<Direction direction>
    [[nodiscard]] constexpr Bitboard step(Bitboard b) {
        switch (direction) {
            case NORTH:
                return b << 8;
            case SOUTH:
                return b >> 8;
            case NORTH_WEST:
                return (b & NOT_FILE_A) << 7;
            case WEST:
                return (b & NOT_FILE_A) >> 1;
            case SOUTH_WEST:
                return (b & NOT_FILE_A) >> 9;
            case NORTH_EAST:
                return (b & NOT_FILE_H) << 9;
            case EAST:
                return (b & NOT_FILE_H) << 1;
            case SOUTH_EAST:
                return (b & NOT_FILE_H) >> 7;
        }
    }

    [[nodiscard]] constexpr Bitboard step(Direction direction, Bitboard b) {
        switch (direction) {
            case NORTH:
                return b << 8;
            case SOUTH:
                return b >> 8;
            case NORTH_WEST:
                return (b & NOT_FILE_A) << 7;
            case WEST:
                return (b & NOT_FILE_A) >> 1;
            case SOUTH_WEST:
                return (b & NOT_FILE_A) >> 9;
            case NORTH_EAST:
                return (b & NOT_FILE_H) << 9;
            case EAST:
                return (b & NOT_FILE_H) << 1;
            case SOUTH_EAST:
                return (b & NOT_FILE_H) >> 7;
        }
        return 0;
    }

    template<Direction direction>
    [[nodiscard]] constexpr Bitboard slide(Square square) {
        Bitboard result;
        Bitboard temp = {square};
        while (temp) {
            temp = step<direction>(temp);
            result |= temp;
        }
        return result;
    }

    [[nodiscard]] Bitboard slide(Direction direction, Square square) {
        Bitboard result;
        Bitboard temp = {square};
        while (temp) {
            temp = step(direction, temp);
            result |= temp;
        }
        return result;
    }

    template<Direction direction>
    [[nodiscard]] constexpr Bitboard slide(Square square, Bitboard occupied) {
        Bitboard result;
        Bitboard temp = {square};
        while (temp && !occupied.get(temp.lsb())) {
            temp = step<direction>(temp);
            result |= temp;
        }
        return result;
    }
} // namespace chess
