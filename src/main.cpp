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

#include "tests/tests.h"
#include "uci/uci.h"
#include "utils/bench.h"

namespace core {
    // Declarations
    Bitboard masks_bit[64], masks_adjacent_file[64], masks_adjacent_north[64], masks_adjacent_south[64], masks_pawn[64][2], masks_passed_pawn[64][2],
            masks_knight[64], masks_king[64], masks_file[64], masks_rank[64], masks_rook[64], masks_diagonal[64],
            masks_anti_diagonal[64], masks_bishop[64], masks_common_ray[64][64];
    LineType line_type[64][64];
    Bitboard attack_table_rook[102400], attack_table_bishop[5248];
} // namespace core

namespace search {
    Depth lmr_reductions[200][MAX_PLY + 1];
}

Logger logger;

void init_all() {
    logger.init("log.txt");
    logger.info("init_all", "Logger has been initialized");

    core::init_masks();
    logger.info("init_all", "Bitboard masks have been initialized");

    core::init_magic();
    logger.info("init_all", "Magic has been initialized");

    search::init_lmr();
    logger.info("init_all", "LMR has been initialized");
}

int main(int argc, char *argv[]) {

    std::string mode;
    if (argc >= 2) {
        mode = std::string(argv[1]);
    }

    init_all();

    if (mode == "test") {
        test::run();
    } else if (mode == "bench") {
        run_bench();
    }  else {
        uci::UCI protocol;
        protocol.start();
    }

    logger.info("main", "Exiting with return code 0");
    return 0;
}