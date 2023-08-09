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

#include "../activations/none.h"
#include "../../chess/constants.h"

#include <array>
#include <cstring>
#include <fstream>
#include <immintrin.h>
#include <type_traits>
#include <vector>

namespace nn::layers {

    template<size_t IN, size_t OUT, typename T, typename ACTIVATION = activations::none<T>>
    class Accumulator {

        static_assert(std::is_invocable_r_v<T, decltype(ACTIVATION::forward), T>, "Invalid ACTIVATION::forward");

        static constexpr size_t register_width = 256 / (sizeof(T) * 8);
        static constexpr size_t chunk_count = OUT / register_width;

        static_assert(std::is_same_v<T, int16_t>, "Only int16 is supported for accumulators");
        static_assert(256 % sizeof(T) == 0);
        static_assert(OUT % register_width == 0);

    public:
        void load_from_file(std::ifstream &file) {
            file.read(reinterpret_cast<char *>(biases.data()), sizeof(biases));
            file.read(reinterpret_cast<char *>(weights.data()), sizeof(weights));
        }

        int load_from_pointer(const unsigned char *ptr, int offset) {
            std::memcpy(biases.data(), ptr + offset, sizeof(T) * OUT);
            offset += sizeof(T) * OUT;
            std::memcpy(weights.data(), ptr + offset, sizeof(T) * IN * OUT);
            offset += sizeof(T) * IN * OUT;

            return offset;
        }

        void refresh(const std::vector<unsigned int> &features) {
            reset();
            for (unsigned int feature : features) {
                add_feature(feature);
            }
        }

        void add_feature(unsigned int feature) {
#ifdef AVX2
            for (size_t i = 0; i < chunk_count; i++) {
                const unsigned int offset = i * register_width;
                __m256i base = _mm256_load_si256((__m256i *) &accumulator[offset]);
                __m256i weight = _mm256_load_si256((__m256i *) &weights[feature * OUT + offset]);
                _mm256_store_si256((__m256i *) &accumulator[offset], _mm256_add_epi16(base, weight));
            }
#else
                for (size_t j = 0; j < OUT; j++) {
                    accumulator[j] += weights[feature * OUT + j];
                }
#endif
        }

        void remove_feature(unsigned int feature) {
#ifdef AVX2
            for (size_t i = 0; i < chunk_count; i++) {
                const size_t offset = i * register_width;
                __m256i base = _mm256_load_si256((__m256i *) &accumulator[offset]);
                __m256i weight = _mm256_load_si256((__m256i *) &weights[feature * OUT + offset]);
                _mm256_store_si256((__m256i *) &accumulator[offset], _mm256_sub_epi16(base, weight));
            }
#else
            for (size_t j = 0; j < OUT; j++) {
                accumulator[j] -= weights[feature * OUT + j];
            }
#endif
        }

        void push(std::array<T, OUT> &result) {

#ifdef AVX2
            static_assert(std::is_invocable_r_v<__m256i, decltype(ACTIVATION::_mm256_forward_epi16), __m256i>, "ACTIVATION::forward doesn't support AVX2 registers");

            for (size_t i = 0; i < chunk_count; i++) {
                const size_t offset = i * register_width;
                __m256i base = _mm256_load_si256((__m256i *) &accumulator[offset]);
                _mm256_store_si256((__m256i *) &result[offset], ACTIVATION::_mm256_forward_epi16(base));
            }
#else
            for (size_t i = 0; i < OUT; i++) {
                result[i] = ACTIVATION::forward(accumulator[i]);
            }
#endif
        }

    private:
        alignas(64) std::array<T, OUT> accumulator;
        alignas(64) std::array<T, OUT> biases;
        alignas(64) std::array<T, IN * OUT> weights;

        void reset() {
            std::copy(biases.begin(), biases.end(), accumulator.begin());
        }
    };
} // namespace nn::layers