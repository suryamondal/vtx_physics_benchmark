#!/bin/bash
# Warning: this test is expexted to fail under any circumstances!

cd "$(dirname "$0")"
if [ ! -d outputs ]; then mkdir outputs; fi
cd outputs

source ../../env_setup_vtx5.sh  # Won't change anything if using release/master
basf2 ../../mdst2ntuple.py -- -i mc_vtx.root mc_vxd.root -o nt_vtx.root
exit $?
