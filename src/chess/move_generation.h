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

#include "attacks.h"
#include "board.h"

namespace chess {

    // Returns a bitboard of all the squares attacking a given square for the given color
    template<Color color>
    [[nodiscard]] Bitboard get_attackers(const Board &board, Square square) {
        constexpr Color enemy_color = color_enemy<color>();

        Bitboard occupied = board.occupied();
        Bitboard enemy = board.sides<enemy_color>();
        return ((masks_pawn[square][color] & board.pieces<PAWN>()) |
                (attacks_piece<KNIGHT>(square, occupied) & board.pieces<KNIGHT>()) |
                (attacks_piece<BISHOP>(square, occupied) & board.pieces<BISHOP>()) |
                (attacks_piece<ROOK>(square, occupied) & board.pieces<ROOK>()) |
                (attacks_piece<QUEEN>(square, occupied) & board.pieces<QUEEN>())) &
               enemy;
    }

    // Returns a bitboard of all the squares attacking a given square for the side to move
    [[nodiscard]] Bitboard get_attackers(const Board &board, Square square) {
        if (board.get_stm() == WHITE)
            return get_attackers<WHITE>(board, square);
        else
            return get_attackers<BLACK>(board, square);
    }

    // Generates all promotion moves for a pawn from 'from' to 'to'
    // and adds them to the move list 'moves'
    [[nodiscard]] Move *make_promo(Move *moves, Square from, Square to) {
        *moves++ = Move(from, to, Move::PROMO_KNIGHT);
        *moves++ = Move(from, to, Move::PROMO_BISHOP);
        *moves++ = Move(from, to, Move::PROMO_ROOK);
        *moves++ = Move(from, to, Move::PROMO_QUEEN);
        return moves;
    }

    // Generates all promotion captures for a pawn from 'from' to 'to'
    // and adds them to the move list 'moves'
    [[nodiscard]] Move *make_promo_capture(Move *moves, Square from, Square to) {
        *moves++ = Move(from, to, Move::PROMO_CAPTURE_KNIGHT);
        *moves++ = Move(from, to, Move::PROMO_CAPTURE_BISHOP);
        *moves++ = Move(from, to, Move::PROMO_CAPTURE_ROOK);
        *moves++ = Move(from, to, Move::PROMO_CAPTURE_QUEEN);
        return moves;
    }

    // Returns a bitboard of all the squares attacked by a given color
    // 'board' is the board, 'occupied' is a bitboard of all the occupied squares
    template<Color color>
    [[nodiscard]] Bitboard get_attacked_squares(const Board &board, Bitboard occupied) {

        constexpr Direction UP_LEFT = color == WHITE ? NORTH_WEST : -NORTH_WEST;
        constexpr Direction UP_RIGHT = color == WHITE ? NORTH_EAST : -NORTH_EAST;

        // Get the pawns of the given color and all other pieces
        Bitboard pawns = board.pieces<color, PAWN>();
        Bitboard pieces = board.sides<color>() & ~pawns;

        // Initially add the attacks of the pawns
        Bitboard result = step<UP_LEFT>(pawns) | step<UP_RIGHT>(pawns);

        // Iterate through all other pieces and add their attacks
        while (pieces) {
            Square from = pieces.pop_lsb();
            result |= attacks_piece<color>(board.piece_at(from).type, from, occupied);
        }

        return result;
    }

    // Generates all legal moves for the given pieces
    // 'captures_only' - if true, only generates capture moves
    // 'pinHV' - if true, pieces are pinned horizontally or vertically
    // 'pinDA' - if true, pieces are pinned diagonally or anti-diagonally
    // 'pieces' - bitboard of the pieces to generate moves for
    // 'mask_special' - a bitboard indicating the squares to generate moves to
    // 'occupied' - a bitboard of all occupied squares
    // 'empty' - a bitboard of all empty squares
    // 'enemy' - a bitboard of all enemy pieces
    template<bool captures_only, bool pinHV, bool pinDA>
    [[nodiscard]] Move *gen_moves_from_pieces(const Board &board, Move *moves, Bitboard pieces, Bitboard mask_special,
                                              Bitboard occupied, Bitboard empty, Bitboard enemy) {

        // Iterate through the pieces
        while (pieces) {

            Square from = pieces.pop_lsb();
            PieceType type = board.piece_at(from).type;

            // Get the attacks of the piece and filter by the special mask
            Bitboard attacks = attacks_piece(type, from, occupied) & mask_special;

            // Check if the piece is pinned horizontally or vertically
            if constexpr (pinHV)
                attacks &= masks_rook[from];

            // Check if the piece is pinned diagonally
            if constexpr (pinDA)
                attacks &= masks_bishop[from];

            // If we're not generating captures only, generate quiet moves
            if constexpr (!captures_only) {
                Bitboard quiets = attacks & empty;
                while (quiets) {
                    *moves++ = Move(from, quiets.pop_lsb());
                }
            }

            // Generate captures
            Bitboard captures = attacks & enemy;
            while (captures) {
                Square to = captures.pop_lsb();
                *moves++ = Move(from, to, Move::CAPTURE);
            }
        }

        return moves;
    }

