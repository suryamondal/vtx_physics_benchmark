#!/bin/bash

cd "$(dirname "$0")"
if [ ! -d outputs ]; then mkdir outputs; fi
cd outputs

source ../../env_setup_vtx5.sh
basf2 ../../mdst2ntuple.py -- -i mc_vtx.root -o nt_vtx.root
exit $?
