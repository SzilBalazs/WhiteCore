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
#include "../utils/stats.h"

#include <cstring>

namespace search {
    enum class TTFlag : uint8_t {
        NONE = 0,
        EXACT = 1,
        // UPPERBOUND
        ALPHA = 2,
        // LOWERBOUND
        BETA = 3
    };

    struct TTEntry {                              // Total: 16 bytes
        uint16_t hash = 0;                        // 2 bytes
        int16_t eval = 0;                         // 2 bytes
        chess::Move hash_move = chess::NULL_MOVE; // 2 bytes
        Depth depth = 0;                          // 1 byte
        TTFlag flag = TTFlag::NONE;               // 1 byte

        constexpr TTEntry() = default;
    };

    static_assert(sizeof(TTEntry) == 8);

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

        uint64_t get_hash_full() {

            if (bucket_count == 0) {
                return 0;
            }

            uint64_t res = 0;
            for (uint64_t i = 0; i < 1000; i++) {
                res += i == (table[i].hash & mask);
            }
            return res;
        }

        void clear() {
            for (uint64_t i = 0; i < bucket_count; i++) {
                table[i] = TTEntry();
            }
        }

        std::optional<TTEntry> probe(uint64_t hash64) {
            TTEntry entry = *get_entry(hash64);
            auto hash16 = static_cast<uint16_t>(hash64 >> 48);

            if (entry.hash != hash16) {
                stat_tracker::record_fail("tt_hit");
                return std::nullopt;
            }

            stat_tracker::record_success("tt_hit");

            return entry;
        }

        void save(uint64_t hash64, Depth depth, Score eval, TTFlag flag, chess::Move best_move) {
            TTEntry *entry = get_entry(hash64);
            auto hash16 = static_cast<uint16_t>(hash64 >> 48);

            if (flag == TTFlag::ALPHA && entry->hash == hash16) {
                best_move = chess::NULL_MOVE;
            }

            if (entry->hash != hash16 || best_move.is_ok()) {
                entry->hash_move = best_move;
            }

            if (entry->hash != hash16 || flag == TTFlag::EXACT || entry->depth <= depth + 4) {
                entry->hash = hash16;
                entry->depth = depth;
                entry->eval = static_cast<int16_t>(eval);
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