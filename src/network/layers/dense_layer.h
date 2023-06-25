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

#include <array>
#include <functional>
#include <thread>
#include <vector>

#include "../activations/none.h"

namespace nn::layers {
    template<unsigned int IN, unsigned int OUT, typename ACTIVATION = activations::none>
    class DenseLayer {

        static_assert(std::is_invocable_r_v<float, decltype(ACTIVATION::forward), float>, "Invalid ACTIVATION::forward");
        static_assert(std::is_invocable_r_v<float, decltype(ACTIVATION::backward), float>, "Invalid ACTIVATION::backward");

        std::array<float, OUT> biases;
        std::array<float, IN * OUT> weights;

        void activate(std::array<float, OUT> &output) {
            for (unsigned int i = 0; i < OUT; i++) {
                output[i] = ACTIVATION::forward(output[i]);
            }
        }

        void forward(const std::vector<unsigned int> &input_features, std::array<float, OUT> &output) {
            std::copy(biases.begin(), biases.end(), output.begin());
            for (unsigned int i : input_features) {
                for (unsigned int j = 0; j < OUT; j++) {
                    output[j] += weights[i * OUT + j];
                }
            }
            activate(output);
        }

        void forward(const std::array<float, IN> &input, std::array<float, OUT> &output) {
            std::copy(biases.begin(), biases.end(), output.begin());
            for (unsigned int i = 0; i < IN; i++) {
                for (unsigned int j = 0; j < OUT; j++) {
                    output[j] += input[i] * weights[i * OUT + j];
                }
            }
            activate(output);
        }

        void backward(const std::array<float, OUT> &loss, const std::array<float, IN> &input, std::array<float, IN> &input_loss, std::array<float, OUT> &bias_gradiant, std::array<float, IN * OUT> &weight_gradient) {
            std::array<float, OUT> loss_before_activation;
            for (unsigned int i = 0; i < OUT; i++) {
                loss_before_activation[i] = loss[i] * ACTIVATION::backward(loss[i]);
                bias_gradiant[i] += loss_before_activation[i];
            }

            for (unsigned int i = 0; i < IN; i++) {
                for (unsigned int j = 0; j < OUT; j++) {
                    weight_gradient[i * OUT + j] += input[i] * loss_before_activation[j];
                    input_loss[i] += weights[i * OUT + j] * loss_before_activation[j];
                }
            }
        }

        void backward(const std::array<float, OUT> &loss, const std::vector<unsigned int> &input_features, std::array<float, OUT> &bias_gradiant, std::array<float, IN * OUT> &weight_gradient) {
            std::array<float, OUT> loss_before_activation;
            for (unsigned int i = 0; i < OUT; i++) {
                loss_before_activation[i] = loss[i] * ACTIVATION::backward(loss[i]);
                bias_gradiant[i] += loss_before_activation[i];
            }

            for (unsigned int i : input_features) {
                for (unsigned int j = 0; j < OUT; j++) {
                    weight_gradient[i * OUT + j] += loss_before_activation[j];
                }
            }
        }
    };
} // namespace nn::layers