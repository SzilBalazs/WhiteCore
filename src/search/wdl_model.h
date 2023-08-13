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

#include "../chess/constants.h"

#include <utility>
#include <cmath>

namespace search {

    constexpr float win_polynomial[5] = {
            -2.48486993e-12, -1.35086324e-09, 1.63515294e-06, 1.28794334e-03, 2.27129636e-01
    };

    constexpr float loss_polynomial[5] = {
            -2.33054260e-12, 1.40330209e-09, 1.53833569e-06, -1.31063191e-03, 2.43447833e-01
    };

    std::pair<int, int> cp_to_wl(Score score) {
        float cp = score;
        float win_chance = win_polynomial[0];
        float loss_chance = loss_polynomial[0];

        for (size_t i = 1; i < 5u; i++) {
            win_chance *= cp;
            win_chance += win_polynomial[i];

            loss_chance *= cp;
            loss_chance += loss_polynomial[i];
        }

        return {std::round(win_chance * 1000), std::round(loss_chance * 1000)};
    }
}
