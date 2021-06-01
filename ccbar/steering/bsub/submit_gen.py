#!/usr/bin/env python3
"""Script for submitting generation jobs with bsub on KEKCC."""
import os
import sys
import argparse
import subprocess

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DEFAULT_OUT_DIR = os.path.join(SCRIPT_DIR, "outputs")
STEER_DIR = os.path.dirname(SCRIPT_DIR)
STEER_FILE = os.path.join(STEER_DIR, "generate.py")


def check_basf2_version(use_vtx):
    try:
        basf2_path = os.environ["BELLE2_LOCAL_DIR"]
    except KeyError:
        try:
            basf2_path = os.environ["BELLE2_RELEASE_DIR"]
        except KeyError:
            print("ERROR Did you forget to run b2setup?")
            sys.exit(1)
    has_vtx = os.path.isfile(os.path.join(basf2_path, "lib/Linux_x86_64/opt/vtx/__init__.py"))
    if not use_vtx == has_vtx:
        print("ERROR You are using the wrong verion of basf2.")
        print(f"      use_vtx = {use_vtx}")
        print(f"      has_vtx = {has_vtx}")
        sys.exit(1)
    if use_vtx:
        try:
            _ = os.environ["BELLE2_VTX_UPGRADE_GT"]
            _ = os.environ["BELLE2_VTX_BACKGROUND_DIR"]
        except KeyError:
            print("ERROR Did you forget to set BELLE2_VTX_* variables?")
            sys.exit(1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-n", type=int, help="Number of events to generate")
    parser.add_argument("--exp", type=int, default=0, help="Experiment, default 0 (nominal geometry/luminosity)")
    parser.add_argument("--run", type=int, default=0, help="Run, default 0")
    parser.add_argument("--bkg", action="store_true", help="Enable background")
    parser.add_argument("--vtx", action="store_true", help="Use new VTX instead of PXD+SVD")
    parser.add_argument("--first-event", type=int, default=1, help="Number of the first event, default 1")
    parser.add_argument("-b", "--batch-size", type=int, default=1000, help="Number of events per process, default 1000")
    parser.add_argument("--dry-run", action="store_true", help="Don't run anything, show what would be run")
    parser.add_argument("-o", "--output", default=DEFAULT_OUT_DIR, help="Output directory, default outputs")
    args = parser.parse_args()
    check_basf2_version(args.vtx)

    sub_dir = "mc_vtx" if args.vtx else "mc_vxd"
    run_dir = f"e{args.exp:04d}r{args.run:05d}"
    out_dir = os.path.join(args.output, sub_dir, run_dir)
    print("Output directory:", out_dir)
    if os.path.isdir(out_dir):
        print("WARNING Output directory exists, you might overwrite files")
        print("        You might produce multiple events with the same number")
        print("        YOU must check that none of the above happens!")
    elif not args.dry_run:
        os.makedirs(out_dir)

    last_event = args.first_event + args.n - 1
    n_jobs = (args.n + args.batch_size - 1) // args.batch_size  # Ceiling integer division
    print(f"Producing {args.n} events (from {args.first_event} to {last_event})")
    print(f"Running {n_jobs} jobs")

    print()
    if not args.dry_run:
        os.chdir(out_dir)
        print("Current directory:", os.getcwd())
    else:
        print("Current directory:", os.path.abspath(out_dir))

    for i in range(n_jobs):
        first_event = args.first_event + args.batch_size * i
        skip_events = first_event - 1
        last_event = min(first_event + args.batch_size, args.first_event + args.n) - 1
        n_events = last_event - first_event + 1
        out_file_name = f"{sub_dir}_{run_dir}_event{first_event:06d}_to{last_event:06d}.root"
        log_file_name = f"{sub_dir}_{run_dir}_event{first_event:06d}_to{last_event:06d}.log"
        print()
        print(f"Job #{i}: {n_events} events ({first_event} to {last_event})")
        basf_cmd = ["basf2", STEER_FILE, "--skip-events", str(skip_events),
                    "-n", str(last_event), "--", "--exp", str(args.exp),
                    "--run", str(args.run), "-o", out_file_name]
        if args.bkg: basf_cmd += ["--bkg"]
        if args.vtx: basf_cmd += ["--vtx"]
        bsub_cmd = ["bsub", "-q", "s", "-oo", log_file_name]
        bsub_cmd += basf_cmd
        print("Command:", " ".join(bsub_cmd))
        if not args.dry_run:
            subprocess.check_call(bsub_cmd)
