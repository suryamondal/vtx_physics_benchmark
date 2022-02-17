#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################
# This functions perform the reconstruction of the
# following decay chains (and c.c. decay chain):
#
# anti-B0 -> D*+ mu- anti-nu_mu
#            |
#            +-> D0 pi+
#                |
#                +-> K- pi+
#                +-> K- pi+ pi+ pi-
#
# USAGE:
# > basf2 mdst2ntuple.py -- -i INPUT -o OUTPUT [--addTopoAna] \
#                           [-c loose|normal|tight]
#
# input = input mdst file
# output = output ROOT file
# c = cuts to be used (see README)
# addTopoAna = add to include TopoAna/MCGen variables
#
# Contributors: G. Casarosa, A. Di Canto (Dec 2019)
#               L. Massaccesi (June 2021)
#
# Software Version: upgrade or master branch
#
# Must use the same version of basf2 used for input files
# generation (master without --vtx, upgrade with --vtx).
# The script will terminate if used with the wrong version.
################################################################

import argparse
import os
import sys
import re
import basf2 as b2
import modularAnalysis as ma
import flavorTagger as ft
import vertex as vx
import variables.collections as vc
import variables.utils as vu
import pdg
from ROOT import Belle2
from variables import variables as vm
from variables.MCGenTopo import mc_gen_topo

try:  # Dummy import to check basf2 version
    import vtx
    HAS_VTX = True
except ImportError:
    HAS_VTX = False

b2.set_log_level(b2.LogLevel.DEBUG)

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--addTopoAna', action="store_true",
                    help='add TopoAna/MCGen variables for MC')
parser.add_argument('--printVars', action="store_true",
                    help='Just print variables and exit.')
parser.add_argument("-c", "--cuts", choices=["loose", "normal", "tight"],
                    default="tight", help="Set of cuts to be used.")
parser.add_argument("-i", '--input', nargs='+',
                    help='mDST input file(s)')
parser.add_argument("-o", '--output', default='test.root',
                    help='ROOT output file, default test.root')
args = parser.parse_args()
b2.B2INFO(f"Steering file args = {args}")

CUTS = {
    "loose": {  # These are intended for intial study of cuts only
        "pisoft": "",
        "pi": "",
        "K": "",
        "mu": "",
        "D0": "abs(dM)<0.45",
        "D*": "[abs(dM)<0.4] and [massDifference(0)<0.2]",
        "B0": "",
        "candidateLimit": 1000,
        "chargeViolation": False,
        "ipConstraint": True,
        "conf": -1.0,
        "fit": "[daughter(0, useCMSFrame(p))<3]"
    },
    "normal": {
        "pisoft": "[dr<2] and [abs(dz)<2] and [nVXDHits>0]",  # VXD=PXD+SVD+VTX
        "pi": "[dr<2] and [abs(dz)<2] and [nVXDHits>0]",
        "K": "[dr<2] and [abs(dz)<2] and [nVXDHits>0]",
        "mu": "[dr<2] and [abs(dz)<2] and [nVXDHits>0]",
        "D0": "abs(dM)<0.45",  # M_D0 = 1.8648 GeV, M_D* = 2.0103, diff = 0.1455
        "D*": "[abs(dM)<0.4] and [massDifference(0)<0.2]",
        "B0": "",  # Missing neutrino
        "candidateLimit": 1000,
        "chargeViolation": False,
        "ipConstraint": True,
        "conf": 0.001,
        "fit": "[Dst_p_CMS<2.5]"
    },
    "tight": {  # These are mostly identical to those of the offline analysis
        "pisoft": "[dr<2] and [abs(dz)<2] and [nVXDHits>0]",
        "pi": "[dr<2] and [abs(dz)<2] and [nVXDHits>0]",
        "K": "[dr<2] and [abs(dz)<2] and [nVXDHits>0]",
        "mu": "[dr<2] and [abs(dz)<2] and [nVXDHits>0]",
        "D0": "abs(dM)<0.1",
        "D*": "[abs(dM)<0.1] and [massDifference(0)<0.151]",
        "B0": "",  # Missing neutrino
        "candidateLimit": 1000,
        "chargeViolation": False,
        "ipConstraint": True,
        "conf": 0.001,
        "fit": "[Dst_p_CMS<2.5] and [mu_E>0.4] and [Dst_E>formula(-3.0*mu_E+3.0)]"
    }# ,
    # "tight": {  # These are mostly identical to those of the offline analysis
    #     "pisoft": "[dr<2] and [abs(dz)<2] and [nVXDHits>0] and [ndf>0]",
    #     "pi": "[dr<2] and [abs(dz)<2] and [nVXDHits>0] and [ndf>0]",
    #     "K": "[dr<2] and [abs(dz)<2] and [nVXDHits>0] and [ndf>0]",
    #     "mu": "[dr<2] and [abs(dz)<2] and [nVXDHits>0] and [ndf>0]",
    #     "D0": "abs(dM)<0.1",
    #     "D*": "[abs(dM)<0.1] and [massDifference(0)<0.151]",
    #     "B0": "",  # Missing neutrino
    #     "candidateLimit": 1000,
    #     "chargeViolation": False,
    #     "ipConstraint": True,
    #     "conf": 0.001,
    #     "fit": "[Dst_p_CMS<2.5] and [mu_E>0.4] and [Dst_E>formula(-3.0*mu_E+3.0)]"
    # }
}

