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
import os
import sys
import time
import wandb
import subprocess
import glob
from datetime import datetime

os.system("rm -rf networks")
os.system("mkdir networks")

wandb.init(
    project="white-core",

    config={
        "learning_rate": 0.001,
        "architecture": 6,
        "eval_influence": 0.9,
        "dataset": glob.glob(f'data/*.plain'),
        "epochs": 20,
        "batch_size": 16384,
        "thread_count": 4
    }
)

commit_hash = subprocess.check_output(["git", "rev-parse", "--short", "HEAD"]).decode("ascii").strip()
current_time = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
wandb.run.name = f"exp-{commit_hash}-{current_time}"

command_string = f"train lr {wandb.run.config['learning_rate']} " \
                 f"epochs {wandb.run.config['epochs']} batch {wandb.run.config['batch_size']} " \
                 f"eval_influence {wandb.run.config['eval_influence']}" \
                 f"threads {wandb.run.config['thread_count']}\n"

p = subprocess.Popen("./WhiteCore", stdout=sys.stdout, stdin=subprocess.PIPE)
p.stdin.write(command_string.encode("utf8"))
p.stdin.flush()

last_iteration = 0
is_running = True

os.remove("log.txt")
time.sleep(20)

while is_running:
    time.sleep(5)
    try:
        f = open("log.txt", "r")
        for line in f.readlines():
            if "END" in line:
                is_running = False
                break
            data = line.split(" ")
            iteration = int(data[0])
            if iteration > last_iteration:
                last_iteration = iteration
                wandb.run.summary["iterations"] = iteration
                wandb.log({"training loss": float(data[1]),
                           "training accuracy": float(data[3]),
                           "validation loss": float(data[4]),
                           "validation accuracy": float(data[5]),
                           "positions per second": int(data[2])})
        f.close()
    except Exception as e:
        print("Failed to sync data with child process", e)

networks = wandb.Artifact('networks', type='models')
networks.add_dir('networks')
wandb.run.log_artifact(networks)

wandb.finish()
