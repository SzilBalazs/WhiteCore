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

typedef uint64_t U64;
typedef int32_t Score;
typedef int8_t Depth;
typedef int8_t Ply;

constexpr int64_t INF_TIME = 1'000'000'000'000'000;
constexpr int64_t INF_NODES = 1'000'000'000'000'000;
constexpr int64_t MOVE_OVERHEAD = 30;
constexpr Score UNKNOWN_SCORE = 300000;
constexpr Score INF_SCORE = 200000;
constexpr Score MATE_VALUE = 100000;
constexpr Score TB_WIN_SCORE = 50000;
constexpr Score TB_WORST_WIN = 49000;
constexpr Score TB_BEST_LOSS = -49000;
constexpr Score TB_LOSS_SCORE = -50000;
constexpr Score WORST_MATE = MATE_VALUE - 100;
constexpr Score DRAW_VALUE = 0;

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

constexpr unsigned char WK_MASK = 1;
constexpr unsigned char WQ_MASK = 2;
constexpr unsigned char BK_MASK = 4;
constexpr unsigned char BQ_MASK = 8;

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

struct Piece {
    PieceType type = PIECE_EMPTY;
    Color color = COLOR_EMPTY;

    constexpr Piece() = default;

    constexpr Piece(PieceType pt, Color c) {
        type = pt;
        color = c;
    }

    constexpr bool is_null() const {
        return type == PIECE_EMPTY || color == COLOR_EMPTY;
    }

	constexpr bool is_ok() const {
		return !is_null();
	}
};

constexpr Piece NULL_PIECE = Piece();
constexpr PieceType index_to_type[7] = {KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, PIECE_EMPTY};
constexpr Color index_to_color[3] = {WHITE, BLACK, COLOR_EMPTY};

constexpr unsigned int PROMO_FLAG = 0x8;    // 0b1000
constexpr unsigned int CAPTURE_FLAG = 0x4;  // 0b0100
constexpr unsigned int SPECIAL1_FLAG = 0x2; // 0b0010
constexpr unsigned int SPECIAL2_FLAG = 0x1; // 0b0001

constexpr unsigned int QUIET_MOVE = 0;
constexpr unsigned int CAPTURE = CAPTURE_FLAG;

constexpr unsigned int DOUBLE_PAWN_PUSH = SPECIAL2_FLAG;
constexpr unsigned int EP_CAPTURE = CAPTURE_FLAG | SPECIAL2_FLAG;

constexpr unsigned int PROMO_KNIGHT = PROMO_FLAG;
constexpr unsigned int PROMO_BISHOP = PROMO_FLAG | SPECIAL2_FLAG;
constexpr unsigned int PROMO_ROOK = PROMO_FLAG | SPECIAL1_FLAG;
constexpr unsigned int PROMO_QUEEN = PROMO_FLAG | SPECIAL1_FLAG | SPECIAL2_FLAG;

constexpr unsigned int PROMO_CAPTURE_KNIGHT = CAPTURE_FLAG | PROMO_FLAG;
constexpr unsigned int PROMO_CAPTURE_BISHOP = CAPTURE_FLAG | PROMO_FLAG | SPECIAL2_FLAG;
constexpr unsigned int PROMO_CAPTURE_ROOK = CAPTURE_FLAG | PROMO_FLAG | SPECIAL1_FLAG;
constexpr unsigned int PROMO_CAPTURE_QUEEN = CAPTURE_FLAG | PROMO_FLAG | SPECIAL1_FLAG | SPECIAL2_FLAG;

constexpr unsigned int KING_CASTLE = SPECIAL1_FLAG;
constexpr unsigned int QUEEN_CASTLE = SPECIAL1_FLAG | SPECIAL2_FLAG;
