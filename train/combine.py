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

import glob
import os
import argparse


def combine_data(directory, output):
    if not os.path.exists(directory):
        print(f"Directory {directory} does not exist.")
        return

    files = glob.glob(f"{directory}/*.plain")

    if not files:
        print("No .plain files found in the given directory.")
        return

    stats = [0, 0, 0]
    filtered_early = 0

    with open(output, "w") as outfile:
        for file in files:
            with open(file, "r") as infile:
                print(f"Reading {file}...")
                for line in infile:
                    fen, ply, move, score, wdl, tmp = line.split(";")

                    if int(ply) <= 10:
                        filtered_early += 1
                        continue

                    wdl = int(wdl) + 1
                    stats[wdl] += 1
                    outfile.write(line)

    total = sum(stats)
    print(f"Black wins {stats[0] / total * 100}% - "
          f"Draws {stats[1] / total * 100}% - "
          f"Wins wins {stats[2] / total * 100}%")
    print(f"Filtered early positions: {filtered_early}")
    print(f"Data from {len(files)} files has been successfully combined into {output}")


def split_data(input_path, rate):
    os.system(f'echo "split input {input_path} rate {rate}\nquit\n" | ./WhiteCore')


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Combine plain files and shuffle them")
    parser.add_argument("directory", type=str, help="the directory to get plain files")
    parser.add_argument("split_rate", type=int, help="train:validation data ratio")
    args = parser.parse_args()

    for file in ["data.plain", "train.plain", "validation.plain"]:
        try:
            os.remove(file)
        except FileNotFoundError:
            pass

    combine_data(args.directory, "data.plain")
    split_data("data.plain", args.split_rate)
