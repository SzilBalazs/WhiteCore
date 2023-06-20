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
#include "constants.h"
#include "board_state.h"
#include "move.h"
#include "../utils/utilities.h"

#include <vector>
#include <sstream>

#define state states.back()

class Board {
public:

	constexpr Color get_stm() {
		return state.stm;
	}

	constexpr Square get_ep() {
		return state.ep;
	}

	constexpr Zobrist get_hash() {
		return state.hash;
	}

	constexpr unsigned int get_move50() {
		return state.move50;
	}

	constexpr CastlingRights get_rights() {
		return state.rights;
	}

	constexpr Piece piece_at(Square square) {
		return mailbox[square];
	}

	template<Color color, PieceType pt>
	constexpr Bitboard pieces() {
		return bb_colors[color] & bb_pieces[pt];
	}

	template<Color color>
	constexpr Bitboard pieces(PieceType pt) {
		return bb_colors[color] & bb_pieces[pt];
	}

	template<PieceType pt>
	constexpr Bitboard pieces(Color color) {
		return bb_colors[color] & bb_pieces[pt];
	}

	constexpr Bitboard pieces(Color color, PieceType pt) {
		return bb_colors[color] & bb_pieces[pt];
	}

	template<PieceType pt>
	constexpr Bitboard pieces() {
		return bb_pieces[pt];
	}

	constexpr Bitboard pieces(PieceType pt) {
		return bb_pieces[pt];
	}

	template<Color color>
	constexpr Bitboard pieces() {
		return bb_colors[color];
	}

	constexpr Bitboard pieces(Color color) {
		return bb_colors[color];
	}

	constexpr Bitboard occupied() {
		return bb_colors[WHITE] | bb_colors[BLACK];
	}

	constexpr Bitboard empty() {
		return ~occupied();
	}

	inline void make_move(Move move) {
		const Square from = move.get_from();
		const Square to = move.get_to();
		Piece piece_moved = piece_at(from);
		const Color stm = piece_moved.color;
		const Color xstm = color_enemy(stm);
		const BoardState state_old = state;
		const Direction UP = stm == WHITE ? NORTH : -NORTH;
		const Direction DOWN = -UP;

		assert(!states.empty());
		assert(stm == state.stm);

		states.emplace_back(state);

		if (move.is_capture() || piece_moved.type == PAWN) {
			state.move50 = 0;
		} else {
			state.move50++;
		}

		state.stm = xstm;

		// Remove state dependent hash values
		state.hash.xor_stm();
		if (state_old.ep != NULL_SQUARE) state.hash.xor_ep(state_old.ep);
		state.hash.xor_castle(state_old.rights);

		if (move.eq_flag(EP_CAPTURE)) {
			state.piece_captured = Piece(PAWN, xstm);
			square_clear(to + DOWN);
		} else {
			state.piece_captured = piece_at(to);
		}

		if (move.is_capture()) assert(state.piece_captured.is_ok());
		else assert(state.piece_captured.is_null());

		if (move.eq_flag(DOUBLE_PAWN_PUSH)) {
			state.ep = from + UP;
			state.hash.xor_ep(state.ep);
		} else {
			state.ep = NULL_SQUARE;
		}

		if (move.is_promo()) {
			if (move.eq_flag(PROMO_BISHOP) || move.eq_flag(PROMO_CAPTURE_BISHOP)) {
				piece_moved.type = BISHOP;
			} else if (move.eq_flag(PROMO_KNIGHT) || move.eq_flag(PROMO_CAPTURE_KNIGHT)) {
				piece_moved.type = KNIGHT;
			} else if (move.eq_flag(PROMO_ROOK) || move.eq_flag(PROMO_CAPTURE_ROOK)) {
				piece_moved.type = ROOK;
			} else if (move.eq_flag(PROMO_QUEEN) || move.eq_flag(PROMO_CAPTURE_QUEEN)) {
				piece_moved.type = QUEEN;
			}
		}

		move_piece(piece_moved, from, to);

		// TODO handle castling

		state.hash.xor_castle(state.rights);
	}

	inline void undo_move(Move move) {
		const Square from = move.get_from();
		const Square to = move.get_to();
		Piece piece_moved = piece_at(to);
		const Color stm = piece_moved.color;
		const Color xstm = color_enemy(stm);
		const Direction UP = xstm == WHITE ? NORTH : -NORTH;
		const Direction DOWN = -UP;

		assert(states.size() > 1);
		assert(piece_moved.color == xstm);
		if (move.is_capture()) assert(state.piece_captured.is_ok());

		if (move.is_promo()) {
			piece_moved.type = PAWN;
		}

		move_piece(piece_moved, to, from);

		if (move.eq_flag(EP_CAPTURE)) {
			square_set(to + DOWN, state.piece_captured);
		} else if (move.is_capture()) {
			square_set(to, state.piece_captured);
		}

		// TODO handle castling

		states.pop_back();
	}

