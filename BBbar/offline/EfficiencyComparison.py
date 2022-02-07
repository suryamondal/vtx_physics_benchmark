#!/usr/bin/env python3
"""Convenience wrapper for EfficiencyComparison.C"""
import os
import sys
import subprocess

SCRIPT_PATH = os.path.abspath(__file__)
SCRIPT_DIR = os.path.dirname(SCRIPT_PATH)
SCRIPT_NAME = os.path.basename(SCRIPT_PATH)

if __name__ == "__main__":
    args = sys.argv[1:] if len(sys.argv) > 1 else []
    if len(args) % 2 != 0 or len(args) == 0:
        print(__doc__)
        print(f"Usage: ./{SCRIPT_NAME} TITLE1 FILE1.root TITLE2 FILE2.root")
        print("Where FILEn.root are the *_efficiency.root generated by ana.")
    else:
        os.chdir(SCRIPT_DIR)
        for x, y in zip(args[::2], args[1::2]):
            print(f"{x} -> {y}")
        output = os.path.join(os.path.dirname(os.path.abspath(args[1])), "effcomp.pdf")
        print(f"Output = {output}")
        titles = ",".join(f'"{x}"' for x in args[::2])
        files = ",".join(f'"{x}"' for x in args[1::2])
        macro = f'EfficiencyComparison.C("{output}",{{{files}}},{{{titles}}})'
        cl = ['root', '-l', '-b', '-q', macro]
        sys.exit(subprocess.run(cl).returncode)