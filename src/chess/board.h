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

#include "../network/nnue.h"
#include "../utils/utilities.h"
#include "attacks.h"
#include "bitboard.h"
#include "board_state.h"
#include "move.h"

#include <algorithm>
#include <regex>
#include <sstream>
#include <vector>

#define state states.back()

namespace chess {

    class Board {

    public:
        [[nodiscard]] inline Color get_stm() const {
            return state.stm;
        }

        [[nodiscard]] inline Square get_ep() const {
            return state.ep;
        }

        [[nodiscard]] inline Zobrist get_hash() const {
            return state.hash;
        }

        [[nodiscard]] inline unsigned int get_move50() const {
            return state.move50;
        }

        [[nodiscard]] inline CastlingRights get_rights() const {
            return state.rights;
        }

        [[nodiscard]] constexpr Piece piece_at(Square square) const {
            return mailbox[square];
        }

        template<Color color, PieceType pt>
        [[nodiscard]] constexpr Bitboard pieces() const {
            return bb_colors[color] & bb_pieces[pt];
        }

        template<Color color>
        [[nodiscard]] constexpr Bitboard pieces(PieceType pt) const {
            return bb_colors[color] & bb_pieces[pt];
        }

        template<PieceType pt>
        [[nodiscard]] constexpr Bitboard pieces(Color color) const {
            return bb_colors[color] & bb_pieces[pt];
        }

        [[nodiscard]] constexpr Bitboard pieces(Color color, PieceType pt) const {
            return bb_colors[color] & bb_pieces[pt];
        }

        template<PieceType pt>
        [[nodiscard]] constexpr Bitboard pieces() const {
            return bb_pieces[pt];
        }

        [[nodiscard]] constexpr Bitboard pieces(PieceType pt) const {
            return bb_pieces[pt];
        }

        template<Color color>
        [[nodiscard]] constexpr Bitboard sides() const {
            return bb_colors[color];
        }

        [[nodiscard]] constexpr Bitboard sides(Color color) const {
            return bb_colors[color];
        }

        [[nodiscard]] constexpr Bitboard occupied() const {
            return bb_colors[WHITE] | bb_colors[BLACK];
        }

        [[nodiscard]] constexpr Bitboard empty() const {
            return ~occupied();
        }

        template<bool is_pv = false>
        [[nodiscard]] bool is_draw() const {
            if (get_move50() > 100) return true;
            size_t cnt = 0;
            for (const BoardState &st : states) {
                if (st.hash == states.back().hash) {
                    cnt++;
                }
            }
            return cnt >= 2 + is_pv;
        }

        [[nodiscard]] bool is_check() const;

        [[nodiscard]] bool has_non_pawn() const {
            const Color stm = get_stm();
            return bool(pieces<KNIGHT>(stm) | pieces<BISHOP>(stm) | pieces<ROOK>() | pieces<QUEEN>());
        }

        void make_null_move() {
            const Color xstm = color_enemy(get_stm());
            const BoardState state_old = state;

            states.emplace_back(state);

            state.stm = xstm;
            state.hash.xor_stm();
            state.ep = NULL_SQUARE;
            if (state_old.ep != NULL_SQUARE) state.hash.xor_ep(state_old.ep);
        }

        void undo_null_move() {
            states.pop_back();
        }

        void hash_move_piece(Zobrist &hash, Piece piece, Square from, Square to) const {
            hash.xor_piece(from, piece_at(from));
            Piece captured_piece = piece_at(to);
            if (captured_piece.is_ok()) {
                hash.xor_piece(to, captured_piece);
            }
            hash.xor_piece(to, piece);
        }

        [[nodiscard]] Zobrist hash_after_move(Move move) const {
            const Square from = move.get_from();
            const Square to = move.get_to();

            Zobrist hash = get_hash();
            hash.xor_stm();

            Piece piece_moved = piece_at(from);
            hash_move_piece(hash, piece_moved, from, to);

            return hash;
        }

