#!/bin/bash

cd "$(dirname "$0")"
if [ ! -d outputs ]; then mkdir outputs; fi
cd outputs

basf2 -n 25 ../generate.py -- -o test_mc.root || exit 1
basf2 ../reconstruct.py -i test_mc.root -- -o test_mdst.root || exit 1
basf2 ../mdst2ntuple.py -- -i test_mdst.root -o test_ntuple.root --isMC || exit 1
