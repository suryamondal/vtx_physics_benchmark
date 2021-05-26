# Tests of the steering files
The scripts in this folder can be used to check that the steering files behave
correctly with the different versions of basf2.

This sequence of steps should allow to test all possible mistakes

|Script           |Version of basf2|Expected result|Output file  |
|:---------------:|:--------------:|:-------------:|:-----------:|
|`test_vxd_gen.sh`|upgrade branch  |Failure        |N/A          |
|`test_vxd_gen.sh`|release-05      |Success        |`mc_vxd.root`|
|`test_vtx_gen.sh`|release-05      |Failure        |N/A          |
|`test_vtx_gen.sh`|upgrade branch  |Success        |`mc_vtx.root`|
|`test_vxd_ana.sh`|upgrade branch  |Failure        |N/A          |
|`test_vxd_ana.sh`|release-05      |Success        |`nt_vxd.root`|
|`test_vtx_ana.sh`|release-05      |Failure        |N/A          |
|`test_vtx_ana.sh`|upgrade branch  |Success        |`nt_vtx.root`|
|`test_mix_ana.sh`|upgrade branch  |Failure        |N/A          |
|`test_mix_ana.sh`|release-05      |Failure        |N/A          |

Explanation
 - `test_vxd_gen.sh` attempts to simulate the current layout, hence it should
   work with release but not with upgrade.
 - `test_vtx_gen.sh` attempts to simulate the layout with VTX, hence it should
   work with upgrade but not with release.
 - `test_vxd_ana.sh` attempts to analyse `mc_vxd.root` (the simulation with
   the current layout), hence it should work with release but not with upgrade.
 - `test_vtx_ana.sh` attempts to analyse `mc_vtx.root` (the simulation with
   the VTX layout), hence it should work with upgrade but not with release.
 - `test_mix_ana.sh` attempts to analyse both `mc_vxd.root` and `mc_vtx.root`
   at the same time (like they were part of the same batch), hence it should
   fail with any version of basf2 because the files are incompatible.

## Other tests
The script `test_vxd_gen_error.sh` is designed specifically to reproduce the
following error
```
[ERROR] In G4Decay::DecayIt , DECAY101: For D0 decay probability exist but decay table is not defined - the particle will be killed;
    isExtDecayer: 0; isPreAssigned: 0  { module: FullSim }
```
this is done by setting an appropriate RNG seed.

The script `text_vxd_gen_error_print_mc_particles.sh` does exactly the same
thing, but instead of enabling debug messages it prints MC particles in the
logfile.
