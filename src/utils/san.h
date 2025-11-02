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

#include "../chess/board.h"
#include "../chess/move_generation.h"

std::string uci_to_san(const chess::Move &move, const chess::Board &x) {

    chess::Board board = x;

    std::stringstream san_move;

    if (move.eq_flag(chess::Move::KING_CASTLE)) {
        san_move << "O-O";
    } else if (move.eq_flag(chess::Move::QUEEN_CASTLE)) {
        san_move << "O-O-O";
    } else {
        const Square from = move.get_from();
        Piece piece = board.piece_at(from);

        if (piece.type != PAWN) {
            san_move << char_from_piece(Piece(piece.type, WHITE));

            chess::Move buffer[200];
            chess::Move *end_ptr = chess::gen_moves(board, buffer, false);
            size_t move_count = end_ptr - buffer;

            bool use_file = false;
            bool use_rank = false;

            for (size_t i = 0; i < move_count; i++) {
                if (buffer[i].get_to() == move.get_to() && board.piece_at(from) == board.piece_at(buffer[i].get_from()) && buffer[i] != move) {
                    if (square_to_file(buffer[i].get_from()) != square_to_file(from)) {
                        use_file = true;
                    } else {
                        use_rank = true;
                    }
                }
            }

            if (use_file) {
                san_move << char('a' + square_to_file(from));
            }

            if (use_rank) {
                san_move << char('1' + square_to_rank(from));
            }


        } else if (move.is_capture()) {
            san_move << char('a' + square_to_file(from));
        }

        if (move.is_capture()) {
            san_move << "x";
        }

        san_move << format_square(move.get_to());

        if (move.is_promo()) {
            san_move << "=" << char_from_piece(Piece(move.get_promo_type(), WHITE));
        }
    }

    board.make_move(move);

    chess::Move buffer[200];
    chess::Move *end_ptr = chess::gen_moves(board, buffer, false);
    size_t move_count = end_ptr - buffer;

    if (move_count == 0) {
        if (board.is_check()) {
            san_move << "# ";
            if (board.get_stm() == WHITE) {
                san_move << "0-1";
            } else {
                san_move << "1-0";
            }
        } else {
            san_move << " 1/2-1/2";
        }
    } else {
        if (board.is_check()) {
            san_move << "+";
        }
        if (board.is_draw<true>()) {
            san_move << " 1/2-1/2";
        }
    }

    return san_move.str();
}