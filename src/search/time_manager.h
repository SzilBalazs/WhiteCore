// WhiteCore is a C++ chess engine
// Copyright (c) 2023-2024 Balázs Szilágyi
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
        static int64_t MOVE_OVERHEAD;

        /**
         * Initializes time management with the given limits.
         *
         * @param limits Limits for the search.
         */
        void init(const Limits &limits) {
            calculate_allocated_time(limits);

            max_nodes = limits.max_nodes.value_or(INF_NODES);
            max_depth = limits.depth.value_or(MAX_PLY);

            start_time = now();

            update_end_time(1.0);
        }

        /**
         * Checks whether there is time left for continued search.
         *
         * @return Returns if the current time is lesser than the maximum end time.
         */
        [[nodiscard]] bool time_left() const {
            return now() < max_end_time;
        }

        /**
         * Computes the total elapsed time since the start of the search.
         *
         * @return Returns the difference between the current time and the start time.
         */
        [[nodiscard]] int64_t get_elapsed_time() const {
            return now() - start_time;
        }

        /**
         * Gets the maximum searchable depth.
         *
         * @return Returns value of the maximum searchable depth.
         */
        [[nodiscard]] Depth get_max_depth() const {
            return max_depth;
        }

        /**
         * Get the maximum number of nodes to be searched.
         *
         * @return Returns value of maximum number of nodes to be searched.
         */
        [[nodiscard]] int64_t get_max_nodes() const {
            return max_nodes;
        }

        /**
         * Computes a scale factor to be applied on the optimal end time.
         *
         * @param bm_stability The stability of the best move.
         * @return Returns if the search should continue.
         */
        bool handle_iteration(int bm_stability, double bm_effort) {
            const double bm_scale = 1.2 - std::min(bm_stability, 10) * 0.04;
            const double effort_scale = 1.5 - bm_effort;

            const double scale = bm_scale * effort_scale;

            update_end_time(scale);

            return now() < opt_end_time;
        }

    private:
        int64_t start_time, max_nodes, max_depth;
        int64_t opt_base_time, opt_end_time;
        int64_t max_time_usage, max_end_time;

        void calculate_allocated_time(const Limits &limits) {
            int64_t moves_to_go = limits.moves_to_go.value_or(20);
            int64_t increment = limits.increment.value_or(0);

            if (limits.time_left) {
                int64_t time_left = limits.time_left.value();

                opt_base_time = (time_left + moves_to_go * increment) / (moves_to_go + 5);
                max_time_usage = (time_left + moves_to_go * increment) / moves_to_go;

                max_time_usage = std::min(max_time_usage, time_left);
            } else {
                opt_base_time = INF_TIME;
                max_time_usage = limits.move_time.value_or(INF_TIME);
            }
        }

        void update_end_time(double scale) {
            int64_t scaled = double(opt_base_time) * scale;
            scaled = std::min(scaled, max_time_usage);

            opt_end_time = start_time + scaled - MOVE_OVERHEAD;
            max_end_time = start_time + max_time_usage - MOVE_OVERHEAD;
        }
    };
} // namespace search