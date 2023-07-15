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

#include "../core/attacks.h"
#include "../core/board.h"
#include "../core/constants.h"

#include "qnetwork.h"

namespace nn {

    constexpr Score pawn_table[64] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            50,  50,  50,  50,  50,  50,  50,  50,
            15,  20,  20,  25,  25,  20,  20,  15,
            5,  10,  10,  15,  15,  10,  10,   5,
            0,   0,   0,  20,  20,   0,   0,   0,
            5,  -5,  -5,   0,   0,  -5,  -5,   5,
            5,  10,  10, -20, -20,  10,  10,   5,
            0,   0,   0,   0,   0,   0,   0,   0,
    };

    constexpr Score bishop_table[64] = {
            0,    0,   0,   0,   0,   0,   0,   0,
            0,    0,   0,   0,   0,   0,   0,   0,
            0,    0,   0,   0,   0,   0,   0,   0,
            -5,    0,   0,   0,   0,   0,   0,  -5,
            -10,   0,   0,   0,   0,   0,   0, -10,
            -20,   0,   0,   0,   0,   0,   0, -20,
            -30,   0,   0,   0,   0,   0,   0, -30,
            -50, -40, -40, -40, -40, -40, -40, -50,
    };

    constexpr Score mg_king_table[64] = {
            -50, -50, -50, -50, -50, -50, -50, -50,
            -50, -50, -50, -50, -50, -50, -50, -50,
            -40, -40, -40, -40, -40, -40, -40, -40,
            -30, -30, -30, -35, -35, -30, -30, -30,
            -20, -20, -20, -30, -30, -20, -20, -20,
            -10, -20, -20, -20, -20, -20, -20, -10,
            20,  20,   0,   0,   0,   0,  20,  20,
            50,  50,  10,   0,   0,  10,  50,  50,
    };

    constexpr Score eg_king_table[64] = {
            -50, -30, -30, -30, -30, -30, -30, -50,
            -30, -20, -10,   0,   0, -10, -20, -30,
            -30, -10,  20,  30,  30,  20, -10, -30,
            -30,   0,  30,  40,  40,  30,   0, -30,
            -30,   0,  30,  40,  40,  30,   0, -30,
            -30, -10,  20,  30,  30,  20, -10, -30,
            -30, -20, -10,   0,   0, -10, -20, -30,
            -50, -30, -30, -30, -30, -30, -30, -50,
    };

    constexpr Score king_safety_table[50] = {
            1, 2, 3, 5, 7, 9, 12, 15, 18, 22,
            26, 30, 35, 40, 45, 51, 57, 63, 70, 77,
            84, 92, 100, 108, 117, 126, 135, 145, 155, 165,
            176, 187, 198, 210, 222, 234, 247, 260, 273, 287,
            301, 315, 330, 345, 360, 376, 392, 408, 425, 442
    };

    constexpr Score mobility_weight[6] = {
            0, 0, 2, 2, 1, 1
    };

    constexpr int attack_weight[6] = {
            0, 1, 2, 3, 4, 5
    };

    template<Color color>
    inline Score evaluate_king_safety(const core::Board &board, Square king, int attacked) {
        core::Bitboard pawn_shield_1, pawn_shield_2;
        if constexpr (color == WHITE) {
            pawn_shield_1 = core::step<NORTH_EAST>(king) | core::step<NORTH>(king) | core::step<NORTH_WEST>(king);
            pawn_shield_2 = core::step<NORTH>(pawn_shield_1);
        } else {
            pawn_shield_1 = core::step<SOUTH_EAST>(king) | core::step<SOUTH>(king) | core::step<SOUTH_WEST>(king);
            pawn_shield_2 = core::step<SOUTH>(pawn_shield_1);
        }

        pawn_shield_1 &= board.pieces<color, PAWN>();
        pawn_shield_2 &= board.pieces<color, PAWN>();

        Score king_safety = 0;
        king_safety += 15 * pawn_shield_1.pop_count();
        king_safety += 5 * pawn_shield_2.pop_count();

        attacked -= 2 * pawn_shield_1.pop_count();
        attacked -= 1 * pawn_shield_2.pop_count();

        attacked = std::clamp(attacked, 0, 49);

        king_safety -= king_safety_table[attacked];

        return king_safety;
    }

    inline Score hce(const core::Board &board) {

        Score score = 0;
        score += 100 * (board.pieces<WHITE, PAWN>().pop_count() - board.pieces<BLACK, PAWN>().pop_count());
        score += 300 * (board.pieces<WHITE, KNIGHT>().pop_count() - board.pieces<BLACK, KNIGHT>().pop_count());
        score += 320 * (board.pieces<WHITE, BISHOP>().pop_count() - board.pieces<BLACK, BISHOP>().pop_count());
        score += 500 * (board.pieces<WHITE, ROOK>().pop_count() - board.pieces<BLACK, ROOK>().pop_count());
        score += 900 * (board.pieces<WHITE, QUEEN>().pop_count() - board.pieces<BLACK, QUEEN>().pop_count());

        int phase = 0;
        int king_attack_score[2] = {0, 0};
        core::Bitboard occ = board.occupied();

        Square wk = board.pieces<WHITE, KING>().lsb();
        Square wk_flip = square_flip(wk);
        Square bk = board.pieces<BLACK, KING>().lsb();

        for (Color color : {WHITE, BLACK}) {
            Score piece_score = 0;

            core::Bitboard enemy_pawns = board.pieces<PAWN>(color_enemy(color));
            Square enemy_king_sq = (color == WHITE ? bk : wk);

            core::Bitboard temp = core::step<EAST>(enemy_king_sq) | core::Bitboard(enemy_king_sq) | core::step<WEST>(enemy_king_sq);
            core::Bitboard enemy_king_zone = temp;
            Direction forward = (color == WHITE ? SOUTH : NORTH);
            for (int i = 0; i < 2; i++) {
                temp = core::step(forward, temp);
                enemy_king_zone |= temp;
            }

            for (PieceType pt : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN}) {
                core::Bitboard bb = board.pieces(color, pt);

                if (pt == BISHOP && bb.pop_count() == 2) piece_score += 50;

                while (bb) {
                    phase += PIECE_TO_PHASE_INT[pt];
                    Square sq = bb.pop_lsb();
                    Square pov_sq = color == WHITE ? square_flip(sq) : sq;

                    // PST
                    if (pt == PAWN) {
                        piece_score += pawn_table[pov_sq];

                        core::Bitboard passed = core::masks_passed_pawn[sq][color];
                        if (!(passed & enemy_pawns))
                            piece_score += 20;
                    } else if (pt == BISHOP) {
                        piece_score += bishop_table[pov_sq];
                    } else if (pt == ROOK) {
                        core::Bitboard pawns = core::masks_file[sq] & board.pieces<PAWN>();
                        int pawn_count = pawns.pop_count();
                        if (pawn_count == 0) {
                            piece_score += 15;
                        } else if (pawn_count == 1) {
                            piece_score += 5;
                        }
                    }

                    // Mobility
                    if (pt != PAWN) {
                        core::Bitboard attacks = core::attacks_piece(pt, sq, occ);
                        piece_score += mobility_weight[pt] * attacks.pop_count();
                        king_attack_score[color] += attack_weight[pt] * (enemy_king_zone & attacks).pop_count();
                    }
                }
            }
            if (color == WHITE) score += piece_score;
            else score -= piece_score;
        }

        if (((wk == F1 || wk == G1) && board.piece_at(H1) == Piece(ROOK, WHITE)) || ((wk == B1 || wk == C1) && board.piece_at(A1) == Piece(ROOK, WHITE)))
            score -= 50;
        if (((bk == F8 || bk == G8) && board.piece_at(H8) == Piece(ROOK, BLACK)) || ((bk == B8 || bk == C8) && board.piece_at(A8) == Piece(ROOK, BLACK)))
            score += 50;

        Score mg_king_score = mg_king_table[wk_flip] - mg_king_table[bk];
        Score eg_king_score = eg_king_table[wk_flip] - eg_king_table[bk];

        mg_king_score += evaluate_king_safety<WHITE>(board, wk, king_attack_score[BLACK]);
        mg_king_score -= evaluate_king_safety<BLACK>(board, bk, king_attack_score[WHITE]);

        Score king_score = (mg_king_score * (phase) + eg_king_score * (64 - phase)) / 64;

        score += king_score;

        if (board.get_stm() == WHITE)
            return score;
        else
            return -score;
    }

    extern QNetwork net;

    inline Score eval(const core::Board &board) {

        core::Bitboard bb = board.occupied();
        std::vector<unsigned int> features;
        float phase = 0.0;
        while (bb) {
            Square sq = bb.pop_lsb();
            Piece piece = board.piece_at(sq);
            phase += PIECE_TO_PHASE[piece.type];
            features.emplace_back(QNetwork::get_feature_index(piece, sq));
        }
        if (board.get_stm() == WHITE)
            return net.forward(features, phase);
        else
            return -net.forward(features, phase);
    }
}
