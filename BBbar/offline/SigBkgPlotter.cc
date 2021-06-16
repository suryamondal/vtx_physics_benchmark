#include "SigBkgPlotter.hh" // Own include
#include "Utils.hh"
#include "Constants.hh"
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TF1.h>
#include <TMath.h>
#include <TPaveText.h>
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

SigBkgPlotter::TRRes2D SigBkgPlotter::Histo2D(
  const char* vx, const char* vy, TString title,
  int xBins, double xLow, double xUp,
  int yBins, double yLow, double yUp)
{
  TRRes2D res = make_tuple(
    m_sig.Histo2D(ROOT::RDF::TH2DModel(
      m_namePrefix + "_sig_" + vx + "_" + vy, m_titlePrefix + " - " + title,
      xBins, xLow, xUp, yBins, yLow, yUp), vx, vy),
    m_bkg.Histo2D(ROOT::RDF::TH2DModel(
      m_namePrefix + "_bkg_" + vx + "_" + vy, m_titlePrefix + " - " + title,
      xBins, xLow, xUp, yBins, yLow, yUp), vx, vy)
  );
  m_h2s.push_back(res);
  return res;
}

void SigBkgPlotter::Histo2D(
  std::initializer_list<TString> particles,
  const char *vx, const char *vy, TString title,
  int xBins, double xLow, double xUp,
  int yBins, double yLow, double yUp)
{
  for (const TString& p : particles) {
    TString t = title; // Replacement happens in-place :(
    TString vx = p + "_" + vx , vy = p + "_" + vy;
    Histo2D(vx, vy, t.ReplaceAll("$p", ParticlesTitles.at(p)),
            xBins, xLow, xUp, yBins, yLow, yUp);
  }
}

void SigBkgPlotter::PrintAll(bool clearInternalList)
{
  for (const auto& t : m_h1s)
    DrawSigBkg(t);
  for (const auto& t : m_h2s)
    DrawSigBkg(t);
  if (clearInternalList) {
    m_h1s.clear();
    m_h2s.clear();
  }
}

void SigBkgPlotter::DrawSigBkg(TH1 *sig, TH1 *bkg)
{
  CHECK(sig);
  CHECK(bkg);
  bool is2d = sig->GetDimension() == 2;
  THStack s("ths", sig->GetTitle() + ";"TS + sig->GetXaxis()->GetTitle()
            + ";" + sig->GetYaxis()->GetTitle() + ";" + sig->GetZaxis()->GetTitle());
  SetColor(sig, kBlack, MyBlue);
  SetColor(bkg, is2d ? kBlack : MyRed, MyRed);
  bkg->SetFillStyle(is2d ? 1001 : 3454);

  if (!is2d && m_normalizeHistos) {
    Normalize(sig);
    Normalize(bkg);
  }

  s.Add(sig);
  s.Add(bkg);
  m_c->cd();
  s.Draw(is2d ? "" : "nostack");
  if (is2d) {
    s.GetHistogram()->GetXaxis()->SetTitleOffset(1.75);
    s.GetHistogram()->GetYaxis()->SetTitleOffset(2.0);
    s.GetHistogram()->GetZaxis()->SetTitleOffset(1.4);
  }

  TLegend leg(0.8, 0.8, 0.95, 0.91);
  leg.AddEntry(sig, "Signal", (m_normalizeHistos && !is2d) ? "PLE" : "F");
  leg.AddEntry(bkg, "Background", (m_normalizeHistos && !is2d) ? "PLE" : "F");
  leg.Draw();

  TPaveText oufSig(0.8, 0.68, 0.95, 0.79, "brNDC");
  TPaveText oufBkg(0.8, 0.56, 0.95, 0.67, "brNDC");
  if (!is2d && !m_normalizeHistos) {
    TString sf;
    double ovf = sig->GetBinContent(sig->GetNbinsX() + 1);
    double unf = sig->GetBinContent(0);
    double ent = sig->GetEntries();
    sf.Form("Overflow %.3lg (%.0lf%%)", ovf, ovf * 100.0 / ent);
    oufSig.AddText(sf);
    sf.Form("Underflow %.3lg (%.0lf%%)", unf, unf * 100.0 / ent);
    oufSig.AddText(sf);

    ovf = bkg->GetBinContent(bkg->GetNbinsX() + 1);
    unf = bkg->GetBinContent(0);
    ent = bkg->GetEntries();
    sf.Form("Overflow %.3lg (%.0lf%%)", ovf, ovf * 100.0 / ent);
    oufBkg.AddText(sf);
    sf.Form("Underflow %.3lg (%.0lf%%)", unf, unf * 100.0 / ent);
    oufBkg.AddText(sf);

    SetPaveStyle(oufSig, MyBlue);
    SetPaveStyle(oufBkg, MyRed);
    oufSig.Draw();
    oufBkg.Draw();
  }

  m_c->SetLogy(is2d ? 0 : 1);
  m_c->SetLogz(is2d ? 1 : 0);
  m_c.PrintPage(sig->GetTitle());
}

