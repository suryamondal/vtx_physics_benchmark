#!/usr/bin/env python3
# -*- coding: utf-8 -*-

############################################
# Author: The Belle II Colaboration
# Contributor: Giulia Casarosa (Jun 2020)
# Software Version: release-04-02-02
#
# this script: 
# - generates & simulates signal samples
#   for the D0 lifetime analysis
# - can be launched with scripts/submit_generation.py
############################################

from basf2 import *
import generators as ge
import simulation as simu
import modularAnalysis as ma
from basf2 import conditions as b2c

import sys
import os
import glob

if len(sys.argv) != 7:
    sys.exit('Must provide enough arguments: [exp number] [run number] [bkg] [PXD data reduction] [T0 jitter] output file name]'
             )


expNumber =int(sys.argv[1])
runNumber =int(sys.argv[2])
bkg = str(sys.argv[3])
pxdDR = str(sys.argv[4])
jitter = str(sys.argv[5])
outputName = sys.argv[6]

bkgFiles = None

if (bkg == 'bkg'):
    bkgFiles = glob.glob('/group/belle2/BGFile/OfficialBKG/early_phase3/5a1f0a9f2ad84a/overlay/phase31/BGx1/set0/*root')

pxd = False
if (pxdDR == 'pxdDR'):
    pxd = True

jit = False
if(jitter == 'jitter'):
    jit = True

b2c.prepend_globaltag("mc_production_MC13a_proc11")

# create path 
main = create_path()

# setup event
evtInfo = register_module('EventInfoSetter')
evtInfo.param('expList', [expNumber])
evtInfo.param('runList', [runNumber])
main.add_module(evtInfo)

# progress
main.add_module('Progress')

# generate signal
ge.add_inclusive_continuum_generator(main, "ccbar", 'D*+',userdecfile='dec3modes.dec')

# simulate
simu.add_simulation(main, bkgfiles = bkgFiles, usePXDDataReduction=pxd, simulateT0jitter=jit)

# write output file
main.add_module("RootOutput",outputFileName=outputName)

# print path
print_path(main)

# process the path
process(main)

# print out the summary
print(statistics)