    // Generates all legal pawn moves for all the 'color' pawns in 'board' board and
    // adds them to the 'moves' move list.
    // 'captures_only' - if true, only generates capture moves
    // 'king' - the square where the friendly king is
    // 'mask_check' - a bitboard indicating to squares which evade check
    // 'moveH' - a bitboard indicating pieces which can move horizontally
    // 'moveV' - a bitboard indicating pieces which can move vertically
    // 'moveD' - a bitboard indicating pieces which can move diagonally
    // 'moveA' - a bitboard indicating pieces which can move anti-diagonally
    template<Color color, bool captures_only>
    [[nodiscard]] Move *gen_pawn_moves(const Board &board, Move *moves, Square king, Bitboard mask_check,
                                       Bitboard moveH, Bitboard moveV, Bitboard moveD, Bitboard moveA) {

        // Define enemy color
        constexpr Color enemy_color = color_enemy<color>();

        // Define move directions
        constexpr Direction UP = color == WHITE ? NORTH : -NORTH;
        constexpr Direction UP_LEFT = color == WHITE ? NORTH_WEST : -NORTH_WEST;
        constexpr Direction UP_RIGHT = color == WHITE ? NORTH_EAST : -NORTH_EAST;
        constexpr Direction DOWN = -UP;
        constexpr Direction DOWN_LEFT = -UP_RIGHT;
        constexpr Direction DOWN_RIGHT = -UP_LEFT;

        // Define ranks for double pawn push and promotion
        constexpr Bitboard rank_double_push = (color == WHITE ? RANK_3 : RANK_6);
        constexpr Bitboard rank_before_promo = (color == WHITE ? RANK_7 : RANK_2);

        // Create a bitboard of squares not on the promotion rank
        constexpr Bitboard not_before_promo = ~rank_before_promo;

        // Get the en passant square
        Square square_ep = board.get_ep();

        // Get the empty and the enemy bitboards
        Bitboard empty = board.empty();
        Bitboard enemy = board.sides<enemy_color>();

        // Get the bitboard of pawns to generate moves for
        Bitboard pawns = board.pieces<color, PAWN>();

        // Get the bitboard of pawns before promotion
        Bitboard pawns_before_promo = rank_before_promo & pawns;
        pawns &= not_before_promo;

        // Generate quiet moves
        if constexpr (!captures_only) {

            Bitboard single_push = step<UP>(pawns & moveH) & empty;                  // Generates single pawn pushes
            Bitboard double_push = step<UP>(single_push & rank_double_push) & empty; // Generates double pawn pushes

            // Filter out moves that are within the 'mask_check'
            single_push &= mask_check;
            double_push &= mask_check;

            // Iterate through single pawn pushes
            while (single_push) {
                Square to = single_push.pop_lsb();
                *moves++ = Move(to + DOWN, to);
            }

            // Iterate through double pawn pushes
            while (double_push) {
                Square to = double_push.pop_lsb();
                *moves++ = Move(to + (2 * DOWN), to, Move::DOUBLE_PAWN_PUSH);
            }
        }

        // Filter out all the legal capture moves to the right
        Bitboard captures_right = step<UP_RIGHT>(pawns & moveD) & enemy & mask_check;
        // Filter out all the legal capture moves to the left
        Bitboard captures_left = step<UP_LEFT>(pawns & moveA) & enemy & mask_check;

        // Iterate through left captures
        while (captures_left) {
            Square to = captures_left.pop_lsb();
            *moves++ = Move(to + DOWN_RIGHT, to, Move::CAPTURE);
        }

        // Iterate through right captures
        while (captures_right) {
            Square to = captures_right.pop_lsb();
            *moves++ = Move(to + DOWN_LEFT, to, Move::CAPTURE);
        }

        // Check if there are any pawns that can be promoted
        if (pawns_before_promo) {

            // Generate quiet moves
            if constexpr (!captures_only) {
                // Filter out all the legal promotions upwards
                Bitboard promo_up = step<UP>(pawns_before_promo & moveH) & empty & mask_check;

                while (promo_up) {
                    Square to = promo_up.pop_lsb();
                    moves = make_promo(moves, to + DOWN, to);
                }
            }

            // Filter out all the legal sideways capture-promotions
            Bitboard promo_right = step<UP_RIGHT>(pawns_before_promo & moveD) & enemy & mask_check;
            Bitboard promo_left = step<UP_LEFT>(pawns_before_promo & moveA) & enemy & mask_check;

            while (promo_right) {
                Square to = promo_right.pop_lsb();
                moves = make_promo_capture(moves, to + DOWN_LEFT, to);
            }

            while (promo_left) {
                Square to = promo_left.pop_lsb();
                moves = make_promo_capture(moves, to + DOWN_RIGHT, to);
            }
        }

        // Check if the square_ep is not empty, if there is an en-passantable pawn and if the square_ep is within the check mask
        if ((square_ep != NULL_SQUARE) && (masks_pawn[board.get_ep()][enemy_color] & pawns) &&
            mask_check.get(square_ep + DOWN)) {

            // Get the occupied squares
            Bitboard occ = board.occupied();

            // Check if there is a pawn on the right side of the square_ep that can move diagonally
            bool ep_right = (step<UP_RIGHT>(pawns & moveD)).get(square_ep);
            // Check if there is a pawn on the right side of the square_ep that can move anti-diagonally
            bool ep_left = (step<UP_LEFT>(pawns & moveA)).get(square_ep);

            // If there is a pawn on the right side
            if (ep_right) {
                Square pawn_attacking = square_ep + DOWN_LEFT;
                Square pawn_attacked = square_ep + DOWN;

                occ.clear(pawn_attacking);
                occ.clear(pawn_attacked);

                Bitboard attack_rook = attacks_rook(pawn_attacked, occ);
                Bitboard attack_bishop = attacks_bishop(pawn_attacked, occ);

                Bitboard attack_rank = masks_rank[pawn_attacked] & attack_rook;
                Bitboard attack_diag = masks_diagonal[pawn_attacked] & attack_bishop;
                Bitboard attack_adiag = masks_anti_diagonal[pawn_attacked] & attack_bishop;

                Bitboard rank_seen_sliders = (board.pieces<enemy_color, QUEEN>() | board.pieces<enemy_color, ROOK>()) & attack_rank;
                Bitboard diag_seen_sliders =
                        (board.pieces<enemy_color, QUEEN>() | board.pieces<enemy_color, BISHOP>()) & attack_diag;
                Bitboard adiag_seen_sliders =
                        (board.pieces<enemy_color, QUEEN>() | board.pieces<enemy_color, BISHOP>()) & attack_adiag;

                bool rank_pin = attack_rank.get(king) && rank_seen_sliders;
                bool diag_pin = attack_diag.get(king) && diag_seen_sliders;
                bool adiag_pin = attack_adiag.get(king) && adiag_seen_sliders;

                if (!(rank_pin || diag_pin || adiag_pin))
                    *moves++ = Move(pawn_attacking, square_ep, Move::EP_CAPTURE);

                occ.set(pawn_attacking);
                occ.set(pawn_attacked);
            }

            // If there is a pawn on the left side
            if (ep_left) {
                Square pawn_attacking = square_ep + DOWN_RIGHT;
                Square pawn_attacked = square_ep + DOWN;

                occ.clear(pawn_attacking);
                occ.clear(pawn_attacked);

                Bitboard attack_rook = attacks_rook(pawn_attacked, occ);
                Bitboard attack_bishop = attacks_bishop(pawn_attacked, occ);

                Bitboard attack_rank = masks_rank[pawn_attacked] & attack_rook;
                Bitboard attack_diag = masks_diagonal[pawn_attacked] & attack_bishop;
                Bitboard attack_adiag = masks_anti_diagonal[pawn_attacked] & attack_bishop;

                Bitboard rank_seen_sliders = (board.pieces<enemy_color, QUEEN>() | board.pieces<enemy_color, ROOK>()) & attack_rank;
                Bitboard rank_seen_diag =
                        (board.pieces<enemy_color, QUEEN>() | board.pieces<enemy_color, BISHOP>()) & attack_diag;
                Bitboard rank_seen_adiag =
                        (board.pieces<enemy_color, QUEEN>() | board.pieces<enemy_color, BISHOP>()) & attack_adiag;

                bool rank_pin = attack_rank.get(king) && rank_seen_sliders;
                bool diag_pin = attack_diag.get(king) && rank_seen_diag;
                bool adiag_pin = attack_adiag.get(king) && rank_seen_adiag;

                if (!(rank_pin || diag_pin || adiag_pin))
                    *moves++ = Move(pawn_attacking, square_ep, Move::EP_CAPTURE);

                occ.set(pawn_attacking);
                occ.set(pawn_attacked);
            }
        }

        return moves;
    }

