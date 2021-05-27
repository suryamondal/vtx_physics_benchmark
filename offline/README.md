# Offline analysis
Here are the ROOT macros and Python scripts to analyse the NTuples.
 - `mc_mass_fit.cc` root macro for fitting the `D*+` MC mass (this is mostly
   a debug tool).
    - Usage: `root -b -q path/to/ntuple.root mc_mass_fit.cc`
    - Output file: `mc_mass_fit.pdf` (rember to rename it!)
