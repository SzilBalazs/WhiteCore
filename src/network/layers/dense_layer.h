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
#include <functional>
#include <thread>
#include <vector>
#include <random>
#include <cstring>

#include "../activations/none.h"

namespace nn::layers {

    template<unsigned int IN, unsigned int OUT>
    struct DenseLayerGradient {
        std::array<float, OUT> biases;
        std::array<float, IN * OUT> weights;

        DenseLayerGradient() {
            std::fill(biases.begin(), biases.end(), 0);
            std::fill(weights.begin(), weights.end(), 0);
        }

        void operator+=(const DenseLayerGradient &g) {
            for (unsigned int i = 0; i < OUT; i++) {
                biases[i] += g.biases[i];
            }
            for (unsigned int i = 0; i < IN * OUT; i++) {
                weights[i] += g.weights[i];
            }
        }
    };

    template<unsigned int IN, unsigned int OUT, typename T, typename T2, typename ACTIVATION = activations::none<T>>
    class DenseLayer {
    private:

        static_assert(std::is_invocable_r_v<T2, decltype(ACTIVATION::forward), T2>, "Invalid ACTIVATION::forward");
        static_assert(std::is_invocable_r_v<T2, decltype(ACTIVATION::backward), T2>, "Invalid ACTIVATION::backward");

        void activate(std::array<T2, OUT> &output) const {
            for (unsigned int i = 0; i < OUT; i++) {
                output[i] = ACTIVATION::forward(output[i]);
            }
        }

    public:

        std::array<T, OUT> biases;
        std::array<T, IN * OUT> weights;

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

        void randomize(std::mt19937 &mt) {
            std::uniform_real_distribution<T> dist(-0.1, 0.1);

            for (unsigned int i = 0; i < IN * OUT; i++) {
                weights[i] = dist(mt);
            }

            for (unsigned int i = 0; i < OUT; i++) {
                biases[i] = dist(mt);
            }
        }

        void write_to_file(std::ofstream &file) {
            file.write(reinterpret_cast<char *>(biases.data()), sizeof(biases));
            file.write(reinterpret_cast<char *>(weights.data()), sizeof(weights));
        }

        template<typename QTYPE, int QBIAS_SCALE, int QWEIGHT_SCALE>
        void quantize(std::ofstream &file) {
            std::array<QTYPE, OUT> qbiases;
            std::array<QTYPE, IN * OUT> qweights;
            for (unsigned int i = 0; i < OUT; i++) {
                qbiases[i] = round(biases[i] * QBIAS_SCALE);
            }
            for (unsigned int i = 0; i < IN * OUT; i++) {
                qweights[i] = round(weights[i] * QWEIGHT_SCALE);
            }

            file.write(reinterpret_cast<char *>(qbiases.data()), sizeof(qbiases));
            file.write(reinterpret_cast<char *>(qweights.data()), sizeof(qweights));
        }

        void forward(const std::vector<unsigned int> &input_features, std::array<T2, OUT> &output) const {
            for (unsigned int i = 0; i < OUT; i++) {
                output[i] = biases[i];
            }

            for (unsigned int i : input_features) {
                for (unsigned int j = 0; j < OUT; j++) {
                    output[j] += weights[i * OUT + j];
                }
            }
            activate(output);
        }

        void forward(const std::array<T, IN> &input, std::array<T2, OUT> &output) const {
            for (unsigned int i = 0; i < OUT; i++) {
                output[i] = biases[i];
            }

            for (unsigned int i = 0; i < IN; i++) {
                for (unsigned int j = 0; j < OUT; j++) {
                    output[j] += input[i] * weights[i * OUT + j];
                }
            }
            activate(output);
        }

        void backward(const std::array<T, OUT> &loss, const std::array<T, IN> &input, const std::array<T, OUT> &output, std::array<T, IN> &input_loss, DenseLayerGradient<IN, OUT> &gradient) const {
            static_assert(std::is_same<T, T2>(), "T and T2 are not equal");

            std::array<T, OUT> loss_before_activation;
            std::fill(input_loss.begin(), input_loss.end(), 0);

            for (unsigned int i = 0; i < OUT; i++) {
                loss_before_activation[i] = loss[i] * ACTIVATION::backward(output[i]);
                gradient.biases[i] += loss_before_activation[i];
            }

            for (unsigned int i = 0; i < IN; i++) {
                for (unsigned int j = 0; j < OUT; j++) {
                    gradient.weights[i * OUT + j] += input[i] * loss_before_activation[j];
                    input_loss[i] += weights[i * OUT + j] * loss_before_activation[j];
                }
            }
        }

        void backward(const std::array<T, OUT> &loss, const std::vector<unsigned int> &input_features, const std::array<T, OUT> &output, DenseLayerGradient<IN, OUT> &gradient) const {
            static_assert(std::is_same<T, T2>(), "T and T2 are not equal");

            std::array<T, OUT> loss_before_activation;

            for (unsigned int i = 0; i < OUT; i++) {
                loss_before_activation[i] = loss[i] * ACTIVATION::backward(output[i]);
                gradient.biases[i] += loss_before_activation[i];
            }

            for (unsigned int i : input_features) {
                for (unsigned int j = 0; j < OUT; j++) {
                    gradient.weights[i * OUT + j] += loss_before_activation[j];
                }
            }
        }
    };
} // namespace nn::layers