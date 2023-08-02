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
from concurrent.futures import ThreadPoolExecutor
import subprocess


THREAD_COUNT = 4


def task():
    output = subprocess.check_output(["./WhiteCore-v0-2", 'bench']).decode("utf8")
    return output


executor = ThreadPoolExecutor(max_workers=THREAD_COUNT)
futures = []
for i in range(THREAD_COUNT):
    futures.append(executor.submit(task))

for future in concurrent.futures.as_completed(futures):
    print(future.result())
