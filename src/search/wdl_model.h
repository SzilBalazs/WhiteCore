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

#include <cmath>
#include <utility>

namespace search::wdl_model {

    constexpr float win_polynomial[5] = {
            -5.20847752e-12, -1.78625265e-09, 2.16137177e-06, 1.33782037e-03, 2.32507081e-01};

    constexpr float loss_polynomial[5] = {
            -5.27279508e-12, 1.89366100e-09, 2.14475979e-06, -1.36349784e-03, 2.41254194e-01};

    std::pair<int, int> cp_to_wl(Score score) {

        if (score > 500) {
            return {1000, 0};
        } else if (score < -500) {
            return {0, 1000};
        }

        float cp = score;
        float win_chance = win_polynomial[0];
        float loss_chance = loss_polynomial[0];

        for (size_t i = 1; i < 5u; i++) {
            win_chance *= cp;
            win_chance += win_polynomial[i];

            loss_chance *= cp;
            loss_chance += loss_polynomial[i];
        }

        int w = std::round(win_chance * 1000);
        int l = std::round(loss_chance * 1000);

        return {std::clamp(w, 0, 1000), std::clamp(l, 0, 1000)};
    }
} // namespace search::wdl_model
