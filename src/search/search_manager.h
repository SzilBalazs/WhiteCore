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

#include "../chess/board.h"
#include "search_thread.h"
#include "time_manager.h"
#include "transposition_table.h"

namespace search {
    class SearchManager {
    public:
        /**
         * Sets the number of threads to use for searching.
         *
         * @param thread_count Number of threads
         */
        void allocate_threads(size_t thread_count) {
            allocated_threads = thread_count;
        }

        /**
         * Sets the amount of memory to use for the transposition table.
         *
         * @param hash_size Amount of memory in MB
         */
        void allocate_hash(unsigned int hash_size) {
            shared.tt.resize(hash_size);
        }

        /**
         * Sets the resource limits of the search.
         *
         * @param limits Search limits
         */
        void set_limits(const Limits &limits) {
            shared.tm.init(limits);
        }

        /**
         * Returns number of positions searched so far.
         *
         * @return Number of positions searched so far.
         */
        int64_t get_node_count() {
            return shared.get_node_count();
        }

        /**
         * Disables/enables console output.
         *
         * @param uci_mode If true the search will print to the console.
         */
        void set_uci_mode(bool uci_mode) {
            shared.uci_mode = uci_mode;
        }

        /**
         * Joins all the search workers to the main thread.
         *
         * @tparam wait_to_finish If true, the search will be forced to stop.
         * @tparam wait_to_finish otherwise will wait for the search to finish.
         */
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

        /**
         * Searches a position to find the best move.
         *
         * @tparam block If true, the function will block until all threads have completed
         * @param board The board on which the search will be performed
         */
        template<bool block>
        void search(const chess::Board &board) {
            join<false>();
            for (size_t thread_id = 0; thread_id < allocated_threads; thread_id++) {
                threads.emplace_back(shared, thread_id);
                threads.back().load_board(board);
            }
            shared.node_count.assign(allocated_threads, 0);
            shared.best_move = chess::NULL_MOVE;
            shared.is_searching = true;
            for (SearchThread &thread : threads) {
                thread.start();
            }
            if (block) join<true>();
        }

        /**
         * Stops all the running search threads.
         *
         */
        void stop() {
            join<false>();
        }

        /**
         * Waits for all the threads to complete and then fetches the result of the search.
         *
         * @return A pair consisting of the best move (first) and the evaluation score (second)
         */
        std::pair<chess::Move, Score> get_result() {
            join<true>();
            return {shared.best_move, shared.eval};
        }

        /**
         * This function clears the transposition table.
         *
         */
        void tt_clear() {
            shared.tt.clear();
        }

    private:
        size_t allocated_threads;
        std::vector<SearchThread> threads;
        SharedMemory shared;
    };
} // namespace search