cuts = CUTS[args.cuts]
print(cuts)


# Regex to select vars that start with (particle_) gen or mc
RE_MC_VARS = re.compile(r'(?:[A-Za-z0-9]+_)?(?:[Gg][Ee][Nn]|[Mm][Cc])')


class Particle:
    """A minimalistic particle class for FilterByDecay that works in this case."""

    def __init__(self, str_or_particle, daughters=None):
        self.daughters = []
        if isinstance(str_or_particle, str):
            self.pdg = abs(pdg.from_name(str_or_particle))
            if not self.pdg in (13, 211, 321):  # FS: pi+, K+, mu
                self.daughters = list(daughters or [])
        else:
            self.pdg = abs(str_or_particle.getPDGCode())
            if not self.pdg in (13, 211, 321):  # FS: pi+, K+, mu
                self.daughters = [Particle(d) for d in str_or_particle.getDaughters()
                                  if abs(d.getPDGCode()) != 22]  # Skip photons
        self.daughters.sort()  # Makes comparisons easier

    def __lt__(self, other):
        return self.pdg < other.pdg

    def __eq__(self, other):
        return self.pdg == other.pdg and self.daughters == other.daughters

    # def __str__(self):  # For debugging
    #     if self.daughters:
    #         return f"[{pdg.to_name(self.pdg)} -> {' '.join(str(x) for x in self.daughters)}]"
    #     return pdg.to_name(self.pdg)


class FilterByDecay(b2.Module):
    """A python module to filter MC particles by their decay tree."""

    def __init__(self, pListName, pDecayTree):
        self.pListName = pListName
        self.pDecayTree = pDecayTree
        super().__init__()

    def initialize(self):
        self.pList = Belle2.PyStoreObj(self.pListName)
        b2.B2INFO(f"FilterByDecay: {self.pListName}")

    def event(self):
        toRemove = []
        for i in range(self.pList.getListSize()):
            part = self.pList.getParticle(i)
            keep = Particle(part) == self.pDecayTree
            # b2.B2INFO(f"Particle: {part}", keep=keep)  # For debugging
            if not keep: toRemove.append(part.getArrayIndex())
        self.pList.removeParticles(toRemove)


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

# Set basf2 to check the input files' globaltags and the version used
b2.conditions.set_globaltag_callback(check_globaltags)

# create path
main = b2.create_path()

# Load the samples
ma.inputMdstList(filelist=args.input, environmentType='default', path=main)

# load pions, kaons and muons from IP and in tracking acceptance
ma.fillParticleList('pi+:soft', cuts["pisoft"], path=main)
ma.fillParticleList('pi+:myTrk', cuts["pi"], path=main)
ma.fillParticleList('K+:myTrk', cuts["K"], path=main)
ma.fillParticleList('mu+:myTrk', cuts["mu"], path=main)

