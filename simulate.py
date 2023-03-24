import os
from datetime import datetime
import shutil
import subprocess
from abc import ABC, abstractmethod
import pandas as pd
import re

HOSTS = {
    "hub":    "SHA=0cb07f5bff5865ca5268dc1a5cc8599a7a4e6894d4ee954913016cc699b84e3f",
    "centos": "SHA=7254f21cc164054e92c513abf4079d0671fd43341cfaf40fedad6bbe00dbdd46",
    "debian": "SHA=f373628225ce71586ae38a844c63a6da1fbe124bb4770b805c1bc4a133a3482e",
    "rhel":   "SHA=15580be79bd3b71c1b7e0cddf38ad84c0228cefdfc8282ab7f3207fa6dbaf228",
    "ubuntu": "SHA=6c1baabfc29d73409938f0be462bfecd8492b5a47f475c78c8f1817f959053ba",
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
