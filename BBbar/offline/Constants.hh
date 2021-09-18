#pragma once
#include <TColor.h>
#include <TString.h>
#include <map>

const int NThreads = 8;

const auto MyRed = TColor::GetColor("#E24A33");
const auto MyBlue = TColor::GetColor("#348ABD");

const std::map<TString,TString> ParticlesTitles {
  {"B0", "B^{0}"}, {"Dst", "D*"}, {"D0", "D^{0}"},
  {"mu", "#mu"}, {"K", "K"}, {"pisoft", "#pi_{soft}"},
  {"pi", "#pi"}, {"pi1", "#pi_{1}"}, {"pi2", "#pi_{2}"}, {"pi3", "#pi_{3}"}
};

/// Condition that tells signal from mis-reconstructed / bkg
extern const TString SignalCondition;

/// Offline cuts common to Kpi and K3pi
extern const TString CommonCuts;

/// Offline cuts for Kpi only
extern const TString KpiCuts;

/// Offline cuts for K3pi only
extern const TString K3piCuts;

// Composite particles (D* and D0)
const std::initializer_list<TString> CompositeParticles {"B0", "Dst", "D0"};

// Final state particles of Kpi
const std::initializer_list<TString> KPiFSParticles {"mu", "K", "pi", "pisoft"};

// Final state particles of K3pi
const std::initializer_list<TString> K3PiFSParticles {"mu", "K", "pi1", "pi2", "pi3", "pisoft"};

// Final state hard particles of Kpi
const std::initializer_list<TString> KPiFSHParticles {"mu", "K", "pi"};

// Final state hard particles of K3pi
const std::initializer_list<TString> K3PiFSHParticles {"mu", "K", "pi1", "pi2", "pi3"};

// Final state pions of Kpi
const std::initializer_list<TString> KPiPions {"pi", "pisoft"};

// Final state pions of K3pi
const std::initializer_list<TString> K3PiPions {"pi1", "pi2", "pi3", "pisoft"};

// All particles of Kpi
const std::initializer_list<TString> KPiAllParticles {
  "B0", "Dst", "D0", "mu",  "K", "pi", "pisoft"};

// All particles of K3pi
const std::initializer_list<TString> K3PiAllParticles {
  "B0", "Dst", "D0", "mu",  "K", "pi1", "pi2", "pi3", "pisoft"};
