import glob
import random
import shutil
import subprocess
from pathlib import Path
from datetime import datetime
import os
import json

SEED = 1  # Seed used by random generator
CHANCE = 20  # Percent chance of report collection
HUB_ID = "SHA=b9353fd"
CSV_MODULE = "../lib/.libs/leech_csv.so"
PSQL_MODULE = "../lib/.libs/leech_psql.so"
PSQL_PARAMS = "dbname=leech"

LEECH_CONFIG = {
    "version": "0.1.0",
    "tables": {
        "CLD": {
            "primary_fields": ["name"],
            "subsidiary_fields": ["meta"],
            "source": {
                "params": "tmp/classes.cache",
                "schema": "leech",
                "table_name": "classes",
                "callbacks": CSV_MODULE,
            },
            "destination": {
                "params": PSQL_PARAMS,
                "schema": "leech",
                "table_name": "classes",
                "callbacks": PSQL_MODULE,
            },
        },
        "VAD": {
            "primary_fields": ["namespace", "bundle", "name"],
            "subsidiary_fields": ["type", "value", "meta"],
            "source": {
                "params": "tmp/variables.cache",
                "schema": "leech",
                "table_name": "variables",
                "callbacks": CSV_MODULE,
            },
            "destination": {
                "params": PSQL_PARAMS,
                "schema": "leech",
                "table_name": "variables",
                "callbacks": PSQL_MODULE,
            },
        },
        "LSD": {
            "primary_fields": ["direction", "hostkey"],
            "subsidiary_fields": ["address", "interval", "lastseen"],
            "source": {
                "params": "tmp/lastseen.cache",
                "schema": "leech",
                "table_name": "lastseen",
                "callbacks": CSV_MODULE,
            },
            "destination": {
                "params": PSQL_PARAMS,
                "schema": "leech",
                "table_name": "lastseen",
                "callbacks": PSQL_MODULE,
            },
        },
        "SDI": {
            "primary_fields": ["name", "version", "architecture"],
            "subsidiary_fields": [],
            "source": {
                "params": "tmp/software.cache",
                "schema": "leech",
                "table_name": "software",
                "callbacks": CSV_MODULE,
            },
            "destination": {
                "params": PSQL_PARAMS,
                "schema": "leech",
                "table_name": "software",
                "callbacks": PSQL_MODULE,
            },
        },
        "SPD": {
            "primary_fields": ["name", "version", "architecture"],
            "subsidiary_fields": ["status"],
            "source": {
                "params": "tmp/patch.cache",
                "schema": "leech",
                "table_name": "patch",
                "callbacks": CSV_MODULE,
            },
            "destination": {
                "params": PSQL_PARAMS,
                "schema": "leech",
                "table_name": "patch",
                "callbacks": PSQL_MODULE,
            },
        },
        "ELD": {
            "primary_fields": ["promise_hash"],
            "subsidiary_fields": [
                "policy_filename",
                "release_id",
                "promise_outcome",
                "namespace",
                "bundle",
                "promise_type",
                "promiser",
                "stack_path",
                "handle",
                "promisee",
                "messages",
                "line_number",
                "policy_file_hash",
            ],
            "source": {
                "params": "tmp/execution_log.cache",
                "schema": "leech",
                "table_name": "execution_log",
                "callbacks": CSV_MODULE,
            },
            "destination": {
                "params": "tmp/execution_log.csv",
                "schema": "leech",
                "table_name": "execution_log",
                "callbacks": CSV_MODULE,
            },
        },
    },
}


def execute(cmd):
    print(f"Executing command: '{' '.join(cmd)}' from '{os.getcwd()}'")
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
        self.hub_workdir = Path("tmp", HUB_ID)
        self.hub_workdir.mkdir(parents=True, exist_ok=True)
        self.patchfile = Path("tmp", self.id, f"{self.ts}.patch")
        with open(Path(self.workdir, "leech.json"), "w") as f:
            json.dump(LEECH_CONFIG, f, indent=2)

    def change_state(self):
        dst = Path("tmp")
        dst.mkdir(parents=True, exist_ok=True)
        for file in Path(self.root, self.id, self.ts).glob("*.cache"):
            shutil.copy(file, dst)

    def record_state(self):
        cmd = ["../bin/leech", "--debug", f"--workdir={self.workdir}", "commit"]
        execute(cmd)

    def generate_diff(self):
        lastseen = "0000000000000000000000000000000000000000"
        path = Path(self.hub_workdir, self.id)
        if path.exists():
            with open(path, "r") as f:
                lastseen = f.read().strip()

        cmd = [
            "../bin/leech",
            "--debug",
            f"--workdir={self.workdir}",
            "diff",
            f"--block={lastseen}",
            f"--file={self.patchfile}",
        ]
        execute(cmd)

    def patch_tables(self):
        cmd = [
            "../bin/leech",
            "--debug",
            f"--workdir={self.hub_workdir}",
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
    execute(["dropdb", "--if-exists", "leech"])
    execute(["createdb", "leech"])
    random.seed()
    events = sorted([Event(path) for path in glob.glob("dumps/SHA=*/*")])
    for ev in events:
        ev.work()


if __name__ == "__main__":
    main()
