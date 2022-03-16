#ifndef GLOBALDEFS_H
#define GLOBALDEFS_H

#include "GlobalLibs.hh"

std::map<TString, TString> motherMap = {
  {"B0", "300553"},
  {"pisoft", "413"},
  {"K", "421"},
  {"mu", "511"}
};

std::vector<TString> commonMCBranch {"__experiment__", "__run__",
    "__event__"};

std::vector<TString> commonBranch {"__experiment__", "__run__",
    "__event__", "B0_M_rank",
    "Dst_M_preFit", "D0_M_preFit", "Dst_M_preFit",
    "mu_dr", "mu_dz", "K_dr", "K_dz", "pisoft_dr", "pisoft_dz",
    "mu_nVXDHits", "K_nVXDHits", "pisoft_nVXDHits", "Dst_p_CMS"};
std::vector<TString> KpiBranch {"pi_dr", "pi_dz", "pi_nVXDHits"};
std::vector<TString> K3piBranch {"pi1_dr", "pi1_dz",
    "pi1_nVXDHits", "pi2_dr", "pi2_dz", "pi2_nVXDHits",
    "pi3_dr", "pi3_dz", "pi3_nVXDHits"};
std::vector<TString> extraBranch {"isSignal", "mdstIndex",
    "particleSource"};

const TString CommonCuts =
  "TMath::Abs(Dst_M_preFit-2.01026) < 0.1"
  "&& TMath::Abs(D0_M_preFit-1.86484) < 0.1"
  "&& TMath::Abs(Dst_M_preFit-D0_M_preFit-0.145426) < 0.005" // Lower cut is not in steering file (for now)
  "&& mu_dr < 2 && TMath::Abs(mu_dz) < 2"
  "&& K_dr < 2 && TMath::Abs(K_dz) < 2"
  "&& pisoft_dr < 2 && TMath::Abs(pisoft_dz) < 2"
  "&& mu_nVXDHits > 0 && K_nVXDHits > 0 && pisoft_nVXDHits > 0" // VXD = PXD+SVD+VTX 
  "&& Dst_p_CMS < 2.5"; // From momentum conservation
  
const TString KpiCuts =
  "pi_dr < 2 && TMath::Abs(pi_dz) < 2"
  "&& pi_nVXDHits > 0";
  
const TString K3piCuts =
  "pi1_dr < 2 && TMath::Abs(pi1_dz) < 2 && pi1_nVXDHits > 0"
  "&& pi2_dr < 2 && TMath::Abs(pi2_dz) < 2 && pi2_nVXDHits > 0"
  "&& pi3_dr < 2 && TMath::Abs(pi3_dz) < 2 && pi3_nVXDHits > 0";

#endif
