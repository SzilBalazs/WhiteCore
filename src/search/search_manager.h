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

#include <atomic>
#include <mutex>

class SearchManager {
public:

	inline void allocate_threads(unsigned int thread_count) {
		allocated_threads = thread_count;
	}

	inline void allocate_hash(unsigned int MB) {
		shared.tt.resize(MB);
	}

	inline void set_limits(const SearchLimits &limits) {
		shared.tm.init(limits);
	}

	template<bool wait_to_finish>
	inline void join() {
		if (!wait_to_finish) {
			shared.is_searching = false;
		}

		for (SearchThread &thread : threads) {
			thread.join();
		}

		threads.clear();
	}

	template<bool block>
	inline void search(const Board &board) {
		join<false>();
		for (unsigned int thread_id = 0; thread_id < allocated_threads; thread_id++) {
			threads.emplace_back(shared, thread_id);
			threads.back().load_board(board);
		}
		shared.node_count = 0;
		shared.best_move = NULL_MOVE;
		shared.is_searching = true;
		for (SearchThread &thread : threads) {
			thread.start();
		}
		if (block) join<true>();
	}

	inline void stop() {
		join<false>();
	}

	inline Move get_best_move() {
		join<true>();
		return shared.best_move;
	}

	inline void tt_clear() {
		shared.tt.clear();
	}

private:

	unsigned int allocated_threads;
	std::vector<SearchThread> threads;
	SharedMemory shared;
};