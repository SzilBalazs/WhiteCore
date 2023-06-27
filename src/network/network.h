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

#include "activations/relu.h"
#include "activations/sigmoid.h"
#include "layers/tapered_eval.h"
#include "layers/dense_layer.h"

namespace nn {

    struct Gradient {
        layers::DenseLayerGradient<768, 2> pst;

        void operator+=(const Gradient &g) {
            pst += g.pst;
        }
    };

    struct Network {

        static constexpr int MAGIC = 2;

        static constexpr unsigned int get_feature_index(Piece piece, unsigned int sq) {
            return (piece.color == WHITE) * 384 + piece.type * 64 + sq;
        }

        layers::DenseLayer<768, 2, activations::none> pst;
        layers::TaperedEval<activations::sigmoid> tapered_eval;

        Network(const std::string &network_path) {
            std::ifstream file(network_path, std::ios::in | std::ios::binary);
            if (!file.is_open()) {
                logger.print("Unable to open:", network_path);
                throw std::invalid_argument("Unable to open: " + network_path);
            }

            int magic;
            file.read(reinterpret_cast<char *>(&magic), sizeof(magic));

            if (magic != MAGIC) {
                logger.print("Invalid network file", network_path, "with magic", magic);
                throw std::invalid_argument("Invalid network file with magic " + std::to_string(magic));
            }

            pst.load_from_file(file);

            file.close();

            logger.print("Loaded network file");

        }

        Network() {
            std::random_device rd;
            std::mt19937 mt(rd());
            pst.randomize(mt);
        }

        float forward(const std::vector<unsigned int> &features, float phase) const {
            std::array<float, 2> l1_output;
            float l2_output;
            pst.forward(features, l1_output);
            tapered_eval.forward(l1_output, l2_output, phase);
            return l2_output;
        }

        void write_to_file(const std::string &output_path) {
            std::ofstream file(output_path, std::ios::out | std::ios::binary);
            if (!file.is_open()) {
                logger.print("Unable to open:", output_path);
                throw std::invalid_argument("Unable to open: " + output_path);
            }

            int magic = MAGIC;
            file.write(reinterpret_cast<char *>(&magic), sizeof(magic));

            pst.write_to_file(file);

            file.close();
        }

    };
} // namespace nn