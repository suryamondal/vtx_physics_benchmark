#pragma once
#include <TColor.h>
#include <TString.h>
#include <map>

const auto MyRed = TColor::GetColor("#E24A33");
const auto MyBlue = TColor::GetColor("#348ABD");

const std::map<TString,TString> ParticlesTitles {
  {"Dst", "D*"}, {"D0", "D^{0}"}, {"K", "K"}, {"pisoft", "#pi_{soft}"},
  {"pi", "#pi"}, {"pi1", "#pi_{1}"}, {"pi3", "#pi_{3}"}, {"pi3", "#pi_{3}"}
};