        void make_move(Move move, nn::NNUE *nnue = nullptr) {
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
            if (state_old.ep != NULL_SQUARE) {
                state.hash.xor_ep(state_old.ep);
            }
            state.hash.xor_castle(state_old.rights);

            if (move.eq_flag(Move::EP_CAPTURE)) {
                state.piece_captured = Piece(PAWN, xstm);
                square_clear(to + DOWN, nnue);
            } else {
                state.piece_captured = piece_at(to);
            }

            if (move.is_capture()) {
                assert(state.piece_captured.is_ok());
            } else {
                assert(state.piece_captured.is_null());
            }

            if (move.eq_flag(Move::DOUBLE_PAWN_PUSH)) {
                state.ep = from + UP;
                state.hash.xor_ep(state.ep);
            } else {
                state.ep = NULL_SQUARE;
            }

            if (move.is_promo()) {
                piece_moved.type = move.get_promo_type();
            }

            move_piece(piece_moved, from, to, nnue);

            if (move.eq_flag(Move::KING_CASTLE)) {
                if (stm == WHITE) {
                    move_piece(Piece(ROOK, WHITE), H1, F1, nnue);
                } else {
                    move_piece(Piece(ROOK, BLACK), H8, F8, nnue);
                }
            } else if (move.eq_flag(Move::QUEEN_CASTLE)) {
                if (stm == WHITE) {
                    move_piece(Piece(ROOK, WHITE), A1, D1, nnue);
                } else {
                    move_piece(Piece(ROOK, BLACK), A8, D8, nnue);
                }
            }

            if (state.rights[CastlingRights::WHITE_KING] && (from == E1 || from == H1 || to == H1)) {
                state.rights -= CastlingRights::WHITE_KING;
            }
            if (state.rights[CastlingRights::WHITE_QUEEN] && (from == E1 || from == A1 || to == A1)) {
                state.rights -= CastlingRights::WHITE_QUEEN;
            }
            if (state.rights[CastlingRights::BLACK_KING] && (from == E8 || from == H8 || to == H8)) {
                state.rights -= CastlingRights::BLACK_KING;
            }
            if (state.rights[CastlingRights::BLACK_QUEEN] && (from == E8 || from == A8 || to == A8)) {
                state.rights -= CastlingRights::BLACK_QUEEN;
            }

            state.hash.xor_castle(state.rights);
        }

        void undo_move(Move move, nn::NNUE *nnue = nullptr) {
            const Square from = move.get_from();
            const Square to = move.get_to();
            Piece piece_moved = piece_at(to);
            const Color stm = piece_moved.color;
            const Direction UP = stm == WHITE ? NORTH : -NORTH;
            const Direction DOWN = -UP;

            assert(states.size() > 1);
            if (move.is_capture()) {
                assert(state.piece_captured.is_ok());
            }

            if (move.is_promo()) {
                piece_moved.type = PAWN;
            }

            if (move.eq_flag(Move::KING_CASTLE)) {
                if (stm == WHITE) {
                    move_piece(Piece(ROOK, WHITE), F1, H1, nnue);
                } else {
                    move_piece(Piece(ROOK, BLACK), F8, H8, nnue);
                }
            } else if (move.eq_flag(Move::QUEEN_CASTLE)) {
                if (stm == WHITE) {
                    move_piece(Piece(ROOK, WHITE), D1, A1, nnue);
                } else {
                    move_piece(Piece(ROOK, BLACK), D8, A8, nnue);
                }
            }

            move_piece(piece_moved, to, from, nnue);

            if (move.eq_flag(Move::EP_CAPTURE)) {
                square_set(to + DOWN, state.piece_captured, nnue);
            } else if (move.is_capture()) {
                square_set(to, state.piece_captured, nnue);
            }

            states.pop_back();
        }

        void load(const std::string &fen, bool validate_fen = false) {

            if (validate_fen && !is_valid_fen(fen)) {
                print("info", "error", "Invalid fen:", fen);
                return;
            }

            board_clear();

            std::stringstream ss(fen);
            std::string pieces, stm, rights, ep, move50;
            ss >> pieces >> stm >> rights >> ep >> move50;

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
                throw std::runtime_error("Invalid fen string: " + fen);
            }

            state.rights = CastlingRights(rights);
            state.ep = square_from_string(ep);

            if (!move50.empty() && std::all_of(move50.begin(), move50.end(), ::isdigit)) {
                state.move50 = std::stoi(move50);
            } else {
                state.move50 = 0;
            }

            state.hash.xor_castle(state.rights);
            if (state.ep != NULL_SQUARE) {
                state.hash.xor_ep(state.ep);
            }
        }

