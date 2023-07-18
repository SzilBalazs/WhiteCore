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

#include <algorithm>

namespace nn::activations {
    template<typename T, int INT_UPPER_BOUND>
    struct crelu {

        static constexpr T UPPER_BOUND = static_cast<T>(INT_UPPER_BOUND);

        static T forward(T value) {
            return std::clamp(value, static_cast<T>(0), UPPER_BOUND);
        }

        static constexpr T backward(T value) {
            return static_cast<T>(0) < value && value < UPPER_BOUND;
        }
    };
} // namespace nn::activations