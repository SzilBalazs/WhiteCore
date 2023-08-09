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

#include "../chess/movegen.h"

#include <random>
#include <string>
#include <sstream>

namespace rng {

    std::string gen_id() {

        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<int> dist(0, 15);

        std::stringstream ss;
        ss << std::hex;
        for (int i = 0; i < 8; i++) {
            ss << dist(gen);
        }

        return ss.str();
    }

    std::string gen_fen(size_t depth = 8u) {

        static std::random_device rd;
        static std::mt19937 gen(rd());

        chess::Board board;
        board.load(STARTING_FEN);

        for (size_t i = 0; i < depth; i++) {
            chess::Move buffer[200];
            chess::Move *end_ptr = chess::gen_moves(board, buffer, false);
            size_t length = end_ptr - buffer;

            // If a checkmate/stalemate is reached we instead generate an entirely new fen
            if (length == 0) {
                return gen_fen();
            }

            std::uniform_int_distribution<size_t> dist(0, length - 1);
            size_t random_index = dist(gen);

            board.make_move(buffer[random_index]);
        }

        return board.get_fen();
    }
}
