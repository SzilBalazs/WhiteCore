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

import chess.pgn
import argparse
import matplotlib.pyplot as plt
import re


def extract_time(comment):
    try:
        match = re.search(r'(\d+\.?\d*)s', comment)
        time_spent = match.group(1)
        return float(time_spent)
    except ValueError:
        pass


def extract_depth(comment):
    try:
        match = re.search(r'/(\d+)', comment)
        depth_reached = match.group(1)
        return int(depth_reached)
    except ValueError:
        pass


def parse_comments(node):
    p1_times_spent = []
    p1_depths_reached = []
    p2_times_spent = []
    p2_depths_reached = []

    while not node.is_end():
        next_node = node.variation(0)
        comment = next_node.comment
        time_spent = extract_time(comment)
        depth_reached = extract_depth(comment)

        if node.turn() == 0:
            p1_times_spent.append(time_spent)
            p1_depths_reached.append(depth_reached)
        else:
            p2_times_spent.append(time_spent)
            p2_depths_reached.append(depth_reached)

        node = next_node

    return p1_times_spent, p1_depths_reached, p2_times_spent, p2_depths_reached


def plot_game(times_spent, depths_reached, player, fig, ax):

    color = 'tab:red'
    ax.set_xlabel('Move number')
    ax.set_ylabel('Time spent (seconds)', color=color)
    ax.plot(times_spent, color=color, marker='o')
    ax.tick_params(axis='y', labelcolor=color)

    ax2 = ax.twinx()
    color = 'tab:blue'
    ax2.set_ylabel('Depth reached', color=color)
    ax2.plot(depths_reached, color=color, marker='o')
    ax2.tick_params(axis='y', labelcolor=color)

    plt.title(player)
    plt.grid(True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('pgn_file', help='Path to PGN file containing the game')
    args = parser.parse_args()

    with open(args.pgn_file) as f:
        game = chess.pgn.read_game(f)

    time_control = game.headers.get('TimeControl', None)
    player_1 = game.headers.get('White', None)
    player_2 = game.headers.get('Black', None)

    (p1_times, p1_depths, p2_times, p2_depths) = parse_comments(game)

    fig, ax = plt.subplots(2)
    plt.subplots_adjust(hspace=0.5)
    fig.suptitle(f'{player_1} vs {player_2} with {time_control} tc')

    plot_game(p1_times, p1_depths, player_1, fig, ax[0])
    plot_game(p2_times, p2_depths, player_2, fig, ax[1])

    plt.show()


if __name__ == '__main__':
    main()
