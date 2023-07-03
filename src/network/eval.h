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

    Score pawn_table[64] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            50,  50,  50,  50,  50,  50,  50,  50,
            15,  20,  20,  25,  25,  20,  20,  15,
            5,  10,  10,  15,  15,  10,  10,   5,
            0,   0,   0,  20,  20,   0,   0,   0,
            5,  -5,  -5,   0,   0,  -5,  -5,   5,
            5,  10,  10, -20, -20,  10,  10,   5,
            0,   0,   0,   0,   0,   0,   0,   0,
    };

    Score bishop_table[64] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            -5,   0,   0,   0,   0,   0,   0,  -5,
            -10,   0,   0,   0,   0,   0,   0, -10,
            -20,   0,   0,   0,   0,   0,   0, -20,
            -30,   0,   0,   0,   0,   0,   0, -30,
            -50, -40, -40, -40, -40, -40, -40, -50,
    };

    Score mg_king_table[64] = {
            -50,-50,-50,-50,-50,-50,-50,-50,
            -50,-50,-50,-50,-50,-50,-50,-50,
            -40,-40,-40,-40,-40,-40,-40,-40,
            -30,-30,-30,-35,-35,-30,-30,-30,
            -20,-20,-20,-30,-30,-20,-20,-20,
            -10,-20,-20,-20,-20,-20,-20,-10,
            20, 20,  0,  0,  0,  0, 20, 20,
            50, 50, 10,  0,  0, 10, 50, 50,
    };

    Score eg_king_table[64] = {
            -50,-30,-30,-30,-30,-30,-30,-50,
            -30,-20,-10,  0,  0,-10,-20,-30,
            -30,-10, 20, 30, 30, 20,-10,-30,
            -30,  0, 30, 40, 40, 30,  0,-30,
            -30,  0, 30, 40, 40, 30,  0,-30,
            -30,-10, 20, 30, 30, 20,-10,-30,
            -30,-20,-10,  0,  0,-10,-20,-30,
            -50,-30,-30,-30,-30,-30,-30,-50,
    };

    inline Score hce(const core::Board &board) {

        Score score = 0;
        score += 100 * (board.pieces<WHITE, PAWN>().pop_count() - board.pieces<BLACK, PAWN>().pop_count());
        score += 300 * (board.pieces<WHITE, KNIGHT>().pop_count() - board.pieces<BLACK, KNIGHT>().pop_count());
        score += 350 * (board.pieces<WHITE, BISHOP>().pop_count() - board.pieces<BLACK, BISHOP>().pop_count());
        score += 500 * (board.pieces<WHITE, ROOK>().pop_count() - board.pieces<BLACK, ROOK>().pop_count());
        score += 900 * (board.pieces<WHITE, QUEEN>().pop_count() - board.pieces<BLACK, QUEEN>().pop_count());

        int phase = 0;
        core::Bitboard occ = board.occupied();

        for (Color color : {WHITE, BLACK}) {
            Score piece_score = 0;
            for (PieceType pt : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN}) {
                core::Bitboard bb = board.pieces(color, pt);
                while (bb) {
                    phase += PIECE_TO_PHASE_INT[pt];
                    Square sq = bb.pop_lsb();
                    Square pov_sq = color == WHITE ? square_flip(sq) : sq;

                    // PST
                    if (pt == PAWN) {
                        piece_score += pawn_table[pov_sq];
                    } else if (pt == BISHOP) {
                        piece_score += bishop_table[pov_sq];
                    }

                    // Mobility
                    if (pt != PAWN)
                        piece_score += attacks_piece(pt, sq, occ).pop_count();
                }
            }
            if (color == WHITE) score += piece_score;
            else score -= piece_score;
        }


        Square wk = board.pieces<WHITE, KING>().lsb();
        Square wk_flip = square_flip(wk);
        Square bk = board.pieces<BLACK, KING>().lsb();

        Score mg_king_score = mg_king_table[wk_flip] - mg_king_table[bk];
        Score eg_king_score = eg_king_table[wk_flip] - eg_king_table[bk];

        core::Bitboard wshield = core::step<NORTH_EAST>(wk) | core::step<NORTH>(wk) | core::step<NORTH_WEST>(wk);
        core::Bitboard bshield = core::step<SOUTH_EAST>(bk) | core::step<SOUTH>(bk) | core::step<SOUTH_WEST>(bk);

        wshield &= board.pieces<WHITE, PAWN>();
        bshield &= board.pieces<BLACK, PAWN>();

        mg_king_score += wshield.pop_count() * 30;
        mg_king_score -= bshield.pop_count() * 30;

        Score king_score = (mg_king_score * (phase) + eg_king_score * (64 - phase)) / 64;

        score += king_score;

        if (board.get_stm() == WHITE)
            return score;
        else
            return -score;
    }

    extern QNetwork net;

    inline Score eval(const core::Board &board) {

        return hce(board);

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