# D0 reconstruction
ma.reconstructDecay('D0:Kpi -> pi+:myTrk K-:myTrk',
                    cut=cuts["D0"], dmID=0, allowChargeViolation=cuts["chargeViolation"],
                    path=main)
ma.reconstructDecay('D0:K3pi -> pi+:myTrk K-:myTrk pi+:myTrk pi-:myTrk',
                    cut=cuts["D0"], dmID=1, allowChargeViolation=cuts["chargeViolation"],
                    path=main)
ma.copyLists('D0:merged', ['D0:Kpi', 'D0:K3pi'], True, path=main)
ma.variablesToExtraInfo("D0:merged", variables={'M': 'M_preFit'}, path=main)

# vx.treeFit(list_name='D0:merged', conf_level=cuts["conf"],
#            # ipConstraint=cuts["ipConstraint"], 
#            updateAllDaughters=True, path=main)

# D* reconstruction
ma.reconstructDecay('D*+:good -> D0:merged pi+:soft', cut=cuts["D*"], path=main)
ma.variablesToExtraInfo("D*+:good", variables={'M': 'M_preFit'}, path=main)

# vx.treeFit(list_name='D*+:good', conf_level=cuts["conf"],
#            # ipConstraint=cuts["ipConstraint"], 
#            updateAllDaughters=True, path=main)

# B0 reconstruction
ma.reconstructDecay('B0:good -> D*-:good mu+:myTrk ?nu', cut=cuts["B0"],
                    candidate_limit=cuts["candidateLimit"],
                    path=main)
ma.variablesToExtraInfo("B0:good", variables={"M": "M_preFit"}, path=main)

# Tree fitting and final ajustments
vx.treeFit(list_name='B0:good', conf_level=cuts["conf"],
           ipConstraint=cuts["ipConstraint"], updateAllDaughters=True, path=main)
ma.applyCuts('B0:good', cuts["fit"], path=main)
ma.matchMCTruth(list_name='B0:good', path=main)

# # build the rest of the event
# ma.buildRestOfEvent("B0:good", fillWithMostLikely=True, path=main)

# # call flavor tagging
# ft.flavorTagger(["B0:good"], path=main)

# Best-candidate selection (does not cut, only adds the rank variable)
ma.rankByHighest("B0:good", "M", allowMultiRank=True, path=main)
ma.rankByHighest("B0:good", "chiProb", allowMultiRank=True, path=main)
ma.rankByHighest("B0:good", "formula(B0_chiProb*Dst_chiProb*D0_chiProb)",
                 allowMultiRank=True, outputVariable="B0DstD0_chiProb_rank", path=main)
ma.rankByLowest("B0:good", "abs(Dst_dM)", allowMultiRank=True, outputVariable="Dst_dM_rank", path=main)
ma.rankByLowest("B0:good", "abs(D0_dM)", allowMultiRank=True, outputVariable="D0_dM_rank", path=main)

# Variables of the event
eventWiseVariables = ['IPX', 'IPY', 'IPZ', 'genIPX', 'genIPY', 'genIPZ',
                      'nTracks', 'beamE', 'beamPx', 'beamPy', 'beamPz']
eventWiseVariables += ['nMCParticles']
for i, a in enumerate('XYZ'): # IPCovXX -> IPCov(0,0), etc
    for j, b in enumerate('XYZ'[i:]):
        vm.addAlias(f'IPCov{a}{b}', f'IPCov({i},{j})')
        eventWiseVariables.append(f'IPCov{a}{b}')
if args.addTopoAna:  # TopoAna variables
    eventWiseVariables += mc_gen_topo(200)

# Variables of all the particles
commonVariables = vc.kinematics
commonVariables += vc.mc_kinematics
cmskinematics = []
for v in commonVariables:
    vm.addAlias(f'{v}_CMS', f'useCMSFrame({v})')
    cmskinematics.append(f'{v}_CMS')
commonVariables += cmskinematics
commonVariables += vc.mc_truth
commonVariables += ['pErr', 'ptErr', 'pxErr', 'pyErr', 'pzErr']
commonVariables += vc.mc_variables + ['isSignalAcceptMissingGamma', 'isPrimarySignal',
                                      'isSignalAcceptMissingNeutrino',
                                      'theta', 'thetaErr', 'mcTheta', 'phi', 'phiErr', 'mcPhi']

