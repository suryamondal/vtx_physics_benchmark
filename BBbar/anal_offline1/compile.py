#!/usr/bin/env python3
"""Compiler script for the offline analysis."""
import os
import re
import subprocess
import glob

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

CPP_COMPILER = "g++"
CPP_FLAGS = ["-g", "-Wall", "-pedantic", "-Wno-literal-suffix"]
ROOT_FLAGS = subprocess.run(["root-config", "--cflags", "--glibs"], check=True,
                            capture_output=True, encoding="utf8").stdout.split()

RE_INCLUDE = re.compile(r'#include "(\S+?)"')

WHITE = "\x1b[1m"
NORM = "\x1b[0m"


def run(*args):
    print(" ".join(args))
    return subprocess.run(args, check=True)


def is_newer(file_a, file_b):
    """Returns True if file_a is newer than file_b."""
    try:
        return os.path.getmtime(file_a) > os.path.getmtime(file_b)
    except Exception:
        return False


def get_local_includes(source_file):
    with open(source_file) as ifs:
        for ln in ifs:
            m = RE_INCLUDE.match(ln.strip())
            if m is None: continue
            if os.path.isfile(m[1]):
                yield m[1]


def is_up_to_date(source_file, out_file):
    if not is_newer(out_file, source_file): return False
    for include_file in get_local_includes(source_file):
        if not is_newer(out_file, include_file): return False
    return True


def change_ext(pathspec, newext):
    base, ext = os.path.splitext(pathspec)
    return base + newext


def log(msg, inp=None, out=None):
    parts = [f"[{msg:^10s}]"]
    if inp is not None: parts.append(inp)
    if out is not None: parts += ["->", out]
    print(f"{WHITE}{' '.join(parts)}{NORM}")


if __name__ == "__main__":
    os.chdir(SCRIPT_DIR)
    if not os.path.isdir("build"):
        os.mkdir("build")

    for source_file in map(os.path.relpath, glob.glob("*.cc")):
        out_file = change_ext(os.path.join("build", source_file), ".o")
        if is_up_to_date(source_file, out_file):
            log("UP TO DATE", source_file, out_file)
        else:
            log("COMPILING", source_file, out_file)
            cmd = [CPP_COMPILER, "-c", "-o", out_file, source_file]
            cmd += CPP_FLAGS
            cmd += ROOT_FLAGS
            run(*cmd)

    out_files = glob.glob("build/*.o")
    if all(is_newer("ana", out_file) for out_file in out_files):
        log("UP TO DATE", "build/*.o", "ana")
    else:
        log("LINKING", "build/*.o", "ana")
        cmd = [CPP_COMPILER, "-o", "ana"] + out_files
        cmd += CPP_FLAGS
        cmd += ROOT_FLAGS
        run(*cmd)

    log("DONE")
