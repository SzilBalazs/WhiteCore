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

#include "../chess/constants.h"

#include <chrono>
#include <iostream>
#include <sstream>

void init_all();

template<typename... Args>
void print(std::stringstream &ss, Args... args) {
    ((ss << args << ' '), ...);
}

template<typename... Args>
void print(Args... args) {
    std::stringstream ss;
    print(ss, args...);
    std::cout << ss.str() << std::endl;
}

template<Color color>
constexpr Color color_enemy() {
    if constexpr (color == WHITE)
        return BLACK;
    else
        return WHITE;
}

Color color_enemy(Color color) {
    return color == WHITE ? BLACK : WHITE;
}

constexpr unsigned int square_to_rank(Square square) {
    return square >> 3;
}

constexpr unsigned int square_to_file(Square square) {
    return square & 7;
}

std::string format_square(Square square) {
    return std::string() + (char) ('a' + (char) square % 8) + (char) ('1' + (char) (square / 8));
}

// Function that converts an uci format square string into an actual square.
Square square_from_string(const std::string &s) {
    if (s[0] == '-') {
        return NULL_SQUARE;
    } else if ('a' <= s[0] && s[0] <= 'z') {
        return Square((s[0] - 'a') + (s[1] - '1') * 8);
    } else if ('A' <= s[0] && s[0] <= 'Z') {
        return Square((s[0] - 'A') + (s[1] - '1') * 8);
    } else {
        throw std::runtime_error("Invalid square string: " + s);
    }
}

// clang-format off
static constexpr Piece PIECE_LOOKUP[128] = {
        {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE},
        {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE},
        {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE},
        {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE},
        {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE},
        {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE},
        {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE},
        {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE},
        {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {BISHOP, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE},
        {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {KING, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {KNIGHT, WHITE}, {PIECE_EMPTY, WHITE},
        {PAWN, WHITE}, {QUEEN, WHITE}, {ROOK, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE},
        {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE},
        {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, BLACK}, {BISHOP, BLACK}, {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, BLACK},
        {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, BLACK}, {KING, BLACK}, {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, BLACK}, {KNIGHT, BLACK}, {PIECE_EMPTY, BLACK},
        {PAWN, BLACK}, {QUEEN, BLACK}, {ROOK, BLACK}, {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, BLACK},
        {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, BLACK}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE}, {PIECE_EMPTY, WHITE},
};
// clang-format on

constexpr Piece piece_from_char(char c) {
    return PIECE_LOOKUP[int(c)];
}

char char_from_piece(Piece piece) {
    char base;
    switch (piece.type) {
        case PAWN:
            base = 'p';
            break;
        case ROOK:
            base = 'r';
            break;
        case KNIGHT:
            base = 'n';
            break;
        case BISHOP:
            base = 'b';
            break;
        case QUEEN:
            base = 'q';
            break;
        case KING:
            base = 'k';
            break;
        default:
            base = ' ';
    }
    if (base != ' ' && piece.color == WHITE)
        base -= 32;
    return base;
}

constexpr Direction opposite(Direction direction) {
    return Direction(-direction);
}

constexpr Direction operator-(Direction direction) {
    return opposite(direction);
}

constexpr int64_t calculate_nps(int64_t time, int64_t nodes) {
    return nodes * 1000 / (time + 1);
}

Square operator+(Square a, int b) {
    return Square(int(a) + b);
}

Square operator-(Square a, int b) {
    return Square(int(a) - b);
}

Square operator+=(Square &a, int b) {
    return a = a + b;
}

Square operator-=(Square &a, int b) {
    return a = a - b;
}

int64_t now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
};