# Variables of the final-state particles (K, pi, mu)
tracksVariables = vc.pid + ['particleID'] + vc.track + ['nCDCHits', 'nVXDHits']
tracksVariables += ['nVTXHits'] if HAS_VTX else  ['nPXDHits', 'nSVDHits']
tracksVariables += ['firstVTXLayer'] if HAS_VTX else ['firstPXDLayer', 'firstSVDLayer']
tracksVariables += ['trackNECLClusters', 'nMatchedKLMClusters', 'klmClusterLayers']
tracksVariables += ['charge', 'omega', 'phi0', 'z0', 'd0',
                    'omegaPull', 'phi0Pull', 'z0Pull', 'd0Pull', 'tanLambdaPull']
tracksVariables += ['omegaErr', 'phi0Err', 'z0Err', 'd0Err', 'tanLambdaErr']

# Variables of the composite particles (D0, D*, B0)
compositeVariables = vc.vertex + vc.inv_mass + ['M_preFit', 'dM']
compositeVariables += vc.mc_vertex

flightVariables = vc.flight_info
flightVariables += vc.mc_flight_info

decaymodeVariable = ['decayModeID']

# Create aliases for the two decay modes
varsKpi = vu.create_aliases_for_selected(
    commonVariables, '^B0 -> [^D*- -> [^anti-D0 -> ^pi- ^K+] ^pi-] ^mu+',
    ['B0', 'Dst', 'D0', 'pi', 'K', 'pisoft', 'mu'])
# varsKpi += vu.create_aliases_for_selected(
#     tracksVariables, 'B0 -> [D*- -> [anti-D0 -> pi- ^K+] pi-] mu+',
#     ['K'])
varsKpi += vu.create_aliases_for_selected(
    tracksVariables, 'B0 -> [D*- -> [anti-D0 -> ^pi- ^K+] ^pi-] ^mu+',
    ['pi', 'K', 'pisoft', 'mu'])
varsKpi += vu.create_aliases_for_selected(
    compositeVariables, '^B0 -> [^D*- -> [^anti-D0 -> pi- K+] pi-] mu+',
    ['B0', 'Dst', 'D0'])
varsKpi += vu.create_aliases_for_selected(
    flightVariables, '^B0 -> [^D*- -> [^anti-D0 -> pi- K+] pi-] mu+',
    ['B0', 'Dst', 'D0'])
varsKpi += vu.create_aliases_for_selected(
    decaymodeVariable, 'B0 -> [D*- -> [^anti-D0 -> pi- K+] pi-] mu+',
    ['D0'])

varsK3pi = vu.create_aliases_for_selected(
    commonVariables, '^B0 -> [^D*- -> [^anti-D0 -> ^pi- ^K+ ^pi- ^pi+] ^pi-] ^mu+',
    ['B0', 'Dst', 'D0', 'pi1', 'K', 'pi2', 'pi3', 'pisoft', 'mu'])
# varsK3pi += vu.create_aliases_for_selected(
#     tracksVariables, 'B0 -> [D*- -> [anti-D0 -> pi- ^K+ pi- pi+] pi-] mu+',
#     ['K'])
varsK3pi += vu.create_aliases_for_selected(
    tracksVariables, 'B0 -> [D*- -> [anti-D0 -> ^pi- ^K+ ^pi- ^pi+] ^pi-] ^mu+',
    ['pi1', 'K', 'pi2', 'pi3', 'pisoft', 'mu'])
varsK3pi += vu.create_aliases_for_selected(
    compositeVariables, '^B0 -> [^D*- -> [^anti-D0 -> pi- K+ pi- pi+] pi-] mu+',
    ['B0', 'Dst', 'D0'])
varsK3pi += vu.create_aliases_for_selected(
    flightVariables, '^B0 -> [^D*- -> [^anti-D0 -> pi- K+ pi- pi+] pi-] mu+',
    ['B0', 'Dst', 'D0'])
varsK3pi += vu.create_aliases_for_selected(
    decaymodeVariable, 'B0 -> [D*- -> [^anti-D0 -> pi- K+ pi- pi+] pi-] mu+',
    ['D0'])

