// WhiteCore is a C++ chess engine
// Copyright (c) 2022-2024 Balázs Szilágyi
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

#include <cstdint>
#include <string>

// clang-format off

// #define TUNE

#if defined(NATIVE) && defined(__BMI2__)
#define BMI2
#endif

#if defined(NATIVE) && defined(__AVX2__)
#define AVX2
#endif

using Score = int32_t;
using Depth = int8_t;
using Ply = int8_t;

constexpr int64_t INF_TIME = 1'000'000'000'000'000;
constexpr int64_t INF_NODES = 1'000'000'000'000'000;
constexpr Score UNKNOWN_SCORE = 30000;
constexpr Score INF_SCORE = 20000;
constexpr Score MATE_VALUE = 10000;
constexpr Score WORST_MATE = MATE_VALUE - 100;

constexpr Score PIECE_VALUES[7] = {
        0, 100, 300, 350, 500, 1000, 0};

const Ply MAX_PLY = 100;

const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

constexpr unsigned int RANDOM_SEED = 1254383;

enum Square : int {
    A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7,
    A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15,
    A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23,
    A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31,
    A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39,
    A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47,
    A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55,
    A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63,
    NULL_SQUARE = 64
};

// clang-format off
constexpr int MVVLVA[6][6] = {
		{0,  0,  0,  0,  0,  0},      // KING
		{0, 14, 13, 12, 11, 10},      // PAWN
		{0, 24, 23, 22, 21, 20},      // KNIGHT
		{0, 34, 33, 32, 31, 30},      // BISHOP
		{0, 44, 43, 42, 41, 40},      // ROOK
		{0, 54, 53, 52, 51, 50}       // QUEEN
};
// clang-format on

const std::string ASCII_WHITE_PIECE = "\u001b[90;107m";
const std::string ASCII_BLACK_PIECE = "\u001b[100;97m";

enum LineType : int {
    HORIZONTAL = 0,
    VERTICAL = 1,
    DIAGONAL = 2,
    ANTI_DIAGONAL = 3
};

enum Direction : int {
    NORTH = 8,
    WEST = -1,
    SOUTH = -8,
    EAST = 1,
    NORTH_EAST = 9,
    NORTH_WEST = 7,
    SOUTH_WEST = -9,
    SOUTH_EAST = -7
};

constexpr Direction DIRECTIONS[8] = {NORTH, WEST, NORTH_EAST, NORTH_WEST, SOUTH, EAST, SOUTH_WEST, SOUTH_EAST};

enum PieceType {
    PIECE_EMPTY = 6,
    KING = 0,
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5
};

enum Color {
    COLOR_EMPTY = 2,
    WHITE = 0,
    BLACK = 1
};

enum NodeType {
    ROOT_NODE,
    PV_NODE,
    NON_PV_NODE
};

constexpr PieceType PIECE_TYPES_BY_VALUE[6] = {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};
constexpr float PIECE_TO_PHASE[6] = {0.0f, 1.0f, 2.0f, 2.0f, 4.0f, 8.0f};
constexpr int PIECE_TO_PHASE_INT[6] = {0, 1, 2, 2, 4, 8};

struct Piece {
    PieceType type = PIECE_EMPTY;
    Color color = COLOR_EMPTY;

    constexpr Piece() = default;

    constexpr Piece(PieceType pt, Color c) {
        type = pt;
        color = c;
    }

    [[nodiscard]] constexpr bool is_null() const {
        return type == PIECE_EMPTY || color == COLOR_EMPTY;
    }

    [[nodiscard]] constexpr bool is_ok() const {
        return !is_null();
    }

    constexpr bool operator==(Piece p) const {
        return color == p.color && type == p.type;
    }
};

constexpr Piece NULL_PIECE = Piece();
constexpr PieceType index_to_type[7] = {KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, PIECE_EMPTY};
constexpr Color index_to_color[3] = {WHITE, BLACK, COLOR_EMPTY};
