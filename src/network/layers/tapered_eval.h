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
#include <algorithm>

#include "../activations/none.h"

namespace nn::layers {

    constexpr float MAX_PHASE = 64.0f;

    template<typename ACTIVATION = activations::none>
    struct TaperedEval {
        void forward(const std::array<float, 2> input, float &output, float phase) const {
            phase = std::min(MAX_PHASE, phase);
            float res = ((MAX_PHASE - phase) * input[0] + (phase) * input[1]) / MAX_PHASE;
            output = ACTIVATION::forward(res);
        }

        void backward(const float &loss, std::array<float, 2> &input_loss, float phase) {
            phase = std::min(MAX_PHASE, phase);
            float loss_before_activation = loss * ACTIVATION::backward(loss);
            input_loss[0] = ((MAX_PHASE - phase) * loss_before_activation) / MAX_PHASE;
            input_loss[1] = ((phase) * loss_before_activation) / MAX_PHASE;
        }
    };
}
