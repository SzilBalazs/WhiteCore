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
    } else {
        uci::UCI protocol;
        protocol.start();
    }

    logger.info("main", "Exiting with return code 0");
    return 0;
}