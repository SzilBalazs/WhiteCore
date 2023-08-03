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

import concurrent
import argparse
import subprocess
from concurrent.futures import ThreadPoolExecutor


def task():
    output = subprocess.check_output(["./WhiteCore", 'bench']).decode("utf8")
    return output


def parse_nps(output):
    tokens = output.split()
    nps_index = tokens.index('nps')
    nps_value = int(tokens[nps_index - 1])
    return nps_value


def main(thread_count):
    executor = ThreadPoolExecutor(max_workers=thread_count)
    futures = [executor.submit(task) for _ in range(thread_count)]

    nps_values = []

    for future in concurrent.futures.as_completed(futures):
        output = future.result()
        nps = parse_nps(output)
        nps_values.append(nps)

    average_nps = sum(nps_values) / len(nps_values)
    print(f"Average NPS: {int(average_nps)}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Calculate average NPS.")
    parser.add_argument("threads", type=int, help="Number of threads to use")
    args = parser.parse_args()
    main(args.threads)
