 #!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Reconstructs D*+ -> [D0 -> K- pi+ (pi+ pi-)] pi+ from mDST."""

##############################################################
# This functions perform the reconstruction of the
# following decay chains (and c.c. decay chain):
#
# D*+ -> D0 pi+
#        |
#        +-> K- pi+
#        +-> K- pi+ pi+ pi-
#
# USAGE:
# > basf2 mdst2ntuple.py -- --input INPUT --output OUTPUT \
#                           [--looseSelection] [--addTopoAna]
#
# input = input mdst file
# output = output ROOT file
# looseSelection = add to remove selection on tracks
# addTopoAna = add to include TopoAna/MCGen variables
#
# Contributors: G. Casarosa, A. Di Canto (Dec 2019)
#               L. Massaccesi (June 2021)
#
# Software Version: release-05-02-06 / upgrade / master
#
# Must use release-05-02-06 or master without --vtx
# Must use upgrade branch with --vtx
# The script will terminate if used with the wrong version.
##############################################################

import argparse
import os
import basf2 as b2
import modularAnalysis as ma
import vertex as vx
import variables.collections as vc
import variables.utils as vu
from variables import variables as vm
from variables.MCGenTopo import mc_gen_topo

try:  # Dummy import to check basf2 version
    import vtx
    HAS_VTX = True
except ImportError:
    HAS_VTX = False


def check_globaltags(base_tags, user_tags, metadata):
    """Checks the compatibility of the GTs with the verions of basf2."""
    if not base_tags:
        b2.B2FATAL("No baseline GT available. Input files probably have incompatible GTs.")
    if not metadata:
        b2.B2FATAL("No GT in input files' metadata, can't check if upgrade was used.")
    md_tags = metadata[0].getDatabaseGlobalTag()
    for md in metadata:
        if md.getDatabaseGlobalTag() != md_tags:
            b2.B2FATAL("Input files used different GTs.")
    tags = base_tags + user_tags
    has_release, has_upgrade = False, False
    for tag in tags:
        if tag.startswith("release"): has_release = True
        elif tag.startswith("upgrade"): has_upgrade = True
    if has_release and has_upgrade:
        b2.B2FATAL("Invalid combination of GTs used.", globaltags=tags)
    if has_upgrade != HAS_VTX:
        kwa_ver = {'globaltags': tags, 'hasVTX': HAS_VTX}
        for var_name in ["RELEASE", "RELEASE_DIR", "LOCAL_DIR"]:
            var_name = f"BELLE2_{var_name}"
            kwa_ver[var_name] = os.environ.get(var_name, "")
        b2.B2FATAL("Incorrect version of basf2 for these GTs.", **kwa_ver)
    return tags  # Standard behaviour


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--addTopoAna', action="store_true",
                    help='add TopoAna/MCGen variables for MC')
parser.add_argument('--looseSelection', action="store_true",
                    help='add to remove selection on tracks')
parser.add_argument("-i", '--input', nargs='+',
                    help='mDST input file(s)')
parser.add_argument("-o", '--output', default='test.root',
                    help='ROOT output file, default test.root')
args = parser.parse_args()
b2.B2INFO(f"Steering file args = {args}")

# Set basf2 to check the input files' globaltags and the version used
b2.conditions.set_globaltag_callback(check_globaltags)

# create path
main = b2.create_path()

# load the samples
ma.inputMdstList(filelist=args.input, environmentType='default', path=main)

# load pions and kaons from IP and in tracking acceptance
myTrk = ''
myTrkSoft = myTrk + ''
ma.fillParticleList('pi+:soft', myTrkSoft, path=main)

if not args.looseSelection:
    if HAS_VTX:
        myTrk += ''
    else:
        myTrk += ''
ma.fillParticleList('pi+:myTrk', myTrk, path=main)
ma.fillParticleList('K+:myTrk', myTrk, path=main)

# D0 reconstruction
myD0 = ''  # '0.86 < M < 2.86'  # M_D0 = 1.8648 GeV
ma.reconstructDecay('D0:Kpi -> pi+:myTrk K-:myTrk',
                    cut=myD0, dmID=0, path=main)
ma.reconstructDecay('D0:K3pi -> pi+:myTrk K-:myTrk pi+:myTrk pi-:myTrk',
                    cut=myD0, dmID=1, path=main)

ma.copyLists('D0:merged', ['D0:Kpi', 'D0:K3pi'], True, path=main)

ma.variablesToExtraInfo("D0:merged", variables={'M': 'M_preFit'}, path=main)

# Dstar reconstruction
myDst = ''  # 'massDifference(0) < 1'  # M_D* = 2.0103, diff = 0.1455
ma.reconstructDecay('D*+:good -> D0:merged pi+:soft', cut=myDst, path=main)

ma.variablesToExtraInfo("D*+:good", variables={'M': 'M_preFit'}, path=main)

conf_level_cut = -1.0 if args.looseSelection else 0.001
vx.treeFit(list_name='D*+:good', conf_level=conf_level_cut, ipConstraint=False,
           updateAllDaughters=True, path=main)

myCuts = ''
ma.applyCuts('D*+:good', myCuts, path=main)

ma.matchMCTruth(list_name='D*+:good', path=main)

