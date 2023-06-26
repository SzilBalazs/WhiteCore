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

#include <utility>

#include "../core/movegen.h"

namespace test {

    template<bool bulk_counting, bool output>
    int64_t perft(core::Board &board, int depth) {
        core::Move moves[200];
        core::Move *moves_end = core::gen_moves(board, moves, false);

        // Bulk counting the number of moves at depth 1.
        if (depth == 1 && bulk_counting)
            return moves_end - moves;
        if (depth == 0)
            return 1;

        // DFS like routine, calling itself recursively with lowered depth.
        int64_t nodes = 0;
        for (core::Move *it = moves; it != moves_end; it++) {
            board.make_move(*it);
            int64_t node_count = perft<bulk_counting, false>(board, depth - 1);
            if constexpr (output) {
                std::cout << *it << ": " << node_count << std::endl; // Used for debugging purposes.
            }
            nodes += node_count;
            board.undo_move(*it);
        }
        return nodes;
    }

    inline void test_perft() {

        struct Test {
            std::string fen;
            int depth;
            int64_t expected;

            Test(std::string fen, int depth, int64_t expected) : fen(std::move(fen)), depth(depth), expected(expected) {}
        };

        std::vector<Test> tests = {
                Test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ", 6, 119060324),
                Test("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 5, 193690690),
                Test("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 6, 11030083),
                Test("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1 ", 5, 15833292),
                Test("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ", 5, 15833292),
                Test("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 ", 5, 89941194),
                Test("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ", 5, 164075551)};

        std::cout << "Testing perft..." << std::endl;
        core::Board board;
        std::vector<Test> failed;
        int64_t start_time = now(), total_nodes = 0;

        for (const Test &test : tests) {
            board.load(test.fen);
            std::cout << "Running " << test.fen << "...\r" << std::flush;
            int64_t node_count = perft<true, false>(board, test.depth);
            total_nodes += node_count;
            if (node_count != test.expected) {
                failed.emplace_back(test);
            }
        }
        std::cout << std::endl;

        int64_t end_time = now();
        int64_t nps = calculate_nps(end_time - start_time, total_nodes);

        if (failed.empty()) {
            std::cout << "All perft test have passed! " << nps << " nps" << std::endl;
        } else {
            std::cout << failed.size() << " perft test have failed:" << std::endl;
            for (const Test &test : failed) {
                std::cout << test.fen << std::endl;
            }
        }
    }

} // namespace test
