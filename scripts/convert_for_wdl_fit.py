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

import argparse


def main(in_file):

    out_file = open("out.txt", "w")

    with open(in_file, "r") as f:
        for line in f:
            data = line.split(";")
            assert len(data) == 6
            fen, ply, move, score, wdl, tmp = data
            stm = 1 if "w" in fen else -1
            score = int(score)
            ply = int(ply)
            wdl = int(wdl) * stm
            out_file.write("{0};{1};{2}\n".format(ply, score, wdl))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process input file")
    parser.add_argument("input", help="The input file to process")

    args = parser.parse_args()
    main(args.input)
