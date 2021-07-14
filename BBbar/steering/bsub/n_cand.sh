#!/bin/bash
# Wrapper for the n_cand.cc macro, showing n. candidates in ana. output
# Usage: ./ncand.sh file.root
scriptdir="$(dirname "$0")"
macro="${scriptdir%/}/n_cand.cc"
root -l -b -q "$macro(\"$1\")"
