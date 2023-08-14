#  WhiteCore is a C++ chess engine
#  Copyright (c) 2023 Balázs Szilágyi
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

import matplotlib.pyplot as plt
import numpy as np

wins = {}
losses = {}
counts = {}

with open("out.txt", "r") as f:
    for line in f:
        ply, score, wdl = line.split(";")
        ply = int(ply)
        score = int(score)
        wdl = int(wdl)
        if ply > 100 or ply < 10:
            continue
        if score > 700 or score < -700:
            continue

        # Skip positions with blunders because they were generated as low depth data.
        if score > 200 and wdl == -1:
            continue
        if score < -200 and wdl == 1:
            continue

        if score not in wins:
            wins[score] = 0
            losses[score] = 0
            counts[score] = 0

        if wdl == 1:
            wins[score] += 1
        if wdl == -1:
            losses[score] += 1

        counts[score] += 1


def calc(raw_x, raw_y):
    x = np.array(raw_x)
    y = np.array(raw_y)
    pol = np.polyfit(x, y, 4)
    model = np.poly1d(pol)

    print(pol)

    plt.scatter(x, y)
    plt.scatter(x, model(x))


scores = []
w_y = []
l_y = []

for score in wins:
    scores.append(score)
    w_y.append(wins[score] / counts[score])
    l_y.append(losses[score] / counts[score])

calc(scores, w_y)
calc(scores, l_y)
plt.show()

