import argparse
import os
import csv


def sanitize(filenames):
    for filename in filenames:
        print(f"Creating sanatized copy: '{filename}' -> '{filename}.new'")
        with open(filename, "r", newline="") as r:
            reader = csv.reader(r)
            with open(f"{filename}.new", "w", newline="") as w:
                writer = csv.writer(w)
                writer.writerows(reader)

    yes = ["yes", "y"]
    answer = input(f"Inspect sources and proceed with [{'/'.join(yes)}]: ")
    if answer.lower() in yes:
        print("Proceeding ...")
        for filename in filenames:
            print(f"Removing file: '{filename}'")
            os.remove(f"{filename}")
            print(f"Renaming file: '{filename}.new' -> '{filename}'")
            os.rename(f"{filename}.new", filename)
    else:
        print("Aborting ...")
        for filename in filenames:
            print(f"Removing '{filename}.new'")
            os.remove(f"{filename}.new")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Sanitize CSV files")
    parser.add_argument("filename", nargs="+", help="name of file to sanitize")
    args = parser.parse_args()
    sanitize(args.filename)
