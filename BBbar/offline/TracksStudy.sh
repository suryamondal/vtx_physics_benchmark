#!/bin/bash
# Wrapper for TracksStudy.C
# Usage ./TracksStudy.sh path/to/file.root
scriptdir="$(dirname "$0")"
macro="${scriptdir%/}/TracksStudy.C"
root -l -b -q "$macro(\"$1\")"