# List of variables to save
vm.addAlias('IPCovXX', 'IPCov(0,0)')
vm.addAlias('IPCovYY', 'IPCov(1,1)')
vm.addAlias('IPCovZZ', 'IPCov(2,2)')
vm.addAlias('IPCovXY', 'IPCov(0,1)')
vm.addAlias('IPCovXZ', 'IPCov(0,2)')
vm.addAlias('IPCovYZ', 'IPCov(1,2)')
eventWiseVariables = [
    'nTracks', 'IPX', 'IPY', 'IPZ',
    'IPCovXX', 'IPCovYY', 'IPCovZZ', 'IPCovXY', 'IPCovXZ', 'IPCovYZ',
    'beamE', 'beamPx', 'beamPy', 'beamPz']
# These variables don't exist in release-05
if 'genIPX' in (v.name for v in vm.getVariables()):
    eventWiseVariables += ['genIPX', 'genIPY', 'genIPZ']
if args.addTopoAna:  # TopoAna variables
    eventWiseVariables += mc_gen_topo(200)

commonVariables = vc.kinematics
commonVariables += ['pErr', 'ptErr', 'pxErr', 'pyErr', 'pzErr']
commonVariables += vc.mc_variables
commonVariables += ['isSignal', 'isSignalAcceptMissingGamma', 'isPrimarySignal']

tracksVariables = vc.track + vc.track_hits
tracksVariables += ['pionID', 'kaonID', 'protonID', 'muonID', 'electronID']
if HAS_VTX:
    tracksVariables += ['firstVTXLayer']
else:
    tracksVariables += ['firstPXDLayer', 'firstSVDLayer']
# This variable changed name after release-05
if 'tanlambdaErr' in (v.name for v in vm.getVariables()):
    vm.addAlias("tanLambdaErr", "tanlambdaErr")
tracksVariables += ['tanLambdaErr']
tracksVariables += ['charge', 'omegaErr', 'phi0Err', 'z0Err', 'd0Err']

compositeVariables = vc.vertex + ['M', 'ErrM']
compositeVariables += ['mcProductionVertexX', 'mcProductionVertexY',
                       'mcProductionVertexZ']

flightVariables = vc.flight_info
flightVariables += vc.mc_flight_info

varsKpi = vu.create_aliases_for_selected(
    commonVariables, '^D*+ -> [^D0 -> ^pi+ ^K-] ^pi+',
    ['Dst', 'D0', 'pi', 'K', 'pisoft'])
varsKpi += vu.create_aliases_for_selected(
    tracksVariables, 'D*+ -> [D0 -> ^pi+ ^K-] ^pi+', ['pi', 'K', 'pisoft'])
varsKpi += vu.create_aliases_for_selected(
    compositeVariables, '^D*+ -> [^D0 -> pi+ K-] pi+', ['Dst', 'D0'])
varsKpi += vu.create_aliases_for_selected(
    flightVariables, '^D*+ -> [^D0 -> pi+ K-] pi+', ['Dst', 'D0'])

varsK3pi = vu.create_aliases_for_selected(
    commonVariables, '^D*+ -> [^D0 -> ^pi+ ^K- ^pi+ ^pi-] ^pi+',
    ['Dst', 'D0', 'pi1', 'K', 'pi2', 'pi3', 'pisoft'])
varsK3pi += vu.create_aliases_for_selected(
    tracksVariables, 'D*+ -> [D0 -> ^pi+ ^K- ^pi+ ^pi-] ^pi+',
    ['pi1', 'K', 'pi2', 'pi3', 'pisoft'])
varsK3pi += vu.create_aliases_for_selected(
    compositeVariables, '^D*+ -> [^D0 -> pi+ K- pi+ pi-] pi+', ['Dst', 'D0'])
varsK3pi += vu.create_aliases_for_selected(
    flightVariables, '^D*+ -> [^D0 -> pi+ K- pi+ pi-] pi+', ['Dst', 'D0'])

vm.addAlias('Dst_M_preFit', 'extraInfo(M_preFit)')
vm.addAlias('D0_M_preFit', 'daughter(0,extraInfo(M_preFit))')
prefit_variables = ['Dst_M_preFit', 'D0_M_preFit']

vm.addAlias('Dst_px_CMS', 'useCMSFrame(px)')
vm.addAlias('Dst_py_CMS', 'useCMSFrame(py)')
vm.addAlias('Dst_pz_CMS', 'useCMSFrame(pz)')
vm.addAlias('Dst_p_CMS', 'useCMSFrame(p)')
cms_variables = ['Dst_px_CMS', 'Dst_py_CMS', 'Dst_pz_CMS', 'Dst_p_CMS']

varsKpi += prefit_variables + cms_variables + eventWiseVariables
varsK3pi += prefit_variables + cms_variables + eventWiseVariables

# Create one ntuple per channel in the same output file
ma.cutAndCopyList('D*+:Kpi', 'D*+:good', 'daughter(0,extraInfo(decayModeID))==0', path=main)
ma.cutAndCopyList('D*+:K3pi', 'D*+:good', 'daughter(0,extraInfo(decayModeID))==1', path=main)

ma.variablesToNtuple(
    decayString='D*+:Kpi', variables=varsKpi, filename=args.output,
    treename='Dst_D0pi_Kpi', path=main)
ma.variablesToNtuple(
    decayString='D*+:K3pi', variables=varsK3pi, filename=args.output,
    treename='Dst_D0pi_K3pi', path=main)

# Process the events
main.add_module('ProgressBar')
b2.process(main)
print(b2.statistics)
