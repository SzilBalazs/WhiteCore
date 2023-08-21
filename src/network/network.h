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

#include "../utils/threats.h"
#include "activations/crelu.h"
#include "activations/relu.h"
#include "activations/sigmoid.h"
#include "layers/dense_layer.h"

namespace nn {

    constexpr size_t L1_SIZE = 512;

    struct Gradient {
        layers::DenseLayerGradient<832, L1_SIZE> l0;
        layers::DenseLayerGradient<L1_SIZE, 1> l1;

        Gradient() = default;

        void operator+=(const Gradient &g) {
            l0 += g.l0;
            l1 += g.l1;
        }
    };

    struct Network {

        static constexpr int MAGIC = 6;

        static constexpr unsigned int get_feature_index(Piece piece, unsigned int sq) {
            return (piece.color == WHITE) * 384 + piece.type * 64 + sq;
        }

        layers::DenseLayer<832, L1_SIZE, float, float, activations::crelu<float, 1>> l0;
        layers::DenseLayer<L1_SIZE, 1, float, float, activations::sigmoid> l1;

        Network(const std::string &network_path) {
            std::ifstream file(network_path, std::ios::in | std::ios::binary);
            if (!file.is_open()) {
                print("Unable to open: ", network_path);
                randomize();
                return;
            }

            int magic;
            file.read(reinterpret_cast<char *>(&magic), sizeof(magic));

            if (magic != MAGIC) {
                print("Invalid network file: ", network_path, " with magic: ", magic);
                throw std::invalid_argument("Invalid network file with magic: " + std::to_string(magic));
            }

            l0.load_from_file(file);
            l1.load_from_file(file);

            print("Loaded network file: ", network_path);
        }

        Network() {
            randomize();
        }

        void randomize() {
            std::random_device rd;
            std::mt19937 mt(rd());
            l0.randomize(mt);
            l1.randomize(mt);
        }

        void forward(const std::vector<unsigned int> &features, std::array<float, L1_SIZE> &l0_output, std::array<float, 1> &l1_output) const {
            l0.forward(features, l0_output);
            l1.forward(l0_output, l1_output);
        }

        void write_to_file(const std::string &output_path) {
            std::ofstream file(output_path, std::ios::out | std::ios::binary);
            if (!file.is_open()) {
                print("Unable to open:", output_path);
                throw std::runtime_error("Unable to open: " + output_path);
            }

            int magic = MAGIC;
            file.write(reinterpret_cast<char *>(&magic), sizeof(magic));

            l0.write_to_file(file);
            l1.write_to_file(file);

            file.close();
        }

        template<typename QTYPE, int QSCALE>
        void quantize(const std::string &output_path) {
            std::ofstream file(output_path, std::ios::out | std::ios::binary);
            if (!file.is_open()) {
                print("Unable to open:", output_path);
                throw std::runtime_error("Unable to open: " + output_path);
            }

            int magic = -MAGIC;
            file.write(reinterpret_cast<char *>(&magic), sizeof(magic));

            l0.quantize<QTYPE, QSCALE, QSCALE>(file);
            l1.quantize<QTYPE, QSCALE * QSCALE, QSCALE>(file);

            file.close();
        }
    };
} // namespace nn