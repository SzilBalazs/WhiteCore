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
            output.assign(N, 0.0f);

            for (size_t i = 0; i < N; i++) {
                size_t move_index = get_move_index(moves[i]);
                for (size_t j = 0; j < IN; j++) {
                    output[i] += input[j] * get_weight(j, move_index);
                }
            }
        }

    private:
        static constexpr size_t OUT = 64 * 64;
        float *weights;

        float &get_weight(size_t in, size_t out) {
            return weights[out * IN + in];
        }

        size_t get_move_index(const chess::Move &move) {
            return move.get_from() * 64 + move.get_to();
        }
    };

} // namespace nn::layers
