import os
from datetime import datetime
import shutil
import subprocess
from abc import ABC, abstractmethod
import pandas as pd
import re

HOSTS = {
    "hub":    "SHA=f8b83ee3ff6d6aec0c772c7dbb4d78ea604d4ce53e7f875c6a88e249ed7fd6e3",
    "centos": "SHA=8b6026383587874304eb95eed145b002c8bf591cbb3acb1598b97e9a9484a8fa",
    "debian": "SHA=5602656702d53c57d14701cf8e0e269623edabc89a0dac4bba604d833900b4ea",
    "rhel":   "SHA=25378825585c9d7b6ccdd73db2ffd6cce72601b72d0e5efe55b1bdb6bc7e8b07",
    "ubuntu": "SHA=7119ca477f28b65baee37d8de62629d4c2c98a7a5cbe1455ada86f1f7ecb0608",
}

report_df = pd.DataFrame(columns=["Timestamp", "Hostname", "Classes size", "Execution log size", "Last seen size", "Patch size", "Software size", "Variables size", "Full state size", "CFEngine report size", "Leech report size"])


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

        for table in self.tables:
            shutil.copy(table, os.path.join("simulate", self.hostname, ".leech"))

        cwd = os.getcwd()
        os.chdir(os.path.join("simulate", self.hostname))
        command = ["../../bin/leech", "--info", "commit"]
        p = subprocess.run(command)
        os.chdir(cwd)
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
        patch_file = os.path.join(os.getcwd(), "simulate", self.hostname, f".leech/{int(self.timestamp.timestamp())}_patchfile")

        cwd = os.getcwd()
        os.chdir(os.path.join("simulate", self.hostname))
        command = ["../../bin/leech", "--info", "delta", "--block", lastseen_block, "--file", patch_file]
        p = subprocess.run(command)
        os.chdir(cwd)
        if p.returncode != 0:
            print("Command '%s' returned %d" % (" ".join(command), p.returncode))
            exit(1)

        print(f"*** Patch {self.timestamp} {self.hostname} ***")
        os.chdir("simulate/hub")
        command = ["../../bin/leech", "--info", "patch", "--field", "uid", "--value", self.hostkey, "--file", patch_file]
        p = subprocess.run(command)
        os.chdir(cwd)
        if p.returncode != 0:
            print("Command '%s' returned %d" % (" ".join(command), p.returncode))
            exit(1)

        classes_size = os.path.getsize(os.path.join("simulate", self.hostname, ".leech/classes.cache"))
        execlog_size = os.path.getsize(os.path.join("simulate", self.hostname, ".leech/execution_log.cache"))
        lastseen_size = os.path.getsize(os.path.join("simulate", self.hostname, ".leech/lastseen.cache"))
        patch_size = os.path.getsize(os.path.join("simulate", self.hostname, ".leech/patch.cache"))
        software_size = os.path.getsize(os.path.join("simulate", self.hostname, ".leech/software.cache"))
        variables_size = os.path.getsize(os.path.join("simulate", self.hostname, ".leech/variables.cache"))
        fullstate_size = classes_size + execlog_size + lastseen_size + patch_size + software_size + variables_size
        leech_size = os.path.getsize(patch_file)

        cfengine_size = 0
        tables_include = ["CLD", "VAD", "LSD", "SDI", "SPD", "ELD"]
        tables_exclude = ["EXS", "PRD", "CNG", "CND", "MOM", "MOY", "MOH", "PLG"]

        with open(self.dump_file, "r") as f:
            buf = f.read()

        buf = strip_surrounding_comments(buf)


        report_df.loc[len(report_df)] = [self.timestamp, self.hostname, classes_size, execlog_size, lastseen_size, patch_size, software_size, variables_size, fullstate_size, cfengine_size, leech_size]

def main():
    events = []

    subprocess.run("rm -rf simulate/**/.leech simulate/report.csv", shell=True)

    for hostname, hostkey in HOSTS.items():
        for dirpath, _, filenames in os.walk(os.path.join("simulate", hostname)):
            if len(filenames) == 0:
                continue

            if "table_dumps" in dirpath:
                timestamp = datetime.fromtimestamp(int(dirpath.split("/")[-1]))
                tables = [os.path.join(dirpath, f) for f in filenames]
                event = Commit(hostname, hostkey, timestamp, tables)
                events.append(event)
            elif "report_dumps" in dirpath:
                for filename in filenames:
                    timestamp = datetime.fromtimestamp(int(filename.split("_")[0]))
                    event = Patch(hostname, hostkey, timestamp, filename)
                    events.append(event)

    events = sorted(events)
    for event in events:
        event.work()

if __name__ == "__main__":
    main()
