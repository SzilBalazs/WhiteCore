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
#include "../core/move.h"

#include <cstring>

namespace search {
    enum TTFlag : uint8_t {
        TT_NONE = 0,
        TT_EXACT = 1,

        // UPPERBOUND
        TT_ALPHA = 2,

        // LOWERBOUND
        TT_BETA = 3
    };

    struct TTEntry {          // Total: 16 bytes
        U64 hash;             // 8 bytes
        Score eval;           // 4 bytes
        core::Move hash_move; // 2 bytes
        Depth depth;          // 1 byte
        TTFlag flag;          // 1 byte
    };

    static_assert(sizeof(TTEntry) == 16);

    class TT {
    public:
        void resize(unsigned int MB) {
            if (bucket_count)
                free(table);

            unsigned int i = 10;
            while ((1ULL << i) <= MB * 1024ULL * 1024ULL / sizeof(TTEntry))
                i++;

            bucket_count = (1ULL << (i - 1));
            mask = bucket_count - 1ULL;

            table = (TTEntry *) malloc(bucket_count * sizeof(TTEntry));

            clear();
        }

        void clear() {
            std::memset(table, 0, bucket_count * sizeof(TTEntry));
        }

        std::optional<TTEntry> probe(U64 hash) {
            TTEntry entry = *get_entry(hash);

            if (entry.hash != hash)
                return std::nullopt;

            return entry;
        }

        void save(U64 hash, Depth depth, Score eval, TTFlag flag, core::Move best_move) {
            TTEntry *entry = get_entry(hash);

            if (entry->hash != hash || best_move.is_ok()) {
                entry->hash_move = best_move;
            }

            if (entry->hash != hash || flag == TT_EXACT || entry->depth <= depth + 4) {
                entry->hash = hash;
                entry->depth = depth;
                entry->eval = eval;
                entry->flag = flag;
            }
        }

        void prefetch(U64 hash) {
            __builtin_prefetch(get_entry(hash), 0);
        }

        core::Move get_hash_move(U64 hash) {
            TTEntry *entry = get_entry(hash);
            if (entry->hash == hash)
                return entry->hash_move;
            return core::NULL_MOVE;
        }

    private:
        TTEntry *table;
        unsigned int bucket_count = 0;
        U64 mask = 0;

        TTEntry *get_entry(U64 hash) {
            return table + (hash & mask);
        }
    };
} // namespace search