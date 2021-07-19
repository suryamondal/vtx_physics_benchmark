 #!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Reconstructs D*+ -> [D0 -> K- pi+ (pi+ pi-)] pi+ from mDST."""

################################################################
# This functions perform the reconstruction of the
# following decay chains (and c.c. decay chain):
#
# D*+ -> D0 pi+
#        |
#        +-> K- pi+
#        +-> K- pi+ pi+ pi-
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
import basf2 as b2
import modularAnalysis as ma
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

CUTS = {
    "loose": {  # These are intended for intial study of cuts only
        "pisoft": "",
        "pi": "",
        "K": "",
        "mu": "",
        "D0": "abs(dM) < 0.45",
        "D*": "[abs(dM) < 0.4] and [massDifference(0) < 0.2]",
        "B0": "",
        "fit": "[daughter(0, useCMSFrame(p)) < 3]"
    },
    "normal": {
        "pisoft": "[dr < 2] and [abs(dz) < 2] and [nVXDHits > 0]",  # VXD=PXD+SVD+VTX
        "pi": "[dr < 2] and [abs(dz) < 2] and [nVXDHits > 0]",
        "K": "[dr < 2] and [abs(dz) < 2] and [nVXDHits > 0]",
        "mu": "[dr < 2] and [abs(dz) < 2] and [nVXDHits > 0]",
        "D0": "abs(dM) < 0.45",  # M_D0 = 1.8648 GeV, M_D* = 2.0103, diff = 0.1455
        "D*": "[abs(dM) < 0.4] and [massDifference(0) < 0.2]",
        "B0": "",  # Missing neutrino
        "fit": "[daughter(0, useCMSFrame(p)) < 2.5]"
    },
    "tight": {  # These are mostly identical to those of the offline analysis
        "pisoft": "[dr < 2] and [abs(dz) < 2] and [nVXDHits > 0]",
        "pi": "[dr < 2] and [abs(dz) < 2] and [nVXDHits > 0]",
        "K": "[dr < 2] and [abs(dz) < 2] and [nVXDHits > 0]",
        "mu": "[dr < 2] and [abs(dz) < 2] and [nVXDHits > 0]",
        "D0": "abs(dM) < 0.1",
        "D*": "[abs(dM) < 0.1] and [massDifference(0) < 0.151]",
        "B0": "",  # Missing neutrino
        "fit": "[daughter(0, useCMSFrame(p)) < 2.5]"
    }
}


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


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--addTopoAna', action="store_true",
                    help='add TopoAna/MCGen variables for MC')
parser.add_argument("-c", "--cuts", choices=["loose", "normal", "tight"],
                    default="normal", help="Set of cuts to be used.")
parser.add_argument("-i", '--input', nargs='+',
                    help='mDST input file(s)')
parser.add_argument("-o", '--output', default='test.root',
                    help='ROOT output file, default test.root')
args = parser.parse_args()
b2.B2INFO(f"Steering file args = {args}")
cuts = CUTS[args.cuts]

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
                    cut=cuts["D0"], dmID=0, path=main)
ma.reconstructDecay('D0:K3pi -> pi+:myTrk K-:myTrk pi+:myTrk pi-:myTrk',
                    cut=cuts["D0"], dmID=1, path=main)
ma.copyLists('D0:merged', ['D0:Kpi', 'D0:K3pi'], True, path=main)
ma.variablesToExtraInfo("D0:merged", variables={'M': 'M_preFit'}, path=main)

# D* reconstruction
ma.reconstructDecay('D*+:good -> D0:merged pi+:soft', cut=cuts["D*"], path=main)
ma.variablesToExtraInfo("D*+:good", variables={'M': 'M_preFit'}, path=main)

# B0 reconstruction
ma.reconstructDecay('B0:good -> D*-:good mu+:myTrk', cut=cuts["B0"], path=main)
ma.variablesToExtraInfo("B0:good", variables={"M": "M_preFit"}, path=main)

# Tree fitting and final ajustments
conf_level_cut = -1.0 if args.cuts == "loose" else 0.001
vx.treeFit(list_name='B0:good', conf_level=conf_level_cut,
           ipConstraint=False,  # TODO ipConstraint=True, originDimension=2, ??
           updateAllDaughters=True, path=main)
ma.applyCuts('B0:good', cuts["fit"], path=main)
ma.matchMCTruth(list_name='B0:good', path=main)

# Variables of the event
eventWiseVariables = ['IPX', 'IPY', 'IPZ', 'genIPX', 'genIPY', 'genIPZ',
                      'nTracks', 'beamE', 'beamPx', 'beamPy', 'beamPz']
for i, a in enumerate('XYZ'): # IPCovXX -> IPCov(0,0), etc
    for j, b in enumerate('XYZ'[i:]):
        vm.addAlias(f'IPCov{a}{b}', f'IPCov({i},{j})')
        eventWiseVariables.append(f'IPCov{a}{b}')
if args.addTopoAna:  # TopoAna variables
    eventWiseVariables += mc_gen_topo(200)

# Variables of all the particles
commonVariables = vc.kinematics
commonVariables += ['pErr', 'ptErr', 'pxErr', 'pyErr', 'pzErr']
commonVariables += vc.mc_variables
commonVariables += ['isSignal', 'isSignalAcceptMissingGamma', 'isPrimarySignal',
                    'isSignalAcceptMissingNeutrino']

