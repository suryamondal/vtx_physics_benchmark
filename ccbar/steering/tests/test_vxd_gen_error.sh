#!/bin/bash

cd "$(dirname "$0")"
if [ ! -d outputs ]; then mkdir outputs; fi
cd outputs

basf2 --random-seed 98aa2382dfac6e8552fd64c28fbcea77a6bb0815f7c6625259365bae3a9723d6 \
-n 10 ../../generate.py -- -o mc_vxd_error.root --debug-gen > mc_vxd_error.log
exit $?
