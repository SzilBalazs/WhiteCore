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

#include "../core/board.h"
#include "search_thread.h"
#include "time_manager.h"
#include "tt.h"

namespace search {
    class SearchManager {
    public:
        void allocate_threads(unsigned int thread_count) {
            allocated_threads = thread_count;
        }

        void allocate_hash(unsigned int MB) {
            shared.tt.resize(MB);
        }

        void set_limits(const Limits &limits) {
            shared.tm.init(limits);
        }

        int64_t get_elapsed_time() {
            return shared.tm.get_elapsed_time();
        }

        int64_t get_node_count() {
            return shared.node_count;
        }

        void set_uci_mode(bool uci_mode) {
            shared.uci_mode = uci_mode;
        }

        template<bool wait_to_finish>
        void join() {
            if (!wait_to_finish) {
                shared.is_searching = false;
            }

            for (SearchThread &thread : threads) {
                thread.join();
            }

            threads.clear();
        }

        template<bool block>
        void search(const core::Board &board) {
            join<false>();
            for (unsigned int thread_id = 0; thread_id < allocated_threads; thread_id++) {
                threads.emplace_back(shared, thread_id);
                threads.back().load_board(board);
            }
            shared.node_count = 0;
            shared.best_move = core::NULL_MOVE;
            shared.is_searching = true;
            for (SearchThread &thread : threads) {
                thread.start();
            }
            if (block) join<true>();
        }

        void stop() {
            join<false>();
        }

        std::pair<core::Move, Score> get_result() {
            join<true>();
            return {shared.best_move, shared.eval};
        }

        void tt_clear() {
            shared.tt.clear();
        }

    private:
        unsigned int allocated_threads;
        std::vector<SearchThread> threads;
        SharedMemory shared;
    };
} // namespace search