#!/bin/bash

cd "$(dirname "$0")"
if [ ! -d outputs ]; then mkdir outputs; fi
cd outputs

basf2 -n 25 ../../generate.py -- -o mc_vxd.root
exit $?
