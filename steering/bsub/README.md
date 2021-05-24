# Batch jobs submission
Generation jobs submission with bsub (typical usage):
```
./submit_gen.py -n N_TOTAL_EVENTS [--vtx] [--bkg]
```

Logs check:
```
./check_logs.py [-d outputs]
```

Analysis (no bsub jobs used, should not be necessary for now):
```
basf2 ../mdst2ntuple.py -- -o nt.root -i outputs/SUBDIR/SUBDIR/*.root
```
