#!/bin/bash

cd "$(dirname "$0")"
if [ ! -d outputs ]; then mkdir outputs; fi
cd outputs

basf2 ../../mdst2ntuple.py -- -i mc_vxd.root -o nt_vxd.root
exit $?
