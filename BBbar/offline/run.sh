#!/bin/bash
if [ ! -d "$1" ]; then
  echo "Usage ./run.sh /path/to/dir/with/ntuples"
  exit 1
fi

dir="$(realpath "$1")"
dir="${dir%/}"
cd "$(dirname "$0")"

for file in "$dir/mc_vtx_ntuple.root" "$dir/mc_vxd_ntuple.root" ; do
  echo
  ./ana "$file" || exit $?
done 2>&1 | tee "$dir/output.log"

./EfficiencyComparison.py \
VTX "$dir/mc_vtx_ntuple_efficiency.root" \
VXD "$dir/mc_vxd_ntuple_efficiency.root"

./compare_outputs.py "$dir/output.log"

for file in "$dir/mc_vtx_ntuple.root" "$dir/mc_vxd_ntuple.root" ; do
  ./TracksStudy.sh "$file"
done
