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
import struct
import numpy as np
import matplotlib.pyplot as plt


col_labels = ["WK", "WP", "WN", "WB", "WR", "WQ",
              "BK", "BP", "BN", "BB", "BR", "BQ"]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input_file', help='Path to the network file')
    args = parser.parse_args()

    with open(args.input_file, 'rb') as f:
        magic_raw = f.read(4)
        magic = struct.unpack('i', magic_raw)[0]
        assert magic == -6

        data = f.read(512 * 2)
        biases = struct.unpack('512h', data)

        data = f.read(768 * 512 * 2)
        weights = struct.unpack('393216h', data)
        weights = np.array(weights)
        weights = weights.reshape((768, 512))
        fig, axs = plt.subplots(8, 12, figsize=(6, 4))
        plt.subplots_adjust(hspace=0.1, wspace=0.1)
        for base in range(64):
            for neuron in range(8):
                numbers = weights[:, neuron + base * 8]
                numbers = np.array(numbers).reshape((96, 8))

                row_start = 0
                row_end = 7
                for i in range(12):
                    x = numbers[row_start:row_end, :]
                    img = np.interp(x, (-128, 128), (0, 255)).astype(np.uint8)
                    axs[neuron][i].set_xticks([])
                    axs[neuron][i].set_yticks([])
                    axs[neuron][i].imshow(img, cmap='copper')
                    row_start += 8
                    row_end += 8

            for i in range(12):
                axs[7][i].set_xlabel(col_labels[i])

            for i in range(8):
                axs[i][0].set_ylabel(i + base * 8 + 1)

            plt.savefig(f'img/network_{base}.png')


if __name__ == '__main__':
    main()
