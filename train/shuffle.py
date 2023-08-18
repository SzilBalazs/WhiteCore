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
import random
import argparse


def combine_and_shuffle(directory, output):

    files = glob.glob(f'{directory}/*.plain')
    lines = []

    for filename in files:
        print("Reading", filename)
        with open(filename, 'r') as file:
            lines.extend(file.readlines())

    print("Shuffling...")
    random.shuffle(lines)

    print("Writing to", output)
    with open(output, 'w') as file:
        file.writelines(lines)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Combine plain files and shuffle them')
    parser.add_argument('directory', type=str, help='the directory to get plain files')
    parser.add_argument('output', type=str, help='the output file')
    args = parser.parse_args()

    combine_and_shuffle(args.directory, args.output)
