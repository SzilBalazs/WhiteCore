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

#include <cassert>
#include <array>

#include "dense_layer.h"

namespace nn::layers {

    template<size_t BUCKETS, size_t IN, size_t OUT>
    struct DenseLayerBucketGradient {
        std::array<DenseLayerGradient<IN, OUT>, BUCKETS> gradients{};

        DenseLayerBucketGradient() = default;

        void operator+=(const DenseLayerBucketGradient &g) {
            for (size_t i = 0; i < BUCKETS; i++) {
                gradients[i] += g.gradients[i];
            }
        }
    };

    template<size_t BUCKETS, size_t IN, size_t OUT, typename T, typename T2, typename ACTIVATION = activations::none<T>>
    struct DenseLayerBucket {

        std::array<DenseLayer<IN, OUT, T, T2, ACTIVATION>, BUCKETS> layers;

        void load_from_file(std::ifstream &file) {
            for (size_t i = 0; i < BUCKETS; i++) {
                layers[i].load_from_file(file);
            }
        }

        int load_from_pointer(const unsigned char *ptr, int offset) {
            for (size_t i = 0; i < BUCKETS; i++) {
                offset = layers[i].load_from_pointer(ptr, offset);
            }

            return offset;
        }

        void randomize(std::mt19937 &mt) {
            for (size_t i = 0; i < BUCKETS; i++) {
                layers[i].randomize(mt);
            }
        }

        void write_to_file(std::ofstream &file) {
            for (size_t i = 0; i < BUCKETS; i++) {
                layers[i].write_to_file(file);
            }
        }

        template<typename QTYPE, int QBIAS_SCALE, int QWEIGHT_SCALE>
        void quantize(std::ofstream &file) {
            for (size_t i = 0; i < BUCKETS; i++) {
                layers[i].template quantize<QTYPE, QBIAS_SCALE, QWEIGHT_SCALE>(file);
            }
        }

        void forward(size_t bucket_index, const std::vector<unsigned int> &input_features, std::array<T2, OUT> &output) const {
            assert(0 <= bucket_index && bucket_index < BUCKETS);
            layers[bucket_index].forward(input_features, output);
        }

        void forward(size_t bucket_index, const std::array<T, IN> &input, std::array<T2, OUT> &output) const {
            assert(0 <= bucket_index && bucket_index < BUCKETS);
            layers[bucket_index].forward(input, output);
        }

        void backward(size_t bucket_index, const std::array<T, OUT> &loss, const std::array<T, IN> &input, const std::array<T, OUT> &output, std::array<T, IN> &input_loss, DenseLayerBucketGradient<BUCKETS, IN, OUT> &gradient) const {
            assert(0 <= bucket_index && bucket_index < BUCKETS);
            static_assert(std::is_same<T, T2>(), "T and T2 are not equal");
            layers[bucket_index].backward(loss, input, output, input_loss, gradient.gradients[bucket_index]);
        }

        void backward(size_t bucket_index, const std::array<T, OUT> &loss, const std::vector<unsigned int> &input_features, const std::array<T, OUT> &output, DenseLayerBucketGradient<BUCKETS, IN, OUT> &gradient) const {
            assert(0 <= bucket_index && bucket_index < BUCKETS);
            static_assert(std::is_same<T, T2>(), "T and T2 are not equal");
            layers[bucket_index].backward(loss, input_features, output, gradient.gradients[bucket_index]);
        }
    };

}
