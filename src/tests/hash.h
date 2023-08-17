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

#include "../chess/board.h"
#include "../uci/uci.h"

#include <utility>
#include <vector>

namespace test {

    void test_hash() {

        struct Test {
            std::string fen;
            std::vector<std::string> moves;
            chess::Zobrist hash;

            Test(std::string fen, std::vector<std::string> moves, uint64_t hash) : fen(std::move(fen)), moves(std::move(moves)), hash(hash) {}
        };

        const std::vector<Test> tests = {
                Test(STARTING_FEN, {"d2d3", "d7d6", "e2e3", "e7e6"}, 12689034350543171487ULL),
                Test(STARTING_FEN, {"e2e3", "e7e6", "d2d3", "d7d6"}, 12689034350543171487ULL),
                Test(STARTING_FEN, {"e2e4", "e7e5", "d2d4", "d7d5"}, 12894059078872434213ULL),
                Test(STARTING_FEN, {"d2d4", "d7d5", "e2e4", "e7e5"}, 2922744524688730821ULL),
                Test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", {}, 2177831812586383056ULL),
                Test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1", {}, 3914531219827666716ULL),
                Test("rnbqk2r/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1", {"e1g1"}, 730654048443189168ULL),
                Test("rnbqk2r/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1", {"e1g1", "e8g8"}, 9794721467020975390ULL)};

        chess::Board board;
        std::vector<Test> failed;

        for (const Test &test : tests) {
            board.load(test.fen);
            for (std::string str : test.moves) {
                chess::Move move = chess::move_from_string(board, str);
                board.make_move(move);
            }
            if (board.get_hash() != test.hash) {
                failed.emplace_back(test);
            }
        }

        if (failed.empty()) {
            std::cout << "All hash test have passed!" << std::endl;
        } else {
            std::cout << failed.size() << " hash test have failed:" << std::endl;
            for (const Test &test : failed) {
                std::cout << test.fen << std::endl;
            }
            std::abort();
        }
    }

} // namespace test