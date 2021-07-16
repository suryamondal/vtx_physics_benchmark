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

Analysis (no bsub jobs used, should not be necessary for now):
```
basf2 ../mdst2ntuple.py -- -o nt.root -i outputs/SUBDIR/SUBDIR/*.root
```
or, to save some time typing
```
./launch_ana.sh outputs/mc_ETC
# Input files: outputs/mc_ETC/*/*.root
# Output file: outputs/mc_ETC_ntuple.root
```
