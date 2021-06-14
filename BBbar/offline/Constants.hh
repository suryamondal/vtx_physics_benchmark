#pragma once
#include <TColor.h>
#include <TString.h>
#include <map>

const auto MyRed = TColor::GetColor("#E24A33");
const auto MyBlue = TColor::GetColor("#348ABD");

const std::map<TString,TString> ParticlesTitles {
  {"Dst", "D*"}, {"D0", "D^{0}"}, {"K", "K"}, {"pisoft", "#pi_{soft}"},
  {"pi", "#pi"}, {"pi1", "#pi_{1}"}, {"pi2", "#pi_{2}"}, {"pi3", "#pi_{3}"}
};

/// Offline cuts common to Kpi and K3pi
extern const TString CommonCuts;

/// Offline cuts for Kpi only
extern const TString KpiCuts;

/// Offline cuts for K3pi only
extern const TString K3piCuts;

// Final state particles of Kpi
const std::initializer_list<TString> KPiFSParticles {"K", "pi", "pisoft"};

// Final state particles of K3pi
const std::initializer_list<TString> K3PiFSParticles {"K", "pi1", "pi2", "pi3", "pisoft"};
