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
