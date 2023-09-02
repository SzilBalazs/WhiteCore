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

#include "activations/softmax.h"
#include "layers/policy_layer.h"

#pragma once

namespace nn {

    template<size_t IN>
    class PolicyNetwork {
    public:
        void forward(const std::array<float, IN> &input, const std::vector<chess::Move> &moves, std::vector<float> &output) {
            l.forward(input, moves, output);
        }

        void backward(const std::array<float, IN> &input, const std::vector<float> &output, const std::vector<chess::Move> &moves, const chess::Move &best_move, layers::PolicyLayerGradient<IN> &gradient) {
            size_t best_move_index = 0;
            for (size_t i = 0; i < moves.size(); i++) {
                if (moves[i] == best_move) {
                    best_move_index = i;
                    break;
                }
            }
            l.backward(input, output, moves, best_move_index, gradient);
        }

    private:
        layers::PolicyLayer<IN> l;
    };

} // namespace nn