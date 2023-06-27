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

#include "../search/search_manager.h"

namespace selfplay {

    class Engine {
    public:
        Engine() {
            sm.set_uci_mode(false);
        }

        void init(unsigned int hash_size, unsigned int thread_count) {
            sm.allocate_hash(hash_size);
            sm.allocate_threads(thread_count);
        }

        std::pair<core::Move, Score> search(const core::Board &board, const search::Limits &limits) {
            sm.set_limits(limits);
            sm.search<true>(board);
            return sm.get_result();
        }


    private:
        search::SearchManager sm;
        // Network net;
    };

} // namespace selfplay
