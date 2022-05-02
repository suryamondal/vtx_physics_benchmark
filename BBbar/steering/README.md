# Steering files
Steering files for the analysis
 - `generate.py` event generation, simulation and reconstruction, outputs mDST
 - `mdst2ntuple.py` decay reconstruction and fitting from mDST, outputs NTuple
 - `mdst2ntuple_D0only.py` same as above, but doesn't reconstruct the `D*`

# Version of basf2 to use
Since `VxdID` representation was changed in the `upgrade` branch, the current
layout must be simulated with a release (or `master`), while VTX must be
simulated with the `upgrade` branch. Also the analysis must be run with the
same version of basf2.

For the current detector layout (VXD=PXD+SVD), use `master` (or
`release-06-00-03`).
```
cd /path/to/local/basf2_master
b2setup
cd /path/to/this/folder
basf2 generate.py -- -o mc_vxd.root [...]
basf2 mdst2ntuple.py -- -i mc_vxd.root -o nt_vxd.root [...]
```

For the upgraded layout (VTX) use a locally-compiled `upgrade` branch.
```
cd /path/to/local/basf2_upgrade
b2setup
cd /path/to/this/folder
source env_setup_vtx5.sh
basf2 generate.py -- --vtx -o mc_vtx.root [...]
basf2 mdst2ntuple.py -- -i mc_vtx.root -o nt_vtx.root [...]
```

`generate.py` should crash if you are using the wrong version of basf2.

`mdst2ntuple.py` should crash if the version of basf2 used is different than
the version that produced the input files.

## Environment variables for VTX
Since there are multiple propsoed VTX geometries, one must set some environment
variables when using the `upgrade` version to select the right one. This is
explained [here](https://confluence.desy.de/display/BI/Full+simulation+effort).

The script `env_setup_vtx5.sh` sets up the enviroment variables for the
5-layers-without-discs layout. It can be used with
```
source env_setup_vtx5.sh
```

Other versions of basf2, including `master` and the relases, will ignore these
enviroment variables.

# Tests
In `tests/` you can find a set of automated tests.

In `tests_manual/` you can find some useful scripts to run steering files on
the fly, just for a quick check.