# FT variables of the composites
ft_variables = []
# for v in ft.flavor_tagging:
#     vm.addAlias(f'B0_{v}', f'{v}')
#     ft_variables.append(f'B0_{v}')

# Angle between pi and K
vm.addAlias('Kpi_MCAngle', 'daughter(0,daughter(0,mcDaughterAngle(0,1)))')
vm.addAlias("B0_M_rank", "extraInfo(M_rank)")
vm.addAlias("B0_chiProb_rank", "extraInfo(chiProb_rank)")
vm.addAlias("B0DstD0_chiProb_rank", "extraInfo(B0DstD0_chiProb_rank)")
vm.addAlias("Dst_dM_rank", "extraInfo(Dst_dM_rank)")
vm.addAlias("D0_dM_rank", "extraInfo(D0_dM_rank)")

vm.addAlias('M_preFit', 'extraInfo(M_preFit)')
vm.addAlias('decayModeID', 'extraInfo(decayModeID)')

rankVariables = ["B0_M_rank", "B0_chiProb_rank", "Dst_dM_rank", "D0_dM_rank",
                 "B0DstD0_chiProb_rank"]

customVariables = []

# Final output variables
varsKpi  += eventWiseVariables + ft_variables + rankVariables + customVariables + ['Kpi_MCAngle']
varsK3pi += eventWiseVariables + ft_variables + rankVariables + customVariables
varsKpi.sort()  # I want to be able to find what I need quickly
varsK3pi.sort()
if args.printVars:
    print(" =============== Kpi  ===============")
    for x in varsKpi: print(x)
    print(" =============== K3pi ===============")
    for x in varsK3pi: print(x)
    vm.printAliases()
    sys.exit()

# Create one ntuple per channel in the same output file
ma.cutAndCopyList('B0:Kpi',  'B0:good', 'D0_decayModeID==0', path=main)
ma.cutAndCopyList('B0:K3pi', 'B0:good', 'D0_decayModeID==1', path=main)

ma.variablesToNtuple('B0:Kpi', varsKpi, filename=args.output, treename='Kpi', path=main)
ma.variablesToNtuple('B0:K3pi', varsK3pi, filename=args.output, treename='K3pi', path=main)

# MC particles lists for efficiency studies
ma.fillParticleListFromMC("B0:MCKpi", "", addDaughters=True, path=main)
ma.fillParticleListFromMC("B0:MCK3pi", "", addDaughters=True, path=main)
main.add_module(FilterByDecay("B0:MCKpi", Particle("B0", [
    Particle("D*+", [
        Particle("D0", [
            Particle("K+"),
            Particle("pi+")]),
        Particle("pi+")]),
    Particle("mu+"),
    Particle("nu_mu")])))
main.add_module(FilterByDecay("B0:MCK3pi", Particle("B0", [
    Particle("D*+", [
        Particle("D0", [
            Particle("K+"),
            Particle("pi+"),
            Particle("pi+"),
            Particle("pi+")]),
        Particle("pi+")]),
    Particle("mu+"),
    Particle("nu_mu")])))

varsMCKpi = [x for x in varsKpi if RE_MC_VARS.match(x)]
varsMCK3pi = [x for x in varsK3pi if RE_MC_VARS.match(x)]
ma.variablesToNtuple('B0:MCKpi', varsMCKpi, filename=args.output, treename='MCKpi', path=main)
ma.variablesToNtuple('B0:MCK3pi', varsMCK3pi, filename=args.output, treename='MCK3pi', path=main)

# FS-particles lists for tracks study
varsFS = ["isCloneTrack", "mcE", "mcP", "mcPT", "mcPX", "mcPY", "mcPZ",
          "mcPDG", "isSignal", "isSignalAcceptWrongFSPs", "nTracks"]
varsFS += vc.track_hits
if HAS_VTX: varsFS.append('nVTXHits')
ma.variablesToNtuple('pi+:soft', varsFS, filename=args.output, treename='Tracks', path=main)

# Process the events
main.add_module('ProgressBar')
b2.process(main)
print(b2.statistics)
