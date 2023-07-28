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

#include "search_limits.h"

namespace search {
    class TimeManager {
    public:
        void init(const Limits &limits) {

            calculate_allocated_time(limits);

            max_nodes = limits.max_nodes.value_or(INF_NODES);
            max_depth = limits.depth.value_or(MAX_PLY);

            start_time = now();
            end_time = start_time + allocated_time - MOVE_OVERHEAD;
        }

        [[nodiscard]] bool time_left() const {
            return now() < end_time;
        }

        [[nodiscard]] int64_t get_elapsed_time() const {
            return now() - start_time;
        }

        [[nodiscard]] Depth get_max_depth() const {
            return max_depth;
        }

        [[nodiscard]] int64_t get_max_nodes() const {
            return max_nodes;
        }

    private:
        int64_t start_time, allocated_time, end_time, max_nodes, max_depth;

        void calculate_allocated_time(const Limits &limits) {
            int64_t moves_to_go = limits.moves_to_go.value_or(30);
            int64_t increment = limits.increment.value_or(0);
            if (limits.time_left) {
                int64_t time_left = limits.time_left.value();
                allocated_time = (time_left + moves_to_go * increment) / moves_to_go;
                allocated_time = std::min(allocated_time, time_left);
            } else {
                allocated_time = limits.move_time.value_or(INF_TIME);
            }
        }
    };
} // namespace search