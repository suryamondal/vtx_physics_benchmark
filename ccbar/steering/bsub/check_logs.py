#!/usr/bin/env python3
"""Script to check bsub logs for failures."""
import os
import re
import argparse

RE_SUCCESS = re.compile(r"\nStarted at([\s\S]+?)\nResource usage summary:\n")
RE_LONG = re.compile(r"\n\[INFO\] =+Error Summary=+\n([\s\S]+?)\n\[INFO\] =+\n")


def lsr(path):
    for fn in os.listdir(path):
        fp = os.path.join(path, fn)
        if os.path.islink(fp):
            continue
        if os.path.isdir(fp):
            yield from lsr(fp)
        elif fn.endswith(".log"):
            yield fp


def increment(dictionary, key):
    if key in dictionary:
        dictionary[key] += 1
    else:
        dictionary[key] = 1


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-d", "--directory", default=".",
                        help="The directory to check, default the cwd.")
    parser.add_argument("-l", "--long", action="store_true",
                        help="Output additional information about EACH log.")
    args = parser.parse_args()

    successes, failures, unknowns = {}, {}, {}
    for fp in lsr(args.directory):
        fd = os.path.dirname(fp)
        with open(fp, errors="replace") as ifs:
            content = ifs.read()
        m = RE_SUCCESS.search(content)
        if not m:
            increment(unknowns, fd)
            if args.long: print("UNKNOWN", fp)
        elif "Success" in m[1]:
            increment(successes, fd)
            if args.long: print("SUCCESS", fp)
        else:
            increment(failures, fd)
            if args.long: print("FAILURE", fp)
        if args.long:
            m = RE_LONG.search(content)
            if not m:
                print("No error summary for this file.")
            else:
                print("Error summary")
                print(m[1])
            print()

    dirs = set(successes.keys()) | set(failures.keys()) | set(unknowns.keys())
    for d in dirs:
        s, f, u = successes.get(d, 0), failures.get(d, 0), unknowns.get(d, 0)
        print(f"{s:4d} successful, {f:4d} failed, {u:4d} unknown in {d}")
