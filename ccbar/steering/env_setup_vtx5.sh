#!/bin/bash

# export BELLE2_VTX_UPGRADE_GT=upgrade_2021-07-16_vtx_5layer
export BELLE2_VTX_UPGRADE_GT=upgrade_2022-01-21_vtx_5layer
export BELLE2_VTX_BACKGROUND_DIR=/group/belle2/TMP/benjamin/data_prod/bgsim/upgrade_bgprod/nominal_phase3/data_VTX-CMOS-5layer/overlay/phase3/BGx1/set0/

# To run explicitly w/o bg overlay:
# export BELLE2_VTX_BACKGROUND_DIR=None

# Go to tracking/validation and execute:
# b2validation -s VTX_13_trackingEfficiency_createPlots.py
