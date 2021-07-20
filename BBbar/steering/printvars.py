#!/usr/bin/env python3
"""Utility to print mdst2ntuple.py's output variables."""
import os
import re
import subprocess

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
STEERING_FILE = os.path.join(SCRIPT_DIR, "mdst2ntuple.py")

RE_VAR = re.compile(r"((?:[A-Za-z0-9]+_)?)([A-Za-z0-9_]+)")


def dict_append(d, key, value):
    if key in d: d[key].add(value)
    else: d[key] = {value}


if __name__ == "__main__":
    # Run steering with --printVars
    cp = subprocess.run(
        ["basf2", STEERING_FILE, "--", "-i", "x", "-o", "y", "--printVars"],
        stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, timeout=30,
        check=True, encoding='utf8', errors='ignore')
    vars = {}
    particles = set()
    for ln in cp.stdout.splitlines():
        m = RE_VAR.fullmatch(ln.strip())
        if m is None: continue
        part = (m[1] or "EVENT").strip("_")
        dict_append(vars, m[2], part)
        particles.add(part)
    particles = list(particles)
    particles.sort(key=str.lower)

    print(f'{"Variable":^40s}', " ".join(f'{x:^8s}' for x in particles))
    for var in sorted(list(vars.keys()), key=str.lower):
        parts = vars[var]
        print(f'{var:^40s}', " ".join(
            f'{x:^8s}' if x in parts else f'{"":^8s}' for x in particles))
