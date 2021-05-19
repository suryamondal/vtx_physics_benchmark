#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Reconstructs the signal samples for the D0 lifetime analysis."""

##############################################
# Author: The Belle II Colaboration
# Contributors: Giulia Casarosa (Jun 2020),
#               Ludovico Massaccesi (May 2021)
# Software Version: release-05-02-04
##############################################

import argparse
import basf2 as b2
import reconstruction as reco
import modularAnalysis as ma
from basf2 import conditions as b2c

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("-o", "--output", default="mdst.root", help="Output file name")
parser.add_argument("--misalign-gt", default=None, help="Misalignment GT, optional")
parser.add_argument("--misalign-beamspot-gt", default=None,
                    help="Beam spot misalignment GT, optional")
args = parser.parse_args()
b2.B2INFO(f"Steering files args = {args}")

# turn on MISALIGNEMENT
if args.misalign_gt is not None:
    b2c.prepend_globaltag(args.misalign_gt)
if args.misalign_beamspot_gt is not None:
    b2c.prepend_globaltag(args.misalign_beamspot_gt)

# create path
main = b2.create_path()

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
ma.outputMdst(args.output, main)

# print path
b2.print_path(main)

# process the path
b2.process(main)

# print out the summary
print(b2.statistics)
