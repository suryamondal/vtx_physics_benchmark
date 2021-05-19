# Steering files
Steering files for the analysis
 - `generate.py` event generation and simulation, outputs raw
 - `reconstruct.py` unpacking and tracking of raw, outputs mDST
 - `mdst2ntuple.py` decay reconstruction and fitting from mDST, outputs NTuple

## Generation
The steering file `generate.py` produces only `ccbar -> D*+` (+cc) events and
simulates the detector response.

The only `D*` decay channel simulated is `D*+ -> D0 pi+` (+cc), while for the
`D0` three channels (+cc) are considered
```
D0 -> K- pi+          14.68%
D0 -> K- pi+ pi+ pi-  30.96%
D0 -> K- pi+ pi0      54.18%
```
The branching fractions are normalized, but these three actually make up only
26.58% of the total decay width.

The output rootfile contains simulated hits/digits and MC stuff.

## Reconstruction
The steering file `reconstruct.py` performs the standard reconstruction of the
hits/digits and outputs to mDST format.

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
