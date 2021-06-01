# BBar
This directory includes efficiency and resolution studies of using
`Upsilon(4S) -> B+ B-` events. Charged B mesons decay in at least one `D0` or
`anti-D0` about 90% of the times, so their decay table is not changed.

The decay table of the `D0`s, instead, is set to
```
D0 -> K- pi+          32.43%
D0 -> K- pi+ pi+ pi-  67.57%
```
The branching fractions are normalized, but these two actually make up only
12.18% of the total decay width (source: PDG, of course).

Steering files for generation, simulation, reconstruction (everything that has
to be done using basf2) in `steering/`. Offline (i.e. non-basf2-based) analysis
in `offline/`.