void SigBkgPlotter::FitAndPrint(
  TString name, const char* func, std::initializer_list<std::pair<TString,double>> p0,
  bool removeFromList)
{
  TH1* h = nullptr;
  name = m_namePrefix + "_sig_" + name;
  for (auto& t : m_h1s) {
    if (get<0>(t)->GetName() == name) {
      h = get<0>(t).GetPtr();
      break;
    }
  }
  CHECK(h);
  SetColor(h, kBlack, MyBlue);

  TF1 ff("ff", func, h->GetBinLowEdge(1), h->GetBinLowEdge(h->GetNbinsX() + 1), TF1::EAddToList::kNo);
  for (const auto& pp0 : p0)
    ff.SetParameter(pp0.first, pp0.second);
  ff.SetLineColor(MyRed);
  ff.SetLineWidth(2);

  m_c->cd();
  h->Draw();
  h->Fit(&ff, "Q");

  TLegend leg(0.8, 0.8, 0.95, 0.91);
  leg.AddEntry(h, "Signal", "F");
  leg.AddEntry(&ff, "Fit", "L");
  leg.Draw();

  TPaveText fr(0.8, 0.79 - 0.055 * (ff.GetNpar() + 1), 0.95, 0.79, "brNDC");
  TString sf;
  for (int i = 0; i < ff.GetNpar(); i++) {
    double p = ff.GetParameter(i), e = ff.GetParError(i);
    int poom = TMath::Max(TMath::FloorNint(TMath::Log10(TMath::Abs(p))), -10);
    int eoom = TMath::Max(TMath::FloorNint(TMath::Log10(TMath::Abs(e) / 2.0)), -10);
    int nf = TMath::Min(TMath::Max(poom - eoom, 3) + 1, 2);
    if (poom > 3 || poom < -3) {
      int oom = (poom / 3) * 3;
      p /= TMath::Power(10, oom);
      e /= TMath::Power(10, oom);
      int nd = TMath::Max(nf - poom + oom - 1, 0);
      sf.Form("%s = (%.*lf #pm %.*lf)e%d", ff.GetParName(i), nd, p, nd, e, oom);
    } else {
      int nd = TMath::Max(nf - poom - 1, 0);
      sf.Form("%s = %.*lf#pm%.*lf", ff.GetParName(i), nd, p, nd ,e);
    }
    fr.AddText(sf);
  }
  sf.Form("#chi^{2}/ndf = %.0lf/%d", ff.GetChisquare(), ff.GetNDF());
  fr.AddText(sf);
  SetPaveStyle(fr);
  fr.Draw();

  m_c->SetLogy();
  m_c.PrintPage(h->GetTitle());

  if (removeFromList) CHECK(false); // Not implemented!
}
