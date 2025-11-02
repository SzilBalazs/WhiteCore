// WhiteCore is a C++ chess engine
// Copyright (c) 2022-2025 Balázs Szilágyi
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

#include <random>

void split_data(const std::string &input, const std::string &output1, const std::string &output2, int rate) {
    std::ifstream in(input, std::ios::in);
    std::ofstream out1(output1, std::ios::out | std::ios::app);
    std::ofstream out2(output2, std::ios::out | std::ios::app);

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<int> dist(0, rate);

    std::string line;
    while (std::getline(in, line)) {
        if (dist(g) == 0)
            out2 << line << "\n";
        else
            out1 << line << "\n";
    }

    in.close();
    out1.close();
    out2.close();
}