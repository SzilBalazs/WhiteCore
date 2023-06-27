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

#include "../core/constants.h"

#include <optional>

namespace search {
    struct Limits {
        std::optional<int64_t> time_left, increment, moves_to_go, depth, move_time, max_nodes;
    };

    inline Limits create_node_limit(int64_t max_nodes) {
        Limits limit;
        limit.max_nodes = max_nodes;
        return limit;
    }

    inline Limits create_depth_limit(int64_t max_depth) {
        Limits limit;
        limit.depth = max_depth;
        return limit;
    }

    inline Limits create_time_limit(int64_t max_time) {
        Limits limit;
        limit.move_time = max_time;
        return limit;
    }
} // namespace search