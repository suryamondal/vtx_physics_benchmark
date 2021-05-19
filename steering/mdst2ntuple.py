 #!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Reconstructs D*+ -> [D0 -> K- pi+ (pi+ pi-)] pi+ from mDST."""

###########################################################
#
# This functions perform the reconstruction of the
# following decay chains (and c.c. decay chain):
#
# D*+ -> D0 pi+
#        |
#        +-> K- pi+
#        +-> K- pi+ pi+ pi-
#
# USAGE:
# > basf2 mdst2ntuple -- --input INPUT --output OUTPUT \
#                        [--isMC] [--gt MC_GLOBALTAG] \
#                        [--looseSelection] [--addTopoAna]
#
# input = input mdst file
# output = output ROOT file
# isMC = add when reconstructing simulated events
# gt = add to specify the global tag for MC
# looseSelection = add to remove selection on tracks
# addTopoAna = add to include TopoAna/MCGen variables
#
# Contributors: G. Casarosa, A. Di Canto (Dec 2019)
#               L. Massaccesi (May 2021)
#
###########################################################

import argparse
import basf2 as b2
import modularAnalysis as ma
import vertex as vx
import variables.collections as vc
import variables.utils as vu
from variables import variables as vm
from variables.MCGenTopo import mc_gen_topo
from basf2 import conditions as b2conditions

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--addTopoAna', action="store_true", help='add TopoAna/MCGen variables for MC')
parser.add_argument('--looseSelection', action="store_true", help='add to remove selection on tracks')
parser.add_argument('--isMC', action="store_true", help='add if you are reconstructing MC')
parser.add_argument("-i", '--input', nargs='+', help='mDST input file(s)')
parser.add_argument("-o", '--output', default='test.root', help='ROOT output file, default test.root')
parser.add_argument('--gt', default='BeamSpotForMC13b', help='Global tag for MC: BeamSpotForMC13b for mc13b_proc11/prompt (default), charm_newBeamSpot for mc13a_proc11, BeamSpotForMC13a for mc13a')
args = parser.parse_args()
b2.B2INFO(f"Steering file args = {args}")

inputfiles = args.input
outputfile = args.output

# new beam-spot payloads
if args.isMC :
    if args.gt != 'default' :
        b2conditions.prepend_globaltag(args.gt)
else :
    b2conditions.prepend_globaltag("beamSpot_timeDep")

# create path
main = b2.create_path()

# load the samples
ma.inputMdstList(filelist=inputfiles, environmentType='default', path=main)

# load pions and kaons from IP and in tracking acceptance
myTrk = '[abs(dr)<1.0] and [abs(dz)<2.0]'
if not args.looseSelection :
    myTrk += ' and [nCDCHits>0]'
ma.fillParticleList('pi+:soft', myTrk, path=main)

if not args.looseSelection :
    myTrk += ' and [nPXDHits>0] and [nSVDHits>0] and [nCDCHits>20]'
ma.fillParticleList('pi+:myTrk', myTrk, path=main)
ma.fillParticleList('K+:myTrk', myTrk, path=main)

#To evaluate the corresponding systematic uncertainty, vary the scale factor within [1.00014, 1.00107].
if not args.isMC :
    ma.trackingMomentum(inputListNames=['pi+:soft','pi+:myTrk','K+:myTrk'], scale=1.00056, path=main)

# D0 reconstruction
myD0 = '1.6 < M < 2.1'
ma.reconstructDecay(decayString='D0:Kpi -> pi+:myTrk K-:myTrk'                     ,cut=myD0,dmID=0,path=main)
ma.reconstructDecay(decayString='D0:K3pi -> pi+:myTrk K-:myTrk pi+:myTrk pi-:myTrk',cut=myD0,dmID=1,path=main)

ma.copyLists('D0:merged',['D0:Kpi','D0:K3pi'],True,path=main)

ma.variablesToExtraInfo("D0:merged", variables={'M': 'M_preFit'}, path=main)

# Dstar reconstruction
myDst = '[massDifference(0)<0.2] and [useCMSFrame(p)>1.5]'
ma.reconstructDecay(decayString='D*+:good -> D0:merged pi+:soft',cut=myDst,path=main)

ma.variablesToExtraInfo("D*+:good", variables={'M': 'M_preFit'}, path=main)

conf_level_cut = 0.001
if args.looseSelection :
    conf_level_cut = -1.
vx.treeFit(list_name='D*+:good',conf_level=conf_level_cut, ipConstraint=True, updateAllDaughters=True, path=main)

ma.applyCuts('D*+:good','[massDifference(0) < 0.16] and [1.7 < daughter(0,M) < 2.05] and [useCMSFrame(p) > 1.8]',path=main)

if args.isMC :
    # perform MC matching (MC truth association)
    ma.matchMCTruth(list_name='D*+:good',path=main)