# Variables of the final-state particles (K, pi, mu)
tracksVariables = vc.track + vc.track_hits
if HAS_VTX: tracksVariables.append('nVTXHits')
tracksVariables += ['pionID', 'kaonID', 'protonID', 'muonID', 'electronID',
                    'charge', 'omegaErr', 'phi0Err', 'z0Err', 'd0Err', 'tanLambdaErr',
                    'omegaPull', 'phi0Pull', 'z0Pull', 'd0Pull', 'tanLambdaPull']
tracksVariables += ['firstVTXLayer'] if HAS_VTX else ['firstPXDLayer', 'firstSVDLayer']

# Variables of the composite particles (D0, D*, B0)
compositeVariables = vc.vertex + ['M', 'ErrM']
compositeVariables += ['mcProductionVertexX', 'mcProductionVertexY',
                       'mcProductionVertexZ', 'extraInfo(M_preFit)']
compositeVariables += vc.flight_info + vc.mc_flight_info

# Create aliases for the two decay modes
varsKpi = vu.create_aliases_for_selected(
    commonVariables, '^B0 -> [^D*- -> [^anti-D0 -> ^pi- ^K+] ^pi-] ^mu+',
    ['B0', 'Dst', 'D0', 'pi', 'K', 'pisoft', 'mu'])
varsKpi += vu.create_aliases_for_selected(
    tracksVariables, 'B0 -> [D*- -> [anti-D0 -> ^pi- ^K+] ^pi-] ^mu+',
    ['pi', 'K', 'pisoft', 'mu'])
varsKpi += vu.create_aliases_for_selected(
    compositeVariables, '^B0 -> [^D*- -> [^anti-D0 -> pi- K+] pi-] mu+',
    ['B0', 'Dst', 'D0'])

varsK3pi = vu.create_aliases_for_selected(
    commonVariables, '^B0 -> [^D*- -> [^anti-D0 -> ^pi- ^K+ ^pi- ^pi+] ^pi-] ^mu+',
    ['B0', 'Dst', 'D0', 'pi1', 'K', 'pi2', 'pi3', 'pisoft', 'mu'])
varsK3pi += vu.create_aliases_for_selected(
    tracksVariables, 'B0 -> [D*- -> [anti-D0 -> ^pi- ^K+ ^pi- ^pi+] ^pi-] ^mu+',
    ['pi1', 'K', 'pi2', 'pi3', 'pisoft', 'mu'])
varsK3pi += vu.create_aliases_for_selected(
    compositeVariables, '^B0 -> [^D*- -> [^anti-D0 -> pi- K+ pi- pi+] pi-] mu+',
    ['B0', 'Dst', 'D0'])

# CMS variables of the D* only
cms_variables = []
for v in ['px', 'py', 'pz', 'p']:
    vm.addAlias(f'Dst_{v}_CMS', f'daughter(0,useCMSFrame({v}))')
    cms_variables.append(f'Dst_{v}_CMS')

# Angle between pi and K
vm.addAlias('Kpi_MCAngle', 'daughter(0,daughter(0,mcDaughterAngle(0,1)))')

varsKpi += cms_variables + eventWiseVariables + ['Kpi_MCAngle']
varsK3pi += cms_variables + eventWiseVariables

# Create one ntuple per channel in the same output file
ma.cutAndCopyList('B0:Kpi', 'B0:good', 'daughter(0,daughter(0,extraInfo(decayModeID)))==0', path=main)
ma.cutAndCopyList('B0:K3pi', 'B0:good', 'daughter(0,daughter(0,extraInfo(decayModeID)))==1', path=main)

ma.variablesToNtuple('B0:Kpi', varsKpi, filename=args.output, treename='Kpi', path=main)
ma.variablesToNtuple('B0:K3pi', varsK3pi, filename=args.output, treename='K3pi', path=main)

# MC particles lists for efficiency studies
ma.fillParticleListFromMC("B0:MCKpi", "", addDaughters=True, path=main)
ma.fillParticleListFromMC("B0:MCK3pi", "", addDaughters=True, path=main)
main.add_module(FilterByDecay("B0:MCKpi", Particle("B0", [
    Particle("D*+", [
        Particle("D0", [
            Particle("K+"),
            Particle("pi+")
        ]),
        Particle("pi+")
    ]),
    Particle("mu+"),
    Particle("nu_mu")
])))
main.add_module(FilterByDecay("B0:MCK3pi", Particle("B0", [
    Particle("D*+", [
        Particle("D0", [
            Particle("K+"),
            Particle("pi+"),
            Particle("pi+"),
            Particle("pi+")
        ]),
        Particle("pi+")
    ]),
    Particle("mu+"),
    Particle("nu_mu")
])))
varsKpiMC = [x for x in varsKpi if x.startswith("gen") or x.startswith("mc")]
varsK3piMC = [x for x in varsK3pi if x.startswith("gen") or x.startswith("mc")]
ma.variablesToNtuple('B0:MCKpi', varsKpiMC, filename=args.output, treename='MCKpi', path=main)
ma.variablesToNtuple('B0:MCK3pi', varsK3piMC, filename=args.output, treename='MCK3pi', path=main)

# Process the events
main.add_module('ProgressBar')
b2.process(main)
print(b2.statistics)
