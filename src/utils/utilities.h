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

#include "../core/constants.h"
#include "logger.h"

void init_all();

template<Color color>
constexpr Color color_enemy() {
	if constexpr (color == WHITE)
		return BLACK;
	else
		return WHITE;
}

inline Color color_enemy(Color color) {
	return color == WHITE ? BLACK : WHITE;
}

constexpr unsigned int square_to_rank(Square square) {
	return square >> 3;
}

constexpr unsigned int square_to_file(Square square) {
	return square & 7;
}

inline std::string format_square(Square square) {
	return std::string() + (char) ('a' + (char) square % 8) + (char) ('1' + (char) (square / 8));
}

// Function that converts an uci format square string into an actual square.
inline Square square_from_string(const std::string &s) {
	if (s[0] == '-') {
		return NULL_SQUARE;
	} else if ('a' <= s[0] && s[0] <= 'z') {
		return Square((s[0] - 'a') + (s[1] - '1') * 8);
	} else if ('A' <= s[0] && s[0] <= 'Z') {
		return Square((s[0] - 'A') + (s[1] - '1') * 8);
	} else {
		logger.error("square_from_string", "Invalid string: " + s);
	}

	return NULL_SQUARE;
}

constexpr Square square_mirror(Square square) {
	return Square(56 - square + square_to_file(square));
}

constexpr Square square_flip(Square sq) {
	return Square(int(sq) ^ 56);
}

inline Piece piece_from_char(char c) {
	Piece piece;

	if ('a' <= c && c <= 'z') {
		piece.color = BLACK;
	} else if ('A' <= c && c <= 'Z') {
		piece.color = WHITE;
		c += 32;
	} else {
		logger.error("piece_from_char", "Invalid char");
	}

	switch (c) {
	case 'p':
		piece.type = PAWN;
		break;
	case 'r':
		piece.type = ROOK;
		break;
	case 'n':
		piece.type = KNIGHT;
		break;
	case 'b':
		piece.type = BISHOP;
		break;
	case 'q':
		piece.type = QUEEN;
		break;
	case 'k':
		piece.type = KING;
		break;
	default:
		logger.error("piece_from_char", "Invalid char!");
	}
	return piece;
}

inline char char_from_piece(Piece piece) {
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

inline Square operator+(Square a, int b) {
	return Square(int(a) + b);
}

inline Square operator-(Square a, int b) {
	return Square(int(a) - b);
}

inline Square operator+=(Square &a, int b) {
	return a = a + b;
}

inline Square operator-=(Square &a, int b) {
	return a = a - b;
}