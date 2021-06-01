#!/bin/bash

cd "$(dirname "$0")"
if [ ! -d outputs ]; then mkdir outputs; fi
cd outputs

echo "Running generate.py ..."
basf2 --random-seed 42 -n 25 ../../generate.py -- \
-o mc_vxd_printMCparticles.root \
--print-mc-particles > mc_vxd_printMCparticles.log
exit $?
