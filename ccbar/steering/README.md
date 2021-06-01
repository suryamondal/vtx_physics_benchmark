# Steering files
Steering files for the analysis
 - `generate.py` event generation, simulation and reconstruction, outputs mDST
 - `mdst2ntuple.py` decay reconstruction and fitting from mDST, outputs NTuple

## Generation, simulation and reconstruction
The steering file `generate.py` produces only `ccbar -> D*+` (+cc) events and
simulates the detector response.

The only `D*` decay channel simulated is `D*+ -> D0 pi+` (+cc), while for the
`D0` two channels (+cc) are considered
```
D0 -> K- pi+          32.43%
D0 -> K- pi+ pi+ pi-  67.57%
```
The branching fractions are normalized, but these two actually make up only
12.18% of the total decay width (source: PDG, of course).

Simulation of the detector and standard reconstruction is then performed. The
output is in mDST format.

## Analysis
The steering file `mdst2ntuple.py` reconstructs the decay chain
 - Pions and kaons from IP (within a few cm) are loaded; the "soft" pion of the
   `D*` has looser tracking requirements than the pions and kaons of the `D0`.
 - `D0 -> K pi` and `D0 -> K 3pi` decays are reconstructed; the `D0`
   is required to have mass between 1.6 GeV and 2.1 GeV.
 - The `D0` candidates are merged, `K pi` channel has precedence over `K 3pi`.
 - `D* -> D0 pi` is reconstructed ("soft" pion used this time); similar
   requirements are used for the mass of the `D*`, and also a cut on its
   momentum is used.
 - The decay tree is fitted, using IP constraint. Pre-fit `D*` mass estimates
   are saved.
 - A lot  of variables are stored in two NTuples (one for the the `K pi` decay
   channel, the other for the `K 3pi` channel).

The output is a rootfile with two NTuples of variables
 - `DstD0PiKPiRS` for the `K pi` channel
 - `DstD0PiK3PiRS` for the `K 3pi` channel

# Version of basf2 to use
Since `VxdID` representation was changed in the `upgrade` branch, the current
layout must be simulated with a release (or `master`), while VTX must be
simulated with the `upgrade` branch. Also the analysis must be run with the
same version of basf2.

For the current detector layout (VXD=PXD+SVD), use `release-05-02-06` or
`master` (will be `release-06` soon).
```
b2setup release-05-02-06
basf2 generate.py -- -o mc_vxd.root [...]
basf2 mdst2ntuple.py -- -i mc_vxd.root -o nt_vxd.root [...]
```

For the upgraded layout (VTX) use a locally-compiled `upgrade` branch.
```
cd /path/to/local/basf2
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
