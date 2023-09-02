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

#include <array>
#include <cmath>

namespace nn::activations::softmax {

    void forward(const std::vector<float> &z, std::vector<float> &s) {
        const size_t N = z.size();
        std::vector<float> e(N);
        float sum = 0.0;

        for (size_t i = 0; i < N; i++) {
            e[i] = std::exp(z[i]);
            sum += e[i];
        }

        s.assign(N, 0.0f);
        for (size_t i = 0; i < N; i++) {
            s[i] = e[i] / sum;
        }
    }

    template<size_t N>
    void backward(const std::array<float, N> &s, std::array<float, N> &z_grad, size_t target) {
        std::fill(z_grad.begin(), z_grad.end(), 0.0f);

        std::array<float, N> s_grad;
        for (size_t i = 0; i < N; i++) {
            s_grad[i] = s[i] - (i == target);
        }

        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                z_grad[i] += s_grad[j] * s[j] * (i == j ? 1.0 : 0.0) - s_grad[i] * s[i];
            }
        }
    }
} // namespace nn::activations::softmax