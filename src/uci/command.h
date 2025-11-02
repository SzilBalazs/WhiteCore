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

#include <functional>
#include <string>
#include <utility>

namespace uci {

    struct [[nodiscard]] Command {

        std::string cmd;
        std::function<void(std::vector<std::string>)> func;

        Command(std::string c, std::function<void(std::vector<std::string>)> f) : cmd(std::move(c)), func(std::move(f)) {}

        [[nodiscard]] bool is_match(const std::vector<std::string> &tokens) const {
            return tokens[0] == cmd;
        }
    };

} // namespace uci