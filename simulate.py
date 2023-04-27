import os
from datetime import datetime
import subprocess
from abc import ABC, abstractmethod
import pandas as pd
import re
import csv
import io
import zlib
import lzma
import bz2
import gzip
from timeit import default_timer as timer

CF_BUFSIZE = 4096
CF_INBAND_OFFSET = 8
CF_MAXTRANSSIZE = CF_BUFSIZE - CF_INBAND_OFFSET - 64

HOSTS = {
    "hub": "SHA=b9353fd9e5ac7a74327610e38eaa6f7636c655707cbb2c4f772e633b72703217",
    "centos": "SHA=f092c2783c2be5dd671be584b9d63904be50887f5bfa4a4e7609829b1994496f",
    "debian": "SHA=108dbe4b97c3ed4e62e6c4a30720e590ae3c3d7301c62f09943bc11f928f3054",
    "rhel": "SHA=c48ba0b489cd6f0f973e6c4c9bdea6e6a5a7a16bfdc90c6fe2d15da235c7ecfe",
    "ubuntu": "SHA=5c976b26f2e96ca8b2f9b6e583af5fe2d1b53d17d84d00fffa8eae7aa1ecab59",
}

COMMITS = {
    "hub": 0,
    "centos": 0,
    "debian": 0,
    "rhel": 0,
    "ubuntu": 0,
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

COLLECTIONS = {}


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
                    os.path.join("simulate", self.hostname, ".leech", basename),
                    "w",
                    newline="",
                ) as w:
                    writer = csv.writer(w)
                    writer.writerow(HEADER_FIELDS[basename])
                    for record in reader:
                        out = io.StringIO()
                        tmp = csv.writer(out)
                        tmp.writerow(record)
                        bytes = len(out.getvalue())
                        if bytes > CF_MAXTRANSSIZE:
                            print(f"Skipping row: {bytes} > {CF_MAXTRANSSIZE}")
                            assert False
                            continue
                        writer.writerow(record)

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

        COMMITS[self.hostname] += 1


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

        if self.hostname not in COLLECTIONS:
            COLLECTIONS[self.hostname] = 0
            # Skip the first collection from a host in the end report
            return

        collect_num = COLLECTIONS[self.hostname]
        COLLECTIONS[self.hostname] += 1

        work_dir = os.path.join("simulate", self.hostname, ".leech")

        CLD_path = os.path.join(work_dir, "classes.cache")
        CLD_size = os.path.getsize(CLD_path) if os.path.exists(CLD_path) else 0

        VAD_path = os.path.join(work_dir, "variables.cache")
        VAD_size = os.path.getsize(VAD_path) if os.path.exists(VAD_path) else 0

        LSD_path = os.path.join(work_dir, "lastseen.cache")
        LSD_size = os.path.getsize(LSD_path) if os.path.exists(LSD_path) else 0

        SDI_path = os.path.join(work_dir, "software.cache")
        SDI_size = os.path.getsize(SDI_path) if os.path.exists(SDI_path) else 0

        SPD_path = os.path.join(work_dir, "patch.cache")
        SPD_size = os.path.getsize(SPD_path) if os.path.exists(SPD_path) else 0

        ELD_path = os.path.join(work_dir, "execution_log.cache")
        ELD_size = os.path.getsize(ELD_path) if os.path.exists(ELD_path) else 0

        with open(patch_file, "rb") as f:
            buf = f.read()

        LCH_size = len(buf)

        begin = timer()
        zlib_size = len(zlib.compress(buf, 9, -15))
        zlib_time = timer() - begin

        begin = timer()
        lzma_size = len(lzma.compress(buf, check=lzma.CHECK_NONE, preset=9))
        lzma_time = timer() - begin

        begin = timer()
        bz2_size = len(bz2.compress(buf, 9))
        bz2_time = timer() - begin

        begin = timer()
        gzip_size = len(gzip.compress(buf, compresslevel=9))
        gzip_time = timer() - begin

        CFE_size = 0
        tables_include = ["CFR", "CLD", "VAD", "LSD", "SDI", "SPD", "ELD"]
        tables_exclude = ["EXS", "PRD", "CNG", "CND", "MOM", "MOY", "MOH", "PLG"]

        num_commits = COMMITS[self.hostname]
        COMMITS[self.hostname] = 0

        with open(self.dump_file, "rb") as f:
            buf = f.readline()  # Remove first line containing comment
            assert re.match(r"^#(?!\r\n).*\n$", buf.decode())

            buf = f.read(10)  # Read size
            count_bytes = True
            while not buf.decode().startswith("#"):
                assert re.match(r"^[0-9]+ +$", buf.decode())
                size = int(buf.decode())
                buf = f.read(size + 1)  # Read size bytes plus newline

                if size == 3:
                    if buf.decode().strip() in tables_include:
                        count_bytes = True
                    elif buf.decode().strip() in tables_exclude:
                        count_bytes = False

                if count_bytes:
                    CFE_size += size

                buf = f.read(10)  # Read next size

        df = (
            pd.read_csv("simulate/report.csv")
            if os.path.exists("simulate/report.csv")
            else pd.DataFrame(
                columns=[
                    "#",
                    "Timestamp",
                    "Hostname",
                    "CLD",
                    "VAD",
                    "LSD",
                    "SDI",
                    "SPD",
                    "ELD",
                    "lch_size",
                    "zlib_size",
                    "zlib_time",
                    "lzma_size",
                    "lzma_time",
                    "bz2_size",
                    "bz2_time",
                    "gzip_size",
                    "gzip_time",
                    "cfe_size",
                    "commits",
                ]
            )
        )
        df.loc[len(df)] = [
            collect_num,
            self.timestamp.strftime("%s"),
            self.hostname,
            CLD_size,
            VAD_size,
            LSD_size,
            SDI_size,
            SPD_size,
            ELD_size,
            LCH_size,
            zlib_size,
            zlib_time,
            lzma_size,
            lzma_time,
            bz2_size,
            bz2_time,
            gzip_size,
            gzip_time,
            CFE_size,
            num_commits,
        ]
        df.to_csv("simulate/report.csv", index=False)


def main():
    events = []

    subprocess.run(
        "rm -rf simulate/leech.log simulate/**/.leech simulate/report.csv", shell=True
    )

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
                    timestamp = datetime.fromtimestamp(int(filename.split("_")[1]))
                    event = Patch(hostname, hostkey, timestamp, dump_file)
                    events.append(event)

    events = sorted(events)
    for event in events:
        event.work()


if __name__ == "__main__":
    main()
