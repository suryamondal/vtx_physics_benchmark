#include "SigBkgPlotter.hh" // Own include
#include "Utils.hh"
#include "Constants.hh"
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
using namespace std;

SigBkgPlotter::TRRes1D SigBkgPlotter::Histo1D(
  const char* variable, TString title, int nBins, double xLow, double xUp)
{
  TRRes1D res = make_tuple(
    m_sig.Histo1D(
      {m_namePrefix + "_sig_" + variable, m_titlePrefix + " - " + title, nBins, xLow, xUp},
      variable),
    m_bkg.Histo1D(
      {m_namePrefix + "_bkg_" + variable, m_titlePrefix + " - " + title, nBins, xLow, xUp},
      variable)
  );
  m_h1s.push_back(res);
  return res;
}

void SigBkgPlotter::Histo1D(
  std::initializer_list<TString> particles, const char* variable,
  TString title, int nBins, double xLow, double xUp)
{
  for (const TString& p : particles) {
    TString t = title; // Replacement happens in-place :(
    TString v = p + "_" + variable;
    Histo1D(v, t.ReplaceAll("$p", ParticlesTitles.at(p)), nBins, xLow, xUp);
  }
}

void SigBkgPlotter::PrintAll(bool clearInternalList)
{
  for (const auto& t : m_h1s)
    DrawSigBkg(t);
  if (clearInternalList)
    m_h1s.clear();
}

void SigBkgPlotter::DrawSigBkg(TH1 *sig, TH1 *bkg)
{
  THStack s("ths", sig->GetTitle() + ";"TS + sig->GetXaxis()->GetTitle() + ";" + sig->GetYaxis()->GetTitle() + ";" + sig->GetZaxis()->GetTitle());
  SetColor(sig, kBlack, MyBlue);
  SetColor(bkg, MyRed);
  bkg->SetFillStyle(3454);

  if (m_normalizeHistos) {
    Normalize(sig);
    Normalize(bkg);
  }

  s.Add(sig);
  s.Add(bkg);
  m_c->cd();
  s.Draw("nostack");

  TLegend leg(0.8, 0.8, 0.95, 0.91);
  leg.AddEntry(sig, "Signal", "F");
  leg.AddEntry(bkg, "Background", "F");
  leg.Draw();

  m_c->SetLogy();
  m_c.PrintPage(sig->GetTitle());
}
