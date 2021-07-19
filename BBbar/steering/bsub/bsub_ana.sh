#!/bin/bash

########################################################################
# Launches the analysis on all mc files in a output directory.
#
# Syntax:
#     ./launch_ana.sh <OUTDIR>/mc_vxd [...]
#     ./launch_ana.sh <OUTDIR>/mc_vtx [...]
#
# These are equivalent to
#     bsub -q s -oo <OUTDIR>/mc_vxd.log \
#       basf2 ../mdst2ntuple.py -- -o <OUTDIR>/mc_vxd.root \
#       -i '<OUTDIR>/mc_vxd/*/*.root' [...]
#     bsub -q s -oo <OUTDIR>/mc_vtx.log \
#       basf2 ../mdst2ntuple.py -- -o <OUTDIR>/mc_vtx.root \
#       -i '<OUTDIR>/mc_vtx/*/*.root' [...]
########################################################################

cd "$(dirname "$0")"

if [ $# -lt 1 ]; then
  echo "This script requires >=1 argument."
  echo "  $0 <OUTDIR/SUBDIR>"
  exit 1
fi

outdir="${1%/}"  # Remove trailing "/"
if [ ! -d "$outdir" ]; then
  echo "Not a directory: $outdir"
  exit 1
fi

outfile="${outdir}_ntuple.root"
logfile="${outdir}_ntuple.log"
inglob="$outdir/*/*.root"
echo "Input files: $inglob"
echo "Output file: $outfile"
echo "Log file: $outfile"
echo "Submitting analysis..."
bsub -q s -oo "$logfile" basf2 ../mdst2ntuple.py -- -o "$outfile" -i "$inglob" "${@:2}"
