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

#include <cmath>
#include <utility>

namespace search::wdl_model {

    constexpr float win_polynomial[5] = {
            -5.20847751880679e-12, -1.7862526510551246e-09, 2.161371766086046e-06, 0.0013378203669848212, 0.23250708093455283};

    constexpr float loss_polynomial[5] = {
            -5.272795083649799e-12, 1.8936610031314425e-09, 2.1447597868068622e-06, -0.0013634978415372843, 0.24125419419645674};

    constexpr float pawn_scale = 164.886179;

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
