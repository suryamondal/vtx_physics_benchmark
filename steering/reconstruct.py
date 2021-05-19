#!/usr/bin/env python3
# -*- coding: utf-8 -*-

############################################
# Author: The Belle II Colaboration
# Contributor: Giulia Casarosa (Jun 2020)
# Software Version: release-04-02-02
#
# this script: 
# - reconstruct signal samples
#   for the D0 lifetime analysis
# - can be launched with scripts/submit_reconstruction.py
############################################

from basf2 import *
import reconstruction as reco
import modularAnalysis as ma
from basf2 import conditions as b2c

import sys
import os
import glob

print('***')
print('*** this is the reconstruction script used:')
with open(sys.argv[0], 'r') as fin:
    print(fin.read(), end="")
print('*** end of reconstruction script')
print('***')

outputName = sys.argv[1]

# turn on MISALIGNEMENT
if len(sys.argv) > 2:
    misalignmentGT = sys.argv[2]
    b2c.prepend_globaltag(misalignmentGT)
if len(sys.argv) > 3:
    beamSpotMisalignmentGT = sys.argv[3]
    b2c.prepend_globaltag(beamSpotMisalignmentGT)


# create path 
main = create_path()

# read input file
main.add_module("RootInput")

# progress
main.add_module('ProgressBar')

# geometry
main.add_module("Gearbox")
main.add_module("Geometry")

# reconstruct
reco.add_reconstruction(main)

# write MDSTs
ma.outputMdst(outputName, main)

# print path 
print_path(main)

# process the path
process(main)

# print out the summary
print(statistics)
