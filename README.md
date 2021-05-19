# VTX Physics Benchmark
Repository for studies of the proposed upgraded VTX. Includes efficiency and
resolution studies of using the following events
```
D*+ --> D0 pi+
        |
        +--> K- pi+
        +--> K- pi+ pi+ pi-
```

The steering files in `steering/` have been copied from the repo
[charm_lifetimes](https://stash.desy.de/projects/B2CHARM/repos/charm_lifetimes/browse)
(many thanks to G. Casarosa and A. di Canto for writing them!). They take care
of event generation, simulation, reconstruction, decay reconstruction and
fitting. Their final outputs are NTuples.
