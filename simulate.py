import os
from datetime import datetime
import shutil
import subprocess
from abc import ABC, abstractmethod

HOSTS = {
    "hub":    "SHA=0cb07f5bff5865ca5268dc1a5cc8599a7a4e6894d4ee954913016cc699b84e3f",
    "centos": "SHA=7254f21cc164054e92c513abf4079d0671fd43341cfaf40fedad6bbe00dbdd46",
    "debian": "SHA=f373628225ce71586ae38a844c63a6da1fbe124bb4770b805c1bc4a133a3482e",
    "rhel":   "SHA=15580be79bd3b71c1b7e0cddf38ad84c0228cefdfc8282ab7f3207fa6dbaf228",
    "ubuntu": "SHA=6c1baabfc29d73409938f0be462bfecd8492b5a47f475c78c8f1817f959053ba",
}

class Event(ABC):
    def __init__(self, hostname, hostkey, timestamp, workdir):
        self.hostname = hostname
        self.hostkey = hostkey
        self.timestamp = timestamp
        self.workdir = workdir

    @abstractmethod
    def work(self):
        pass

    def __lt__(self, other):
        return self.timestamp < other.timestamp

class Commit(Event):
    def __init__(self, hostname, hostkey, timestamp, workdir, tables):
        super().__init__(hostname, hostkey, timestamp, workdir)
        self.tables = tables

    def work(self):
        print(f"*** Commit {self.timestamp} {self.hostname} ***")
        os.makedirs(os.path.join(self.workdir, ".leech"), exist_ok=True)

        for table in self.tables:
            print("Copying '%s' to '%s'" % (table, os.path.join(self.workdir, ".leech")))
            shutil.copy(table, os.path.join(self.workdir, ".leech"))

        command = "cd %s && ../../bin/leech --verbose commit" % self.workdir
        print("Running command: %s" % command)
        p = subprocess.run(command, shell=True)
        if p.returncode != 0:
            print("Command '%s' returned %d" % (command, p.returncode))
            exit(1)

class Patch(Event):
    def __init__(self, hostname, hostkey, timestamp, workdir):
        super().__init__(hostname, hostkey, timestamp, workdir)

    def work(self):
        print(f"*** Delta {self.timestamp} {self.hostname} ***")
        lastseen_block = "0000000000000000000000000000000000000000"
        lastseen_block_path = os.path.join(self.workdir, "../hub/.leech", self.hostkey)
        if os.path.exists(lastseen_block_path):
            print("Loading last seen block from %s" % lastseen_block_path)
            with open(lastseen_block_path, "r") as f:
                lastseen_block = f.read().strip()
        patch_file = os.path.join(self.workdir, ".leech", "%s_patchfile" % self.timestamp.timestamp())

        command = "cd %s && ../../bin/leech --verbose delta --block %s --file %s" % (self.workdir, lastseen_block, patch_file)
        print("Running command: %s" % command)
        p = subprocess.run(command, shell=True)
        if p.returncode != 0:
            print("Command '%s' returned %d" % (command, p.returncode))
            exit(1)

        print(f"*** Patch {self.timestamp} {self.hostname} ***")
        command = "cd %s && ../../bin/leech --verbose patch --field %s --value %s --file %s" %(os.path.join(os.getcwd(), "simulate/hub"), "host_key", self.hostkey, patch_file)
        print("Running command: %s" % command)
        p = subprocess.run(command, shell=True)
        if p.returncode != 0:
            print("Command '%s' returned %d" % (command, p.returncode))
            exit(1)

def main():
    events = []

    subprocess.run("rm -rf simulate/**/.leech", shell=True)

    for hostname, hostkey in HOSTS.items():
        workdir = os.path.join(os.getcwd(), "simulate", hostname)

        for dirpath, _, filenames in os.walk(os.path.join("simulate", hostname)):
            if len(filenames) == 0:
                continue

            # for filename in [os.path.join(dirpath, f) for f in filenames]:
            #     with open(filename, "r", newline="") as f:
            #         reader = csv.reader(f)
            #         with open("%s.tmp" % filename, "w", newline="") as g:
            #             writer = csv.writer(g)
            #             writer.writerows(reader)
            #     os.remove(filename)
            #     os.rename("%s.tmp" % filename, filename)

            if "table_dumps" in dirpath:
                timestamp = datetime.fromtimestamp(int(dirpath.split("/")[-1]))
                tables = [os.path.join(dirpath, f) for f in filenames]
                event = Commit(hostname, hostkey, timestamp, workdir, tables)
                events.append(event)
            elif "report_dumps" in dirpath:
                for filename in filenames:
                    timestamp = datetime.fromtimestamp(int(filename.split("_")[0]))
                    event = Patch(hostname, hostkey, timestamp, workdir)
                    events.append(event)

    events = sorted(events)

    for event in events:
        event.work()

if __name__ == "__main__":
    main()
