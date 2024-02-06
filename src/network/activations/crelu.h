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

#include <algorithm>
#include <immintrin.h>

namespace nn::activations {
    template<typename T, int INT_UPPER_BOUND>
    struct crelu {

        static constexpr T UPPER_BOUND = static_cast<T>(INT_UPPER_BOUND);

        static T forward(T value) {
            return std::clamp(value, static_cast<T>(0), UPPER_BOUND);
        }

#ifdef AVX2
        static __m256i _mm256_forward_epi16(__m256i value) {
            static_assert(std::is_same_v<T, int16_t>, "Only int16 is supported with AVX2");
            const __m256i lower_bound = _mm256_setzero_si256();
            const __m256i upper_bound = _mm256_set1_epi16(UPPER_BOUND);
            return _mm256_min_epi16(_mm256_max_epi16(value, lower_bound), upper_bound);
        }
#endif

        static constexpr T backward(T value) {
            return static_cast<T>(0) < value && value < UPPER_BOUND;
        }
    };
} // namespace nn::activations