# List of variables to save
vm.addAlias('IPCovXX','IPCov(0,0)')
vm.addAlias('IPCovYY','IPCov(1,1)')
vm.addAlias('IPCovZZ','IPCov(2,2)')
vm.addAlias('IPCovXY','IPCov(0,1)')
vm.addAlias('IPCovXZ','IPCov(0,2)')
vm.addAlias('IPCovYZ','IPCov(1,2)')
eventWiseVariables = ['nTracks','IPX','IPY','IPZ','IPCovXX','IPCovYY','IPCovZZ','IPCovXY','IPCovXZ','IPCovYZ','beamE','beamPx','beamPy','beamPz']
if args.isMC:
    # eventWiseVariables += ['genIPX','genIPY','genIPZ']  # Variables don't exist?
    # TopoAna variables
    if args.addTopoAna :
        eventWiseVariables += mc_gen_topo(200)

commonVariables = vc.kinematics
commonVariables += ['pErr','ptErr','pxErr','pyErr','pzErr']
if args.isMC:
    commonVariables += vc.mc_variables + ['isSignal','isSignalAcceptMissingGamma','isPrimarySignal']

tracksVariables = vc.track + vc.track_hits + ['pionID','kaonID','protonID','muonID','electronID','firstPXDLayer','firstSVDLayer']
tracksVariables += ['charge','omegaErr','phi0Err','tanlambdaErr','z0Err','d0Err']

compositeVariables = vc.vertex + ['M','ErrM']
if args.isMC:
    compositeVariables += ['mcProductionVertexX', 'mcProductionVertexY', 'mcProductionVertexZ']

flightVariables = vc.flight_info
if args.isMC:
    flightVariables += vc.mc_flight_info

varsKpi = vu.create_aliases_for_selected(list_of_variables=commonVariables,
                                         decay_string='^D*+ -> [^D0 -> ^pi+ ^K-] ^pi+',
                                         prefix=['Dst','Dz','pi','K','pisoft']) + \
          vu.create_aliases_for_selected(list_of_variables=tracksVariables,
                                         decay_string='D*+ -> [D0 -> ^pi+ ^K-] ^pi+',
                                         prefix=['pi','K','pisoft']) + \
          vu.create_aliases_for_selected(list_of_variables=compositeVariables,
                                         decay_string='^D*+ -> [^D0 -> pi+ K-] pi+',
                                         prefix=['Dst','Dz']) + \
          vu.create_aliases_for_selected(list_of_variables=flightVariables,
                                         decay_string='^D*+ -> [^D0 -> pi+ K-] pi+',
                                         prefix=['Dst','Dz'])

varsK3pi = vu.create_aliases_for_selected(list_of_variables=commonVariables,
                                          decay_string='^D*+ -> [^D0 -> ^pi+ ^K- ^pi+ ^pi-] ^pi+',
                                          prefix=['Dst','Dz','pi1','K','pi2','pi3','pisoft']) + \
           vu.create_aliases_for_selected(list_of_variables=tracksVariables,
                                          decay_string='D*+ -> [D0 -> ^pi+ ^K- ^pi+ ^pi-] ^pi+',
                                          prefix=['pi1','K','pi2','pi3','pisoft']) + \
           vu.create_aliases_for_selected(list_of_variables=compositeVariables,
                                          decay_string='^D*+ -> [^D0 -> pi+ K- pi+ pi-] pi+',
                                          prefix=['Dst','Dz']) + \
           vu.create_aliases_for_selected(list_of_variables=flightVariables,
                                          decay_string='^D*+ -> [^D0 -> pi+ K- pi+ pi-] pi+',
                                          prefix=['Dst','Dz'])

vm.addAlias('Dst_M_preFit','extraInfo(M_preFit)')
vm.addAlias('Dz_M_preFit','daughter(0,extraInfo(M_preFit))')
prefit_variables = ['Dst_M_preFit','Dz_M_preFit']

vm.addAlias('Dst_px_CMS','useCMSFrame(px)')
vm.addAlias('Dst_py_CMS','useCMSFrame(py)')
vm.addAlias('Dst_pz_CMS','useCMSFrame(pz)')
vm.addAlias('Dst_p_CMS','useCMSFrame(p)')
cms_variables = ['Dst_px_CMS','Dst_py_CMS','Dst_pz_CMS','Dst_p_CMS']

varsKpi  += prefit_variables + cms_variables + eventWiseVariables
varsK3pi += prefit_variables + cms_variables + eventWiseVariables

# Create one ntuple per channel in the same output file
ma.cutAndCopyList('D*+:Kpi' ,'D*+:good','daughter(0,extraInfo(decayModeID))==0',path=main)
ma.cutAndCopyList('D*+:K3pi','D*+:good','daughter(0,extraInfo(decayModeID))==1',path=main)

ma.variablesToNtuple(decayString='D*+:Kpi' ,variables=varsKpi ,filename=outputfile,treename='DstD0PiKPiRS' ,path=main)
ma.variablesToNtuple(decayString='D*+:K3pi',variables=varsK3pi,filename=outputfile,treename='DstD0PiK3PiRS',path=main)

# Process the events
b2.set_log_level(b2.LogLevel.ERROR)

main.add_module('ProgressBar')
b2.process(main)

print(b2.statistics)