    // Generates all the legal king moves.
    template<bool captures_only>
    [[nodiscard]] Move *gen_king_moves(const Board &board, Move *moves, Square king,
                                       Bitboard squares_safe, Bitboard empty, Bitboard enemy) {

        // Calculate all safe squares that the king can move to
        Bitboard king_target = masks_king[king] & squares_safe;

        if constexpr (!captures_only) {

            // Generate all legal king moves to squares that are empty
            Bitboard king_quiets = king_target & empty;

            while (king_quiets) {
                *moves++ = Move(king, king_quiets.pop_lsb());
            }
        }

        // Generate all legal king moves to squares that contain enemy pieces
        Bitboard king_captures = king_target & enemy;

        while (king_captures) {
            Square to = king_captures.pop_lsb();
            *moves++ = Move(king, to, Move::CAPTURE);
        }

        return moves;
    }

    // Returns the "to" squares which evades check.
    [[nodiscard]] Bitboard gen_check_mask(const Board &board, Square king, Bitboard checkers) {
        unsigned int checks = checkers.pop_count();
        if (checks == 0) {
            return 0xffffffffffffffffULL;
        } else if (checks == 1) {
            Square checker = checkers.lsb();
            PieceType type = board.piece_at(checker).type;
            if (type == ROOK || type == BISHOP || type == QUEEN) {
                return checkers | masks_common_ray[king][checker];
            } else {
                return checkers;
            }
        } else {
            return 0;
        }
    }

