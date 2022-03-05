#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate MC signal samples for D0 lifetime analysis."""

############################################################
# Author: The Belle II Colaboration
# Contributors: Ludovico Massaccesi (June 2021)
# Software Version: release-06-00-03 / upgrade / master
#
# Must use release-06-00-03 or master without --vtx
# Must use upgrade branch with --vtx
# The script will terminate if used with the wrong version.
############################################################

import argparse
import os
import sys
import basf2 as b2
import generators as ge
import simulation as simu
import reconstruction as reco
import mdst
try:
    import vtx
    HAS_VTX = True
except ImportError:
    import background
    HAS_VTX = False

# Can't use __file__, the interpreter doesn't set it
SCRIPT_DIR = os.path.dirname(os.path.abspath(sys.argv[0]))
DECAY_FILE = os.path.join(SCRIPT_DIR, "decays.dec")

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--evt", type=int, default=10,
                    help="Number of Event to be generated")
parser.add_argument("--exp", type=int, default=0,
                    help="Experiment, default 0 (nominal geometry/luminosity)")
parser.add_argument("--run", type=int, default=0, help="Run, default 0")
parser.add_argument("--bkg", action="store_true", help="Enable background")
parser.add_argument("--vtx", action="store_true",
                    help="Use new VTX instead of PXD+SVD")
parser.add_argument("--seed", default='',
                    help="Random Seed")
parser.add_argument("-o", "--output", default="mc.root",
                    help="Output file, default mc.root")
parser.add_argument("--debug-gen", action="store_true",
                    help="Debug the generator modules")
parser.add_argument("--print-mc-particles", action="store_true",
                    help="Prints the MC particles for debugging")
args = parser.parse_args()
b2.B2INFO(f"Steering file arguments = {args}")

# Check that we are using the correct version of basf2
if args.vtx != HAS_VTX:
    kwa_ver = {'useVTX': args.vtx, 'hasVTX': HAS_VTX}
    for var_name in ["RELEASE", "RELEASE_DIR", "LOCAL_DIR"]:
        var_name = f"BELLE2_{var_name}"
        kwa_ver[var_name] = os.environ.get(var_name, "")
    b2.B2FATAL("Incorrect version of basf2.", **kwa_ver)

# VTX-specific GT must replace master/release GT
if args.vtx:
    b2.conditions.disable_globaltag_replay()
    b2.conditions.prepend_globaltag(vtx.get_upgrade_globaltag())

if args.bkg:
    if args.vtx:
        bkg_files = vtx.get_upgrade_background_files()
    else:
        bkg_files = background.get_background_files()
    if not bkg_files:
        b2.B2FATAL("Using --bkg but no background file was found.")
else:
    bkg_files = None

if args.debug_gen:
    b2.logging.package('generators').log_level = b2.LogLevel.DEBUG
    b2.logging.package('generators').debug_level = 199
    b2.logging.package('simulation').log_level = b2.LogLevel.DEBUG
    b2.logging.package('simulation').debug_level = 199

# set random seed
if args.seed:
    b2.set_random_seed(args.seed);
# b2.set_random_seed('94887e3828c78b3bd0b761678bd255317f110e183c2ed59ebdcd027e7610b9d6');

# create path
main = b2.create_path()

# setup event + progress
main.add_module('EventInfoSetter', expList=[args.exp], runList=[args.run], evtNumList=[args.evt])
main.add_module('ProgressBar')

# generate signal
ge.add_evtgen_generator(main, finalstate='signal', signaldecfile=DECAY_FILE)
if args.print_mc_particles:
    main.add_module("PrintMCParticles").set_name("PrintMCParticles_generated")

# simulate
vtx_kwa = {'useVTX': True} if args.vtx else {}
simu.add_simulation(main, bkgfiles=bkg_files, **vtx_kwa)
# l1t.add_tsim(...) --> TRG simulation now included in add_simulation
if args.print_mc_particles and args.debug_gen:
    main.add_module("PrintMCParticles").set_name("PrintMCParticles_simulated")

# reconstruct
vtx_kwa = {'useVTX': True,
           "vtx_bg_cut": None,
           "vtx_ckf_mode": "VXDTF2_after",
           # "vtx_ckf_mode": "VXDTF2_before_with_second_ckf",
           "use_vtx_to_cdc_ckf": False,
           "use_ckf_based_cdc_vtx_merger": False} if args.vtx else {}
reco.add_reconstruction(main, **vtx_kwa)

# write MDSTs
mdst.add_mdst_output(main, mc=True, filename=args.output)

# print path + process + statistics
if args.debug_gen:
    b2.print_path(main)
b2.process(main)
print(b2.statistics)
