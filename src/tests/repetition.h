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

#include "../core/board.h"
#include "../uci/uci.h"

#include <vector>

namespace test {

    void test_repetition() {

        struct Test {
            std::string fen;
            std::vector<std::string> moves;

            Test(std::string fen, std::vector<std::string> moves) : fen(std::move(fen)), moves(std::move(moves)) {}
        };

        const std::vector<Test> tests = {
                Test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", {"g1f3", "b8c6", "f3g1", "c6b8"}),
                Test("7k/2R5/2P1pp1p/2K5/7q/8/6R1/1q6 w - - 0 1", {"c7c8", "h8h7", "c8c7", "h7h8", "c7c8", "h8h7", "c8c7", "h7h8"})};

        core::Board board;
        std::vector<Test> failed;

        for (const Test &test : tests) {
            board.load(test.fen);
            for (const std::string& str : test.moves) {
                core::Move move = uci::move_from_string(board, str);
                board.make_move(move);
            }
            if (!board.is_draw()) {
                failed.emplace_back(test);
            }
        }

        if (failed.empty()) {
            std::cout << "All repetition test have passed!" << std::endl;
        } else {
            std::cout << failed.size() << " repetition test have failed:" << std::endl;
            for (const Test &test : failed) {
                std::cout << test.fen << std::endl;
            }
            std::abort();
        }
    }

} // namespace test