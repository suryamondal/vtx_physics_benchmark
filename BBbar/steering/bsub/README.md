# Batch jobs submission
Generation jobs submission with bsub (typical usage):
```
./submit_gen.py -n N_TOTAL_EVENTS [--vtx] [--bkg]
```
Random seed is fixed and depends on the exp/run/events generated.

Logs check:
```
./check_logs.py [-d outputs]
```

Analysis:
```
basf2 ../mdst2ntuple.py -- -o nt.root -i outputs/SUBDIR/SUBDIR/*.root
```
or, to save some time typing
```
./launch_ana.sh outputs/mc_ETC [... extra steering file args ...]
./bsub_ana.sh outputs/mc_ETC  # Uses bsub
./test_ana.sh outputs/mc_ETC  # Stops at 20 events
# Input files: outputs/mc_ETC/*/*.root
# Output file: outputs/mc_ETC_ntuple.root
```
