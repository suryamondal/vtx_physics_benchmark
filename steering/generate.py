#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate MC signal samples for D0 lifetime analysis."""

##############################################
# Author: The Belle II Colaboration
# Contributors: Giulia Casarosa (Jun 2020)
#               Ludovico Massaccesi (May 2021)
# Software Version: release-05-02-04
##############################################

import argparse
import os
import sys
import glob
import basf2 as b2
import generators as ge
import simulation as simu
from basf2 import conditions as b2c

# Can't use __file__, the interpreter doesn't set it
SCRIPT_DIR = os.path.dirname(os.path.abspath(sys.argv[0]))
DECAY_FILE = os.path.join(SCRIPT_DIR, "dec3modes.dec")

BKG_FILES = '/group/belle2/BGFile/OfficialBKG/early_phase3/5a1f0a9f2ad84a/overlay/phase31/BGx1/set0/*root'

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--exp", type=int, default=1003, help="Experiment, default 1003")
parser.add_argument("--run", type=int, default=5, help="Run, default 5")
parser.add_argument("--bkg", action="store_true", help="Enable background")
parser.add_argument("--nopxddr", dest="pxddr", action="store_false", help="Disable PXD data reduction")
parser.add_argument("--t0jit", action="store_true", help="Enable T0 jitter simulation")
parser.add_argument("-o", "--output", default="mc.root", help="Output file, default mc.root")
args = parser.parse_args()
b2.B2INFO(f"Steering file arguments = {args}")

bkg_files = None
if args.bkg:
    bkg_files = glob.glob(BKG_FILES)

b2c.prepend_globaltag("mc_production_MC13a_proc11")

# create path
main = b2.create_path()

# setup event
main.add_module('EventInfoSetter', expList=[args.exp], runList=[args.run])

# progress
main.add_module('ProgressBar')

# generate signal
ge.add_inclusive_continuum_generator(main, "ccbar", 'D*+', userdecfile=DECAY_FILE)

# simulate
simu.add_simulation(main, bkgfiles=bkg_files, usePXDDataReduction=args.pxddr,
                    simulateT0jitter=args.t0jit)

# write output file
main.add_module("RootOutput", outputFileName=args.output)

# print path
b2.print_path(main)

# process the path
b2.process(main)

# print out the summary
print(b2.statistics)
