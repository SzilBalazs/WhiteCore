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

namespace nn::layers {

    template<size_t IN>
    struct PolicyLayerGradient {
        PolicyLayerGradient() {
            weights = new float[IN * OUT];
        }

        ~PolicyLayerGradient() {
            delete[] weights;
        }

        float *weights;
        static constexpr size_t OUT = 64 * 64;
    };

    template<size_t IN>
    class PolicyLayer {
    public:
        PolicyLayer() {
            weights = new float[IN * OUT];
        }

        ~PolicyLayer() {
            delete[] weights;
        }

        void forward(const std::array<float, IN> &input, const std::vector<chess::Move> &moves, std::vector<float> &output) const {
            const size_t N = moves.size();
            std::vector<float> z(N);

            for (size_t i = 0; i < N; i++) {
                size_t move_index = get_move_index(moves[i]);
                for (size_t j = 0; j < IN; j++) {
                    z[i] += input[j] * weights[move_index * IN + j];
                }
            }
            activations::softmax::forward(z, output);
        }

        void backward(const std::array<float, IN> &input, const std::vector<float> &output, const std::vector<chess::Move> &moves, size_t best_move_index, PolicyLayerGradient<IN> &gradient) {
            const size_t N = moves.size();

            std::vector<float> z_grad;
            activations::softmax::backward(output, z_grad, best_move_index);

            for (size_t i = 0; i < N; i++) {
                size_t move_index = get_move_index(moves[i]);
                for (size_t j = 0; j < IN; j++) {
                    gradient.weights[move_index * IN + j] += input[j] * z_grad[i];
                }
            }
        }

    private:
        static constexpr size_t OUT = 64 * 64;
        float *weights;

        size_t get_move_index(const chess::Move &move) {
            return move.get_from() * 64 + move.get_to();
        }
    };

} // namespace nn::layers
