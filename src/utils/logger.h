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

#include <iostream>
#include <sstream>

class Logger {
public:
    template<typename... Args>
    explicit Logger(Args... args) {
        print(args...);
        std::cout << ss.str() << std::flush;
    }

    template<typename T, typename... Args>
    void print(T a, Args... args) {
        ss << a << " ";
        print(args...);
    }

    void print() {
        ss << "\n";
    }

private:
    std::stringstream ss;
};