        [[nodiscard]] std::string get_fen() const {
            std::string fen;

            Square square = A8;
            int empty = 0;
            while (true) {
                if (mailbox[square].is_null()) {
                    empty++;
                } else {
                    if (empty) fen += std::to_string(empty);
                    fen += char_from_piece(mailbox[square]);
                    empty = 0;
                }

                if (square == H1) {
                    break;
                }

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
            fen += get_rights().to_string();
            fen += " ";
            fen += get_ep() == NULL_SQUARE ? "-" : format_square(get_ep());
            fen += " ";
            fen += std::to_string(get_move50());
            return fen;
        }

        void display() const {
            std::vector<std::string> text;
            text.emplace_back("50-move draw counter: " + std::to_string(state.move50));
            text.emplace_back("Hash: " + std::to_string(get_hash()));
            text.emplace_back("Fen: " + get_fen());
            text.emplace_back("Castling rights: " + get_rights().to_string());
            text.emplace_back("Side to move: " + std::string(get_stm() == WHITE ? "White" : "Black"));

            if (get_ep() != NULL_SQUARE) {
                text.emplace_back("En passant square: " + format_square(get_ep()));
            }

            std::cout << "\n     A   B   C   D   E   F   G   H  \n";
            std::cout << "   ╭───┬───┬───┬───┬───┬───┬───┬───╮";
            for (int i = 8; i >= 1; i--) {
                if (i <= 7 && !text.empty()) {
                    std::cout << "        " << text.back();
                    text.pop_back();
                }
                std::cout << "\n " << i << " │";
                for (int j = 1; j <= 8; j++) {
                    Piece piece = piece_at(Square((i - 1) * 8 + (j - 1)));
                    std::cout << (piece.color == WHITE ? ASCII_WHITE_PIECE : (piece.color == BLACK ? ASCII_BLACK_PIECE : "")) << " " << char_from_piece(piece) << " \u001b[0m│";
                }
                if (i <= 7 && !text.empty()) {
                    std::cout << "        " << text.back();
                    text.pop_back();
                }
                std::cout << "\n";
                if (i != 1) {
                    std::cout << "   ├───┼───┼───┼───┼───┼───┼───┼───┤";
                }
            }
            std::cout << "   ╰───┴───┴───┴───┴───┴───┴───┴───╯\n\n"
                      << std::flush;
        }

        [[nodiscard]] std::vector<unsigned int> to_features() const {
            std::vector<unsigned int> result;
            Bitboard bb = occupied();
            while (bb) {
                Square sq = bb.pop_lsb();
                Piece piece = piece_at(sq);
                result.emplace_back(nn::NNUE::get_feature_index(piece, sq));
            }
            return result;
        }

    private:
        Piece mailbox[64];
        Bitboard bb_pieces[6], bb_colors[2];

        std::vector<BoardState> states;

        void square_clear(Square square, nn::NNUE *nnue = nullptr) {
            const Piece piece = piece_at(square);
            if (piece.is_null()) return;

            bb_colors[piece.color].clear(square);
            bb_pieces[piece.type].clear(square);
            mailbox[square] = NULL_PIECE;

            state.hash.xor_piece(square, piece);

            if (nnue) nnue->deactivate(piece, square);
        }

        void square_set(Square square, Piece piece, nn::NNUE *nnue = nullptr) {
            assert(piece.is_ok());
            square_clear(square, nnue);

            bb_colors[piece.color].set(square);
            bb_pieces[piece.type].set(square);
            mailbox[square] = piece;

            state.hash.xor_piece(square, piece);

            if (nnue) nnue->activate(piece, square);
        }

        void move_piece(Piece piece, Square from, Square to, nn::NNUE *nnue = nullptr) {
            assert(piece.is_ok());

            square_clear(from, nnue);
            square_set(to, piece, nnue);
        }

        void board_clear() {

            for (Bitboard &i : bb_pieces) i = 0;
            for (Bitboard &i : bb_colors) i = 0;

            for (Square square = A1; square < 64; square += 1) {
                mailbox[square] = NULL_PIECE;
            }

            states.clear();
            states.emplace_back();
        }

        static bool is_valid_fen(const std::string &fen) {
            const static std::regex fen_regex("^"
                                              "([rnbqkpRNBQKP1-8]+\\/){7}"
                                              "([rnbqkpRNBQKP1-8]+)"
                                              " [bw]"
                                              " ([-KQkq]+|)"
                                              " (([a-h][36])|-)"
                                              " \\d+"
                                              ".*");
            return std::regex_match(fen, fen_regex);
        }
    };


    Bitboard get_all_attackers(const Board &board, Square square, Bitboard occ) {
        return ((masks_pawn[square][BLACK] & board.pieces<WHITE, PAWN>()) |
                (masks_pawn[square][WHITE] & board.pieces<BLACK, PAWN>()) |
                (attacks_piece<KNIGHT>(square, occ) & board.pieces<KNIGHT>()) |
                (attacks_piece<BISHOP>(square, occ) & board.pieces<BISHOP>()) |
                (attacks_piece<ROOK>(square, occ) & board.pieces<ROOK>()) |
                (attacks_piece<QUEEN>(square, occ) & board.pieces<QUEEN>()) |
                (attacks_piece<KING>(square, occ) & board.pieces<KING>())) &
               occ;
    }

    Bitboard least_valuable_piece(const Board &board, Bitboard attackers, Color stm, PieceType &type) {
        for (PieceType t : PIECE_TYPES_BY_VALUE) {
            Bitboard s = attackers & board.pieces(stm, t);
            if (s) {
                type = t;
                return s & -s.bb;
            }
        }
        return 0;
    }
} // namespace chess

#undef state
