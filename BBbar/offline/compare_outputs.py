#!/usr/bin/env python3
import sys
import re
import os
from collections import namedtuple

RE_NTUPLE = re.compile(r"Info in <TCanvas::Print>: pdf file (.*?) has been created")
RE_CHANNEL = re.compile(r"Processing (\w+)\.\.\.")
RE_TR = re.compile(r" *([^\s\|]+) *\|(?: *[^ \|]+ *\|){3} *([^\s\|]+)")
RE_SIGMA68 = re.compile(r"(\w+)Cuts_sig_(\w+) sigma68 = ([0-9\.-]+) \+- ([0-9\.-]+), center = ([0-9\.-]+) \+- ([0-9\.-]+)")
RE_COUNT = re.compile(r"_\d+$")


Variable = namedtuple("Variable", "channel particle variable operation")


def make_variable(s):
    s = s.split("_", 3)
    if len(s) == 4:
        return Variable(*s)
    if len(s) == 3:  # Actually unused
        return Variable(s[0], "", s[1], s[2])
    if len(s) == 2:
        return Variable(s[0], "", s[1], "")
    return Variable("", "", s[0], "")


def read_log_file(file_path):
    ntuples = {"VTX": [], "VXD": []}
    current_ntuple = None
    with open(file_path) as ifs:
        for ln in ifs:
            ln = ln.strip()
            m = RE_NTUPLE.match(ln)
            if m:
                current_ntuple = "VXD" if "vxd" in m[1].lower() else "VTX"
            elif current_ntuple in ntuples:
                ntuples[current_ntuple].append(ln)
    return ntuples


def read_log_lines(lines):
    current_channel = None
    ntuple = {}
    for ln in lines:
        m = RE_CHANNEL.match(ln)
        if m or ln == "Total":
            current_channel = m[1] if m else "Total"
            continue
        m = RE_TR.match(ln)
        if m:
            if m[1] in ("Signal", "Background", "MC", "Efficiency"):
                ntuple[make_variable(f"{current_channel}_{m[1]}")] = m[2]
            continue
        m = RE_SIGMA68.match(ln)
        if m:
            channel, variable = m[1], RE_COUNT.sub("", m[2])
            ntuple[make_variable(f"{channel}_{variable}_σ68")] = f"{m[3]} ± {m[4]}"
            ntuple[make_variable(f"{channel}_{variable}_σ68center")] = f"{m[5]} ± {m[6]}"
            continue
    return ntuple


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} PATH_TO_LOGFILE")
        sys.exit(1)

    outfile = os.path.splitext(sys.argv[1])[0] + ".csv"
    if outfile.lower() == sys.argv[1].lower():
        outfile += ".csv"

    ntuples = read_log_file(sys.argv[1])
    ntuples = {k: read_log_lines(v) for k, v in ntuples.items()}
    with open(outfile, "w") as ofs:
        variables = list(set(k for x in ntuples.values() for k in x.keys()))
        # Sort by channel, then operation, then particle, then variable
        variables.sort(key=lambda x: (x[0], x[3], x[2], x[1]))

        print("Channel,Particle,Variable,Operation," + ",".join(ntuples.keys()), file=ofs)
        for v in variables:
            print(",".join(v) + "," + ",".join(x.get(v, "") for x in ntuples.values()), file=ofs)
