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
        if score > 500 or score < -500:
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


def normalize(values, w):
    x = np.array(values)
    y = np.array(w)
    pol = np.polyfit(x, y, 4)
    pol[-1] -= 0.5
    solutions = np.roots(pol)
    real_solutions = [solution for solution in solutions if np.isreal(solution)]
    real_solutions = np.real(real_solutions)
    solution_closest_to_zero = real_solutions[np.abs(real_solutions).argmin()]

    return solution_closest_to_zero


def calc(raw_x, raw_y):
    x = np.array(raw_x)
    y = np.array(raw_y)
    pol = np.polyfit(x, y, 4)
    model = np.poly1d(pol)

    plt.scatter(x, y)
    plt.scatter(x, model(x))

    print("[", end="")
    for i in pol:
        print(i, end=", ")
    print("]")


scores = []
w_y = []
l_y = []

for score in wins:
    scores.append(score)
    w_y.append(wins[score] / counts[score])
    l_y.append(losses[score] / counts[score])

calc(scores, w_y)
calc(scores, l_y)
scale = normalize(scores, w_y)
print("Pawn scale:", scale)
plt.show()
