import argparse
import os
import json


def prettier(filenames):
    for filename in filenames:
        print(f"Creating prettified copy: '{filename}' -> '{filename}.new'")
        with open(filename, "r") as r:
            parsed = json.load(r)
        with open(f"{filename}.new", "w") as f:
            json.dump(parsed, f, indent=2)

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


def main():
    parser = argparse.ArgumentParser(description="Prettify JSON files")
    parser.add_argument(
        "filenames", metavar="FILENAME", nargs="+", help="name of file to prettify"
    )
    args = parser.parse_args()
    prettier(args.filenames)


if __name__ == "__main__":
    main()
