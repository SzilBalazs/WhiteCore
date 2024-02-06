// WhiteCore is a C++ chess engine
// Copyright (c) 2023-2024 Balázs Szilágyi
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

#include "../chess/constants.h"
#include "../external/incbin/incbin.h"
#include "../utils/utilities.h"
#include "activations/crelu.h"
#include "layers/accumulator.h"
#include "layers/dense_layer_bucket.h"

#include <cassert>

namespace nn {

    INCBIN(DefaultNetwork, "tmp.bin");

    class NNUE {

    public:
        static constexpr int QSCALE = 64;

        NNUE() {
            const unsigned char *data = gDefaultNetworkData;

            int magic;
            std::memcpy(&magic, data, sizeof(int));
            int offset = sizeof(int);

            if (magic != MAGIC) {
                print("Invalid default network file with magic", magic);
                throw std::invalid_argument("Invalid default network file with magic" + std::to_string(magic));
            }

            offset = accumulator.load_from_pointer(data, offset);
            offset = l1.load_from_pointer(data, offset);

            assert(offset == gDefaultNetworkSize);
        }

        /*void load_from_file(const std::string &nnue_path) {
            std::ifstream file(nnue_path, std::ios::in | std::ios::binary);
            if (!file.is_open()) {
                logger.print("Unable to open:", nnue_path);
                throw std::invalid_argument("Unable to open: " + nnue_path);
            } else {
                int magic;
                file.read(reinterpret_cast<char *>(&magic), sizeof(magic));

                if (magic != MAGIC) {
                    logger.print("Invalid network file", nnue_path, "with magic", magic);
                    throw std::invalid_argument("Invalid network file with magic " + std::to_string(magic));
                }

                accumulator.load_from_file(file);
                l1.load_from_file(file);

                file.close();

                logger.print("Loaded network file", nnue_path);
            }
        }*/

        void refresh(const std::vector<unsigned int> &features) {
            accumulator.refresh(features);
        }

        void activate(Piece piece, unsigned int sq) {
            assert(piece.is_ok());
            accumulator.add_feature(get_feature_index(piece, sq));
        }

        void deactivate(Piece piece, unsigned int sq) {
            assert(piece.is_ok());
            accumulator.remove_feature(get_feature_index(piece, sq));
        }

        Score evaluate(Color stm) {
            accumulator.push(l0_output);
            l1.forward(stm, l0_output, l1_output);
            int32_t score = l1_output[0];
            if (stm == BLACK) score *= -1;
            return (score * 400) / (QSCALE * QSCALE);
        }

        static constexpr unsigned int get_feature_index(Piece piece, unsigned int sq) {
            return (piece.color == WHITE) * 384 + piece.type * 64 + sq;
        }

    private:
        static constexpr int MAGIC = -6;
        static constexpr size_t L1_SIZE = 512;

        alignas(64) std::array<int16_t, L1_SIZE> l0_output;
        alignas(64) std::array<int32_t, 1> l1_output;

        layers::Accumulator<768, L1_SIZE, int16_t, activations::crelu<int16_t, QSCALE>> accumulator;
        layers::DenseLayerBucket<2, L1_SIZE, 1, int16_t, int32_t, activations::none<int16_t>> l1;
    };

} // namespace nn
