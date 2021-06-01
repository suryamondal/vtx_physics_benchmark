#!/bin/bash

cd "$(dirname "$0")"
if [ ! -d outputs ]; then mkdir outputs; fi
cd outputs

source ../../env_setup_vtx5.sh
basf2 -n 25 ../../generate.py -- --vtx -o mc_vtx.root
exit $?
