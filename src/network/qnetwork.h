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

#include <cstring>
#include <fstream>

namespace nn {

    struct QNetwork {

        static constexpr int MAGIC = 4;

        static constexpr unsigned int get_feature_index(Piece piece, unsigned int sq) {
            return (piece.color == WHITE) * 384 + piece.type * 64 + sq;
        }

        layers::DenseLayer<768, 32, activations::crelu> l0;
        layers::DenseLayer<32, 1, activations::none> l1;

        QNetwork() = default;

        QNetwork(const std::string &network_path) {
            std::ifstream file(network_path, std::ios::in | std::ios::binary);
            if (!file.is_open()) {
                logger.print("Unable to open:", network_path);
                std::random_device rd;
                std::mt19937 mt(rd());
                l0.randomize(mt);
                l1.randomize(mt);
            } else {
                int magic;
                file.read(reinterpret_cast<char *>(&magic), sizeof(magic));

                if (magic != MAGIC) {
                    logger.print("Invalid network file", network_path, "with magic", magic);
                    throw std::invalid_argument("Invalid network file with magic " + std::to_string(magic));
                }

                l0.load_from_file(file);
                l1.load_from_file(file);

                file.close();

                logger.print("Loaded qnetwork", network_path);
            }
        }

        QNetwork(const unsigned char *ptr) {

            int magic;
            std::memcpy(&magic, ptr, sizeof(int));
            int offset = sizeof(int);

            if (magic != MAGIC) {
                logger.print("Invalid default network file");
                throw std::invalid_argument("Invalid default network file");
            }

            offset = l0.load_from_pointer(ptr, offset);
            offset = l1.load_from_pointer(ptr, offset);
            logger.print("Loaded default network");
        }

        Score forward(const std::vector<unsigned int> &features, float phase) const {
            std::array<float, 32> l0_output;
            std::array<float, 1> l1_output;
            l0.forward(features, l0_output);
            l1.forward(l0_output, l1_output);
            return l1_output[0] * 400;
        }
    };
} // namespace nn