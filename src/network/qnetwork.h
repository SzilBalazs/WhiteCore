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
#include "../core/constants.h"
#include "../utils/logger.h"

#include <fstream>

namespace nn {

    struct QNetwork {

        static constexpr int MAGIC = 1;

        static constexpr unsigned int get_feature_index(Piece piece, unsigned int sq) {
            return (piece.color == WHITE) * 384 + piece.type * 64 + sq;
        }

        layers::DenseLayer<768, 1, activations::none> pst;

        QNetwork() = default;

        QNetwork(const std::string &network_path) {
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

        Score forward(const std::vector<unsigned int> &features) const {
            std::array<float, 1> output;
            pst.forward(features, output);
            return output[0] * 400;
        }
    };
} // namespace nn