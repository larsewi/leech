import os
from datetime import datetime
import shutil
import subprocess
from abc import ABC, abstractmethod
import pandas as pd
import re
import csv

HOSTS = {
    "hub": "SHA=f8b83ee3ff6d6aec0c772c7dbb4d78ea604d4ce53e7f875c6a88e249ed7fd6e3",
    "centos": "SHA=8b6026383587874304eb95eed145b002c8bf591cbb3acb1598b97e9a9484a8fa",
    "debian": "SHA=5602656702d53c57d14701cf8e0e269623edabc89a0dac4bba604d833900b4ea",
    "rhel": "SHA=25378825585c9d7b6ccdd73db2ffd6cce72601b72d0e5efe55b1bdb6bc7e8b07",
    "ubuntu": "SHA=7119ca477f28b65baee37d8de62629d4c2c98a7a5cbe1455ada86f1f7ecb0608",
}

HEADER_FIELDS = {
    "classes.cache": ["name", "meta"],
    "variables.cache": ["namespace", "bundle", "name", "type", "value", "meta"],
    "lastseen.cache": ["direction", "hostkey", "address", "interval", "lastseen"],
    "software.cache": ["name", "version", "architecture"],
    "patch.cache": ["name", "version", "architecture", "status"],
    "execution_log.cache": [
        "promise_hash",
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
}

report_df = pd.DataFrame(
    columns=[
        "Timestamp",
        "Hostname",
        "Classes size",
        "Execution log size",
        "Last seen size",
        "Patch size",
        "Software size",
        "Variables size",
        "Full state size",
        "CFEngine report size",
        "Leech report size",
    ]
)

LOGFILE = "simulate/leech.log"


class Event(ABC):
    def __init__(self, hostname, hostkey, timestamp):
        self.hostname = hostname
        self.hostkey = hostkey
        self.timestamp = timestamp

    @abstractmethod
    def work(self):
        pass

    def __lt__(self, other):
        return self.timestamp < other.timestamp


class Commit(Event):
    def __init__(self, hostname, hostkey, timestamp, tables):
        super().__init__(hostname, hostkey, timestamp)
        self.tables = tables

    def work(self):
        print(f"*** Commit {self.timestamp} {self.hostname} ***")
        os.makedirs(os.path.join("simulate", self.hostname, ".leech"), exist_ok=True)

        # Copy a sanatized CSV with table header fields to leech work directory
        for table in self.tables:
            basename = os.path.basename(table)
            with open(table, "r", newline="") as r:
                reader = csv.reader(r)
                with open(
                    os.path.join("simulate", self.hostname, ".leech", basename), "w", newline=""
                ) as w:
                    writer = csv.writer(w)
                    writer.writerow(HEADER_FIELDS[basename])
                    writer.writerows(reader)
            # shutil.copy(table, os.path.join("simulate", self.hostname, ".leech"))

        cwd = os.getcwd()
        os.chdir(os.path.join("simulate", self.hostname))
        command = ["../../bin/leech", "--info", "commit"]
        p = subprocess.run(command, capture_output=True)
        os.chdir(cwd)
        print(p.stdout.decode(errors="ignore"), end="")
        print(p.stderr.decode(errors="ignore"), end="")
        if p.returncode != 0:
            print("Command '%s' returned %d" % (" ".join(command), p.returncode))
            exit(1)


def strip_surrounding_comments(buf):
    left = ""
    right = ""

    for ch in buf:
        if re.match(r"^#(?!\r\n).*\r\n$", left):
            break
        else:
            left = left + ch

    for ch in reversed(buf):
        if re.match(r"^#(?!\r\n).*\r\n$", right):
            break
        else:
            right = ch + right

    return buf[len(left) : -len(right)]


class Patch(Event):
    def __init__(self, hostname, hostkey, timestamp, dump_file):
        self.dump_file = dump_file
        super().__init__(hostname, hostkey, timestamp)

    def work(self):
        print(f"*** Delta {self.timestamp} {self.hostname} ***")
        lastseen_block = "0000000000000000000000000000000000000000"
        lastseen_block_path = os.path.join("simulate/hub/.leech", self.hostkey)

        if os.path.exists(lastseen_block_path):
            with open(lastseen_block_path, "r") as f:
                lastseen_block = f.read().strip()
        patch_file = os.path.join(
            os.getcwd(),
            "simulate",
            self.hostname,
            f".leech/{int(self.timestamp.timestamp())}_patchfile",
        )

        cwd = os.getcwd()
        os.chdir(os.path.join("simulate", self.hostname))
        command = [
            "../../bin/leech",
            "--info",
            "delta",
            "--block",
            lastseen_block,
            "--file",
            patch_file,
        ]
        p = subprocess.run(command, capture_output=True)
        os.chdir(cwd)
        print(p.stdout.decode(), end="")
        print(p.stderr.decode(), end="")
        if p.returncode != 0:
            print("Command '%s' returned %d" % (" ".join(command), p.returncode))
            exit(1)

        print(f"*** Patch {self.timestamp} {self.hostname} ***")
        os.chdir("simulate/hub")
        command = [
            "../../bin/leech",
            "--info",
            "patch",
            "--field",
            "uid",
            "--value",
            self.hostkey,
            "--file",
            patch_file,
        ]
        p = subprocess.run(command, capture_output=True)
        os.chdir(cwd)
        print(p.stdout.decode(errors="ignore"), end="")
        print(p.stderr.decode(errors="ignore"), end="")
        if p.returncode != 0:
            print("Command '%s' returned %d" % (" ".join(command), p.returncode))
            exit(1)

        work_dir = os.path.join("simulate", self.hostname, ".leech")

        classes_path = os.path.join(work_dir, "classes.cache")
        classes_size = os.path.getsize(classes_path) if os.path.exists(classes_path) else 0

        execlog_path = os.path.join(work_dir, "execution_log.cache")
        execlog_size = os.path.getsize(execlog_path) if os.path.exists(execlog_path) else 0

        lastseen_path = os.path.join(work_dir, "lastseen.cache")
        lastseen_size = os.path.getsize(lastseen_path) if os.path.exists(lastseen_path) else 0

        patch_path = os.path.join(work_dir, "patch.cache")
        patch_size = os.path.getsize(patch_path) if os.path.exists(patch_path) else 0

        software_path = os.path.join(work_dir, "software.cache")
        software_size = os.path.getsize(software_path) if os.path.exists(software_path) else 0

        variables_path = os.path.join(work_dir, "variables.cache")
        variables_size = os.path.getsize(variables_path) if os.path.exists(variables_path) else 0

        fullstate_size = (
            classes_size
            + execlog_size
            + lastseen_size
            + patch_size
            + software_size
            + variables_size
        )
        leech_size = os.path.getsize(patch_file)

        cfengine_size = 0
        tables_include = ["CLD", "VAD", "LSD", "SDI", "SPD", "ELD"]
        tables_exclude = ["EXS", "PRD", "CNG", "CND", "MOM", "MOY", "MOH", "PLG"]

        with open(self.dump_file, "r") as f:
            buf = f.read()

        buf = strip_surrounding_comments(buf)

        report_df.loc[len(report_df)] = [
            self.timestamp,
            self.hostname,
            classes_size,
            execlog_size,
            lastseen_size,
            patch_size,
            software_size,
            variables_size,
            fullstate_size,
            cfengine_size,
            leech_size,
        ]
        print(report_df.tail(10))


def main():
    events = []

    subprocess.run("rm -rf simulate/leech.log simulate/**/.leech simulate/report.csv", shell=True)

    for hostname, hostkey in HOSTS.items():
        for dirpath, _, filenames in os.walk(os.path.join("simulate", hostname)):
            if len(filenames) == 0:
                continue

            if "cache_dumps" in dirpath:
                timestamp = datetime.fromtimestamp(int(dirpath.split("/")[-1]))
                tables = [os.path.join(dirpath, f) for f in filenames]
                event = Commit(hostname, hostkey, timestamp, tables)
                events.append(event)
            elif "report_dumps" in dirpath:
                for filename in filenames:
                    dump_file = os.path.join(dirpath, filename)
                    timestamp = datetime.fromtimestamp(int(filename.split("_")[0]))
                    event = Patch(hostname, hostkey, timestamp, dump_file)
                    events.append(event)

    events = sorted(events)
    for event in events:
        event.work()


if __name__ == "__main__":
    main()
