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

#include "../search/search_manager.h"

#include <vector>

void run_bench() {
    std::vector<std::string> fens = {
            "r1bq1k1r/pp3pp1/2nP4/7p/3p4/6N1/PPPQ1PPP/2KR1B1R b - - 1 16",
            "3Q4/1p3p2/2ppk3/4p2r/2PbP2p/3P3P/rq1BKP2/3R4 w - - 6 32",
            "8/4k3/4p3/1R3pp1/6p1/4PqP1/5P2/1R4K1 w - - 20 68",
            "8/kpq1n1p1/p3p3/8/N1p4P/P3P2K/1P3QP1/8 b - - 6 39",
            "8/5p2/2k5/7p/P1n2P2/1N4K1/8/8 b - - 1 50",
            "3r2k1/pp1b1pp1/1b6/2r3q1/3N4/P1BQPPPp/1P3K1P/3RR3 b - - 8 30",
            "1r4k1/p1q1r1p1/4p2p/pP1p4/3P4/R1P5/4Q1PP/R5K1 w - - 6 32",
            "8/5Q2/P6K/8/6q1/6k1/8/8 b - - 1 106",
            "rn1q1rk1/ppb1npp1/2p1p2p/3p3P/3PP3/2NQBP2/PPP2P2/R3KBR1 b Q - 5 11",
            "r2q1rk1/1p3pp1/p1npbn1p/4p3/4P3/P1BB1N2/P2Q1PPP/R3R1K1 b - - 2 14",
            "b1rr2k1/p3ppbp/6p1/N1n3q1/1p1N4/1P3P1P/4Q1P1/B2RR2K w - - 2 27",
            "4R3/3n4/1p1r1kp1/5p1p/p1r2N1P/5PK1/6P1/4R3 w - - 0 40",
            "8/8/4p3/p2kP1p1/1pN1p1P1/1P2K1P1/2P5/2b5 w - - 8 40",
            "r1bqk2r/pp1n1pp1/2pb3p/4p3/2PPB3/P3BN2/1PQ2PPP/R3K2R b KQkq - 1 12",
            "8/5p1k/2N4p/1p1pB2K/1P1P2P1/5P2/n1n5/8 w - - 3 45",
            "1qnr4/7p/1nk1p3/3b3Q/3P4/B2N2P1/5P1P/1R4K1 b - - 12 34",
            "1r6/5k2/8/8/5P1K/1P6/6N1/8 b - - 22 67",
            "r3k2r/pp1b1ppp/3b1n2/2np4/8/2N1PN2/Pq1BBPPP/R2QK2R w KQkq - 0 13",
            "8/3K4/4pk1p/7P/4P3/8/8/8 b - - 4 70",
            "8/2p3p1/3n4/1p6/3kpPB1/PP6/2PK1P2/8 w - - 5 45",
            "r2qk2r/ppp1bppp/2np1n2/4p3/2B1P1b1/2NP1N2/PPPB1PPP/R2Q1RK1 b kq - 0 1",
            "2b5/7n/2Ppp1N1/B1pP2p1/2P2p2/pp1k1P2/1rp1NP1R/2b2B1K w - - 0 1",
            "r1b1k2r/pppn1p1p/5np1/q3p3/1bBP4/2N2Q2/PPPB1PPP/R3K1NR w KQkq - 4 9",
            "rn2kb1r/pp3p1p/2p2p2/3qp3/3P2b1/5NP1/PPP2PBP/R1BQK2R w KQkq e6 0 9",
            "r1bqk1nr/pp3p1p/2pp2p1/2b4Q/2BpP3/3P4/PPP2PPP/RNB2RK1 w kq - 0 9",
            "rnbq1rk1/ppp1bpp1/3pp2p/6n1/2PPPB2/2NB1N2/PPQ2PPP/2KR3R b - - 5 9",
            "r2qkb1r/p2bnppp/2p1p3/2PpP3/8/2P1BN2/PP3PPP/RN1QK2R b KQkq - 2 9",
            "r2qkb1r/pb1n1ppp/2pp1n2/P3p3/1p1PP3/3BBN2/1PP1NPPP/R2QK2R b KQkq - 0 9",
            "r1bqk1nr/p4pbp/1pn1p1p1/1NppP3/5P2/3PB1P1/PPP3BP/R2QK1NR b KQkq - 0 9",
            "r2qkb1r/5ppp/p1p1pn2/1pp5/P3P1b1/3PBN2/1PPN1PPP/R2QK2R w KQkq b6 0 9",
            "r2qk2r/pbpnbppp/1p1ppn2/8/2PP4/P1N2NP1/1PQ1PPBP/R1B1K2R w KQkq - 2 9",
            "r1bqkb1r/pp1p1ppp/8/2p1P3/1n2Q3/8/PPP2PPP/RNB1KB1R b KQkq - 5 9"};

    chess::Board board;
    search::SearchManager sm;
    sm.set_uci_mode(false);
    sm.allocate_threads(1);
    sm.allocate_hash(32);

    search::Limits limits = search::Limits::create_depth_limit(11);

    int64_t nodes = 0;
    int64_t total_time = 1;

    for (const std::string &fen : fens) {
        sm.tt_clear();
        board.load(fen, true);
        sm.set_limits(limits);

        int64_t start_time = now();

        sm.search<true>(board);

        int64_t end_time = now();
        int64_t elapsed_time = end_time - start_time;
        total_time += elapsed_time;

        nodes += sm.get_node_count();
    }

    int64_t nps = calculate_nps(total_time, nodes);
    print(nodes, "nodes", nps, "nps");
}
