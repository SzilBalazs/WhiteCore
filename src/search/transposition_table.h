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

#include "../chess/constants.h"
#include "../chess/move.h"

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

    struct TTEntry {                                // Total: 16 bytes
        uint64_t hash = 0;                          // 8 bytes
        Score eval = 0;                             // 4 bytes
        chess::Move hash_move = chess::NULL_MOVE;   // 2 bytes
        Depth depth = 0;                            // 1 byte
        TTFlag flag = TT_NONE;                      // 1 byte

        constexpr TTEntry() = default;
    };

    static_assert(sizeof(TTEntry) == 16);

    class TT {
    public:

        ~TT() {
            free_table();
        }

        void free_table() {
            if (bucket_count) {
                free(table);
                bucket_count = 0;
            }
        }

        void resize(unsigned int MB) {
            free_table();

            uint64_t i = 10;
            while ((1ULL << i) <= MB * 1024ULL * 1024ULL / sizeof(TTEntry))
                i++;

            bucket_count = (1ULL << (i - 1));
            mask = bucket_count - 1ULL;

            table = (TTEntry *) malloc(bucket_count * sizeof(TTEntry));
        }

        void clear() {
            for (uint64_t i = 0; i < bucket_count; i++) {
                table[i] = TTEntry();
            }
        }

        std::optional<TTEntry> probe(uint64_t hash) {
            TTEntry entry = *get_entry(hash);

            if (entry.hash != hash)
                return std::nullopt;

            return entry;
        }

        void save(uint64_t hash, Depth depth, Score eval, TTFlag flag, chess::Move best_move) {
            TTEntry *entry = get_entry(hash);

            if (flag == TT_ALPHA && entry->hash == hash) {
                best_move = chess::NULL_MOVE;
            }

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

        void prefetch(uint64_t hash) {
            __builtin_prefetch(get_entry(hash), 0);
        }

        chess::Move get_hash_move(uint64_t hash) {
            TTEntry *entry = get_entry(hash);
            if (entry->hash == hash)
                return entry->hash_move;
            return chess::NULL_MOVE;
        }

    private:
        TTEntry *table = nullptr;
        uint64_t bucket_count = 0;
        uint64_t mask = 0;

        TTEntry *get_entry(uint64_t hash) {
            return table + (hash & mask);
        }
    };
} // namespace search