    // Generates all the legal slider and knight moves using the gen_moves_from_pieces utility.
    template<bool captures_only>
    [[nodiscard]] Move *gen_slider_and_jumper(const Board &board, Move *moves, Bitboard pieces,
                                              Bitboard occupied, Bitboard empty, Bitboard enemy, Bitboard checkMask,
                                              Bitboard pinHV, Bitboard pinDA) {
        Bitboard pinnedHV = pinHV & pieces;
        Bitboard pinnedDA = pinDA & pieces;
        pieces &= ~(pinnedHV | pinnedDA);

        moves = gen_moves_from_pieces<captures_only, false, false>(board, moves, pieces, checkMask, occupied, empty, enemy);

        moves = gen_moves_from_pieces<captures_only, true, false>(board, moves, pinnedHV, checkMask & pinHV, occupied, empty,
                                                                  enemy);

        moves = gen_moves_from_pieces<captures_only, false, true>(board, moves, pinnedDA, checkMask & pinDA, occupied, empty,
                                                                  enemy);

        return moves;
    }

    // Generates all the legal moves in a board.
    template<Color color, bool captures_only>
    [[nodiscard]] Move *gen_moves(const Board &board, Move *moves) {
        // Define enemy color
        constexpr Color enemyColor = color_enemy<color>();

        // Define friendly king square
        Square king = board.pieces<color, KING>().lsb();
        assert(king != NULL_SQUARE);

        // Define bitboards used for move generation
        Bitboard pieces_friendly = board.sides<color>();
        Bitboard empty = board.empty();
        Bitboard enemy = board.sides<enemyColor>();
        Bitboard occupied = board.occupied();
        Bitboard checkers = get_attackers<color>(board, king);

        occupied.clear(king);
        Bitboard squares_safe = ~get_attacked_squares<enemyColor>(board, occupied);
        occupied.set(king);

        // Generate mask_check
        Bitboard mask_check = gen_check_mask(board, king, checkers);

        // Generate king moves
        moves = gen_king_moves<captures_only>(board, moves, king, squares_safe, empty, enemy);

        // If we are in a double check, only king moves are legal
        if (mask_check == 0)
            return moves;

        // Generate pinMasks
        Bitboard squares_seen = attacks_piece<QUEEN>(king, occupied);
        Bitboard possible_pins = squares_seen & pieces_friendly;

        occupied ^= possible_pins;

        // Get all the pinners
        Bitboard possible_pinners = (attacks_piece<QUEEN>(king, occupied) ^ squares_seen) & enemy;
        Bitboard pinners = ((attacks_piece<ROOK>(king, occupied) & board.pieces<ROOK>()) |
                            (attacks_piece<BISHOP>(king, occupied) & board.pieces<BISHOP>()) |
                            (attacks_piece<QUEEN>(king, occupied) & board.pieces<QUEEN>())) &
                           possible_pinners;

        // Define bitboards used for storing pin information
        Bitboard pinH, pinV, pinD, pinA, pinHV, pinDA, moveH, moveV, moveD, moveA;

        // Calculate pins
        while (pinners) {
            Square pinner = pinners.pop_lsb();
            LineType type = line_type[king][pinner];
            switch (type) {
                case HORIZONTAL:
                    pinH |= masks_common_ray[king][pinner] | pinner;
                    break;
                case VERTICAL:
                    pinV |= masks_common_ray[king][pinner] | pinner;
                    break;
                case DIAGONAL:
                    pinD |= masks_common_ray[king][pinner] | pinner;
                    break;
                case ANTI_DIAGONAL:
                    pinA |= masks_common_ray[king][pinner] | pinner;
                    break;
            }
        }

        pinHV = pinH | pinV;
        pinDA = pinD | pinA;

        pinH &= pieces_friendly;
        pinV &= pieces_friendly;
        pinD &= pieces_friendly;
        pinA &= pieces_friendly;

        moveH = ~(pinV | pinD | pinA);
        moveV = ~(pinH | pinD | pinA);
        moveD = ~(pinH | pinV | pinA);
        moveA = ~(pinH | pinV | pinD);

        occupied ^= possible_pins;

        // Generate pawn moves
        moves = gen_pawn_moves<color, captures_only>(board, moves, king, mask_check, moveH, moveV, moveD, moveA);

        // Generate knight and slider moves
        Bitboard pieces_slider_and_jumper = pieces_friendly & ~board.pieces<PAWN>();
        pieces_slider_and_jumper.clear(king);

        moves = gen_slider_and_jumper<captures_only>(board, moves, pieces_slider_and_jumper, occupied, empty, enemy,
                                                     mask_check, pinHV, pinDA);

        // Generate castling moves
        if constexpr (!captures_only) {
            if constexpr (color == WHITE) {
                if (board.get_rights()[CastlingRights::WHITE_KING] &&
                    (squares_safe & WK_CASTLE_SAFE) == WK_CASTLE_SAFE && (empty & WK_CASTLE_EMPTY) == WK_CASTLE_EMPTY) {

                    *moves++ = Move(E1, G1, Move::KING_CASTLE);
                }

                if (board.get_rights()[CastlingRights::WHITE_QUEEN] &&
                    (squares_safe & WQ_CASTLE_SAFE) == WQ_CASTLE_SAFE && (empty & WQ_CASTLE_EMPTY) == WQ_CASTLE_EMPTY) {

                    *moves++ = Move(E1, C1, Move::QUEEN_CASTLE);
                }
            } else {
                if (board.get_rights()[CastlingRights::BLACK_KING] &&
                    (squares_safe & BK_CASTLE_SAFE) == BK_CASTLE_SAFE && (empty & BK_CASTLE_EMPTY) == BK_CASTLE_EMPTY) {

                    *moves++ = Move(E8, G8, Move::KING_CASTLE);
                }

                if (board.get_rights()[CastlingRights::BLACK_QUEEN] &&
                    (squares_safe & BQ_CASTLE_SAFE) == BQ_CASTLE_SAFE && (empty & BQ_CASTLE_EMPTY) == BQ_CASTLE_EMPTY) {

                    *moves++ = Move(E8, C8, Move::QUEEN_CASTLE);
                }
            }
        }

        return moves;
    }

    // Wrapper around the stm template.
    template<bool captures_only>
    [[nodiscard]] Move *gen_moves(const Board &board, Move *moves) {
        if (board.get_stm() == WHITE) {
            return gen_moves<WHITE, captures_only>(board, moves);
        } else {
            return gen_moves<BLACK, captures_only>(board, moves);
        }
    }

    // Wrapper around captures only template.
    [[nodiscard]] Move *gen_moves(const Board &board, Move *moves, bool captures_only) {
        if (captures_only) {
            return gen_moves<true>(board, moves);
        } else {
            return gen_moves<false>(board, moves);
        }
    }

    bool Board::is_check() const {
        return bool(chess::get_attackers(*this, pieces<KING>(get_stm()).lsb()));
    }

    chess::Move move_from_string(const chess::Board &board, const std::string &str) {
        chess::Move moves[200];
        chess::Move *moves_end = chess::gen_moves(board, moves, false);
        for (chess::Move *it = moves; it != moves_end; it++) {
            if (it->to_uci() == str) {
                return *it;
            }
        }
        return chess::NULL_MOVE;
    }

} // namespace chess