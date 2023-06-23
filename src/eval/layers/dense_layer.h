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
#include <vector>
#include <functional>
#include <thread>

#include "../activations/none.h"

template<typename T, unsigned int IN, unsigned int OUT, typename ACTIVATION = activation::none>
class DenseLayer {

    static_assert(std::is_invocable_r_v<T, decltype(ACTIVATION::forward), T>, "Invalid ACTIVATION::forward");
    static_assert(std::is_invocable_r_v<T, decltype(ACTIVATION::backward), T>, "Invalid ACTIVATION::backward");

    std::array<T, OUT> biases;
    std::array<T, IN * OUT> weights;

    void activate(std::array<T, OUT> &output) {
        for (unsigned int i = 0; i < OUT; i++) {
            output[i] = ACTIVATION::forward(output[i]);
        }
    }

    void forward(const std::vector<unsigned int> &input_features, std::array<T, OUT> &output) {
        std::copy(biases.begin(), biases.end(), output.begin());
        for (unsigned int i : input_features) {
            for (unsigned int j = 0; j < OUT; j++) {
                output[j] += weights[i * OUT + j];
            }
        }
        activate(output);
    }

    void forward(const std::array<T, IN> &input, std::array<T, OUT> &output) {
        std::copy(biases.begin(), biases.end(), output.begin());
        for (unsigned int i = 0; i < IN; i++) {
            for (unsigned int j = 0; j < OUT; j++) {
                output[j] += input[i] * weights[i * OUT + j];
            }
        }
        activate(output);
    }

    void backward(const std::array<T, OUT> &loss, const std::array<T, IN> &input, std::array<T, IN> &input_loss, std::array<T, OUT> &bias_gradiant, std::array<T, IN * OUT> &weight_gradient) {
        std::array<T, OUT> loss_before_activation;
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

    void backward(const std::array<T, OUT> &loss, const std::vector<unsigned int> &input_features, std::array<T, OUT> &bias_gradiant, std::array<T, IN * OUT> &weight_gradient) {
        std::array<T, OUT> loss_before_activation;
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
