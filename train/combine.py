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

    files = glob.glob(f'{directory}/*.plain')

    if not files:
        print("No .plain files found in the given directory.")
        return

    stats = [0, 0, 0]

    with open(output, 'w') as outfile:
        for file in files:
            with open(file, 'r') as infile:
                print(f"Reading {file}...")
                for line in infile:
                    outfile.write(line)
                    tokens = line.split(";")
                    wdl = int(tokens[4]) + 1
                    stats[wdl] += 1

    total = sum(stats)
    print(f"Black wins {stats[0] / total * 100}% - "
          f"Draws {stats[1] / total * 100}% - "
          f"Wins wins {stats[2] / total * 100}%")
    print(f"Data from {len(files)} files has been successfully combined into {output}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Combine plain files and shuffle them')
    parser.add_argument('directory', type=str, help='the directory to get plain files')
    parser.add_argument('output', type=str, help='the output file')
    args = parser.parse_args()

    combine_data(args.directory, args.output)
