import glob
import random
import shutil
import subprocess
from pathlib import Path
from datetime import datetime

SEED = 1 # Seed used by random generator
CHANCE = 20 # Percent chance of report collection


def execute(cmd):
    proc = subprocess.run(cmd, capture_output=True)
    print(proc.stdout.decode(errors="ignore"), end="")
    print(proc.stderr.decode(errors="ignore"), end="")
    if proc.returncode != 0:
        print(f"Command '{' '.join(cmd)}' returned {proc.returncode}")
        exit(1)


class Event:
    def __init__(self, path):
        self.root, self.id, self.ts = path.split("/")
        self.timestamp = datetime.fromtimestamp(int(self.ts))
        self.workdir = Path("tmp", self.id)
        self.workdir.mkdir(parents=True, exist_ok=True)
        self.patchfile = Path("tmp", self.id, f"{self.ts}.patch")

    def change_state(self):
        dst = Path("tmp")
        dst.mkdir(parents=True, exist_ok=True)
        for file in Path(self.root, self.id, self.ts).glob("*.cache"):
            shutil.copy(file, dst)

    def record_state(self):
        cmd = ["../bin/leech", "--verbose", f"--workdir={self.workdir}", "commit"]
        execute(cmd)

    def generate_diff(self):
        lastseen = "0000000000000000000000000000000000000000"
        path = Path(self.workdir, self.id)
        if path.exists():
            with open(path, "r") as f:
                lastseen = f.read().strip()

        cmd = [
            "../bin/leech",
            "--verbose",
            f"--workdir={self.workdir}",
            "diff",
            f"--block={lastseen}",
            f"--file={self.patchfile}",
        ]
        execute(cmd)

    def patch_tables(self):
        cmd = [
            "../bin/leech",
            "--verbose",
            f"--workdir={self.workdir}",
            "patch",
            "--field=uid",
            f"--value={self.id}",
            f"--file={self.patchfile}",
        ]
        execute(cmd)

    def work(self):
        print(f" --- {self.id} --- ")
        self.change_state()
        self.record_state()
        if CHANCE > random.randint(1, 100):
            self.generate_diff()
            self.patch_tables()

    def __lt__(self, other):
        return self.timestamp < other.timestamp


def main():
    shutil.rmtree("tmp", ignore_errors=True)
    random.seed()
    events = sorted([Event(path) for path in glob.glob("dumps/SHA=*/*")])
    for ev in events:
        ev.work()


if __name__ == "__main__":
    main()
