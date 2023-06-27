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

#include "network.h"

namespace nn {

    class Adam {
    public:

        Adam(float learning_rate) : LR(learning_rate), m_gradient(), v_gradient() {}

        void update(const std::vector<Gradient> &gradients, Network &network) {
            Gradient total;
            for (const Gradient &g : gradients) {
                total += g;
            }

            update(network.pst.weights, m_gradient.pst.weights, v_gradient.pst.weights, total.pst.weights);
            update(network.pst.biases, m_gradient.pst.biases, v_gradient.pst.biases, total.pst.biases);
        }

    private:
        static constexpr float BETA1 = 0.9f;
        static constexpr float BETA2 = 0.999f;
        static constexpr float EPSILON = 1e-8;
        float LR;

        Gradient m_gradient, v_gradient;

        template<uint64_t LEN>
        void update(std::array<float, LEN> &target, std::array<float, LEN> &m, std::array<float, LEN> &v, const std::array<float, LEN> &grad) {
            for (unsigned int i = 0; i < LEN; i++) {
                apply_gradient(target[i], m[i], v[i], grad[i]);
            }
        }

        void apply_gradient(float &target, float &m, float &v, float grad) {
            m = BETA1 * m + (1.0f - BETA1) * grad;
            v = BETA2 * v + (1.0f - BETA2) * grad * grad;

            target -= LR * m / (std::sqrt(v) + EPSILON);
        }
    };

}