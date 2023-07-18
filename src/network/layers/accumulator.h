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

#include <type_traits>
#include <cstring>
#include <array>

namespace nn::layers {

    template<unsigned int IN, unsigned int OUT, typename T, typename ACTIVATION = activations::none<T>>
    class Accumulator {

        static_assert(std::is_invocable_r_v<T, decltype(ACTIVATION::forward), T>, "Invalid ACTIVATION::forward");

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
            for (unsigned int j = 0; j < OUT; j++) {
                accumulator[j] += weights[feature * OUT + j];
            }
        }

        void remove_feature(unsigned int feature) {
            for (unsigned int j = 0; j < OUT; j++) {
                accumulator[j] -= weights[feature * OUT + j];
            }
        }

        void copy_accumulator(std::array<T, OUT> &result) {
            std::copy(accumulator.begin(), accumulator.end(), result.begin());
            for (T &i : result) {
                i = ACTIVATION::forward(i);
            }
        }

    private:

        std::array<T, OUT> accumulator;
        std::array<T, OUT> biases;
        std::array<T, IN * OUT> weights;

        void reset() {
            std::copy(biases.begin(), biases.end(), accumulator.begin());
        }
    };
}