	inline void board_load(const std::string &fen) {
		board_clear();

		logger.info("Board::board_load", "Loading fen", fen);

		std::stringstream ss(fen);
		std::string pieces, stm, rights, ep, move50, move_cnt;
		ss >> pieces >> stm >> rights >> ep >> move50 >> move_cnt;

		Square square = A8;
		for (char c : pieces) {
			if ('1' <= c && c <= '8') {
				square += c - '0';
			} else if (c == '/') {
				square -= 16;
			} else {
				square_set(square, piece_from_char(c));
				square += 1;
			}
		}

		if (stm == "w") {
			state.stm = WHITE;
		} else if (stm == "b") {
			state.stm = BLACK;
			state.hash.xor_stm();
		} else {
			logger.error("Board::board_load", "Invalid stm string!");
		}

		// TODO parse castling rights

		state.ep = square_from_string(ep);
		state.move50 = std::stoi(move50);

		state.hash.xor_castle(state.rights);
		if (state.ep != NULL_SQUARE) state.hash.xor_ep(state.ep);

		logger.info("Board::board_load", "Finished loading fen");
	}

	inline std::string get_fen() {
		std::string fen;

		Square square = A8;
		int empty = 0;
		while (true) {
			if (mailbox[square].is_null()) empty++;
			else {
				if (empty) fen += std::to_string(empty);
				fen += char_from_piece(mailbox[square]);
				empty = 0;
			}

			if (square == H1) break;

			if (square_to_file(square) == 7) {
				square -= 15;
				if (empty) fen += std::to_string(empty);
				fen += '/';
				empty = 0;
			} else {
				square += 1;
			}
		}

		if (empty) fen += std::to_string(empty);
		fen += " ";
		fen += (get_stm() == WHITE ? "w" : "b");
		fen += " ";
		fen += "- "; // TODO Castling
		fen += get_ep() == NULL_SQUARE ? "-" : format_square(get_ep());
		fen += " ";
		fen += std::to_string(get_move50());
		return fen;
	}

	inline void display() {
		std::vector<std::string> text;
		text.emplace_back(std::string("50-move draw counter: ") + std::to_string(state.move50));
		text.emplace_back(std::string("Hash: ") + std::to_string(get_hash().hash));
		text.emplace_back(std::string("Fen: ") + get_fen());

		if (get_ep() != NULL_SQUARE)
			text.emplace_back(std::string("En passant square: ") + format_square(get_ep()));
		std::string cr = "None"; // TODO Castling rights

		text.emplace_back(std::string("Castling rights: ") + cr);
		text.emplace_back(std::string("Side to move: ") + std::string(get_stm() == WHITE ? "White" : "Black"));

		std::cout << "\n     A   B   C   D   E   F   G   H  \n";
		for (int i = 8; i >= 1; i--) {
			std::cout << "   +---+---+---+---+---+---+---+---+";
			if (i <= 7 && !text.empty()) {
				std::cout << "        " << text.back();
				text.pop_back();
			}
			std::cout << "\n " << i << " |";
			for (int j = 1; j <= 8; j++) {
				Piece piece = piece_at(Square((i - 1) * 8 + (j - 1)));
				std::cout << (piece.color == WHITE ? ASCII_WHITE_PIECE : (piece.color == BLACK ? ASCII_BLACK_PIECE : "")) << " " << char_from_piece(piece) << " \u001b[0m|";
			}
			if (i <= 7 && !text.empty()) {
				std::cout << "        " << text.back();
				text.pop_back();
			}
			std::cout << "\n";
		}
		std::cout << "   +---+---+---+---+---+---+---+---+\n\n"
			 << std::endl;
	}

private:
	Piece mailbox[64];
	Bitboard bb_pieces[6], bb_colors[2];

	std::vector<BoardState> states;

	inline void square_clear(Square square) {
		const Piece piece = piece_at(square);
		if (piece.is_null()) return;

		bb_colors[piece.color].clear(square);
		bb_pieces[piece.type].clear(square);
		mailbox[square] = NULL_PIECE;

		state.hash.xor_piece(square, piece);
	}

	inline void square_set(Square square, Piece piece) {
		assert(piece.is_ok());
		square_clear(square);

		bb_colors[piece.color].set(square);
		bb_pieces[piece.type].set(square);
		mailbox[square] = piece;

		state.hash.xor_piece(square, piece);
	}

	inline void move_piece(Piece piece, Square from, Square to) {
		assert(piece.is_ok());

		square_clear(from);
		square_set(to, piece);
	}

	inline void board_clear() {

		logger.info("Board::board_clear", "Clearing board...");

		for (Bitboard &i : bb_pieces) i = 0;

		for (Bitboard &i : bb_colors) i = 0;

		for (Square square = A1; square < 64; square += 1)
			mailbox[square] = NULL_PIECE;

		states.clear();

		states.emplace_back(BoardState());

		logger.info("Board::board_clear", "Board has been cleared");
	}
};

#undef state
