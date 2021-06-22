#include "SigBkgPlotter.hh" // Own include
#include "Utils.hh"
#include "Constants.hh"
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TF1.h>
#include <TMath.h>
#include <TPaveText.h>
#include <TGraph.h>
using namespace std;

SigBkgPlotter::TRRes1D SigBkgPlotter::Histo1D(
  const char* variable, TString title, int nBins, double xLow, double xUp)
{
  TString nameSig = GetUniqueName(m_namePrefix + "_sig_" + variable);
  TString nameBkg = GetUniqueName(m_namePrefix + "_bkg_" + variable);
  title = m_titlePrefix + " - " + title;
  TRRes1D res = make_tuple(
    m_sig.Histo1D({nameSig, title, nBins, xLow, xUp}, variable),
    m_bkg.Histo1D({nameBkg, title, nBins, xLow, xUp}, variable));
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
  TString nameSig = GetUniqueName(m_namePrefix + "_sig_" + vx + "_" + vy);
  TString nameBkg = GetUniqueName(m_namePrefix + "_bkg_" + vx + "_" + vy);
  title = m_titlePrefix + " - " + title;
  TRRes2D res = make_tuple(
    m_sig.Histo2D(ROOT::RDF::TH2DModel(
      nameSig, title, xBins, xLow, xUp, yBins, yLow, yUp), vx, vy),
    m_bkg.Histo2D(ROOT::RDF::TH2DModel(
      nameBkg, title, xBins, xLow, xUp, yBins, yLow, yUp), vx, vy)
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
    TString pvx = p + "_" + vx, pvy = p + "_" + vy;
    Histo2D(pvx, pvy, t.ReplaceAll("$p", ParticlesTitles.at(p)),
            xBins, xLow, xUp, yBins, yLow, yUp);
  }
}

void SigBkgPlotter::PrintAll(bool clearInternalList)
{
  if (m_normalizeHistos != m_histsAlreadyNormalized) {
    if (m_normalizeHistos) {
      for (auto &t : m_h1s) {
        Normalize(get<0>(t).GetPtr());
        Normalize(get<1>(t).GetPtr());
      }
    } else {
      for (auto &t : m_h1s) {
        Unnormalize(get<0>(t).GetPtr());
        Unnormalize(get<1>(t).GetPtr());
      }
    }
  }
  m_histsAlreadyNormalized = m_normalizeHistos;

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
  if (sig->GetDimension() == 2) {
    TString sigTitle = sig->GetTitle(), bkgTitle = bkg->GetTitle();
    sig->SetTitle(sig->GetTitle() + " - Signal"TS);
    bkg->SetTitle(bkg->GetTitle() + " - Background"TS);

    m_c->Clear();
    m_c->Divide(2);

    m_c->cd(1);
    gPad->SetRightMargin(0.16);
    sig->Draw("COLZ");
    gPad->SetLogy(0);
    gPad->SetLogz(m_logScale ? 1 : 0);
    TPaveText oufSig(0.5, 0.6, 0.8, 0.8, "brNDC");
    TString sf;
    TH2UO uo = Get2DHistUnderOverFlows(sig);
    sf.Form("%.0lf %.0lf %.0lf", uo.xuyo, uo.xiyo, uo.xoyo);
    oufSig.AddText(sf);
    sf.Form("%.0lf %.0lf %.0lf", uo.xuyi, uo.xiyi, uo.xoyi);
    oufSig.AddText(sf);
    sf.Form("%.0lf %.0lf %.0lf", uo.xuyu, uo.xiyu, uo.xoyu);
    oufSig.AddText(sf);
    SetPaveStyle(oufSig);
    oufSig.Draw();

    m_c->cd(2);
    gPad->SetRightMargin(0.16);
    bkg->Draw("COLZ");
    gPad->SetLogy(0);
    gPad->SetLogz(m_logScale ? 1 : 0);
    TPaveText oufBkg(0.5, 0.6, 0.8, 0.8, "brNDC");
    uo = Get2DHistUnderOverFlows(bkg);
    sf.Form("%.0lf %.0lf %.0lf", uo.xuyo, uo.xiyo, uo.xoyo);
    oufBkg.AddText(sf);
    sf.Form("%.0lf %.0lf %.0lf", uo.xuyi, uo.xiyi, uo.xoyi);
    oufBkg.AddText(sf);
    sf.Form("%.0lf %.0lf %.0lf", uo.xuyu, uo.xiyu, uo.xoyu);
    oufBkg.AddText(sf);
    SetPaveStyle(oufBkg);
    oufBkg.Draw();

    double hmax = TMath::Max(sig->GetMaximum(), bkg->GetMaximum());
    sig->SetMaximum(hmax);
    bkg->SetMaximum(hmax);

    m_c.PrintPage(sigTitle);
    m_c->Clear();
    sig->SetTitle(sigTitle);
    bkg->SetTitle(bkgTitle);
  } else {
    THStack s("ths", sig->GetTitle() + ";"TS + sig->GetXaxis()->GetTitle()
              + ";" + sig->GetYaxis()->GetTitle() + ";" + sig->GetZaxis()->GetTitle());
    SetColor(sig, kBlack, MyBlue);
    SetColor(bkg, MyRed);
    bkg->SetFillStyle(3454);

    s.Add(sig);
    s.Add(bkg);
    m_c->cd();
    s.Draw("nostack");

    TLegend leg(0.8, 0.8, 0.95, 0.91);
    leg.AddEntry(sig, "Signal", m_normalizeHistos ? "PLE" : "F");
    leg.AddEntry(bkg, "Background", m_normalizeHistos ? "PLE" : "F");
    leg.Draw();

    TPaveText oufSig(0.8, 0.68, 0.95, 0.79, "brNDC");
    TPaveText oufBkg(0.8, 0.56, 0.95, 0.67, "brNDC");
    if (!m_normalizeHistos) {
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

    m_c->SetLogy(m_logScale ? 1 : 0);
    m_c->SetLogz(0);
    m_c.PrintPage(sig->GetTitle());
  }
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
  leg.AddEntry(&ff, "Fit function", "L");
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

  if (m_logScale) m_c->SetLogy();
  m_c.PrintPage(h->GetTitle());

  h->GetListOfFunctions()->Delete();

  if (removeFromList) CHECK(false); // Not implemented!
}

void SigBkgPlotter::PrintROC(TString name, bool keepLow, bool removeFromList)
{
  TH1* sig = nullptr;
  TH1* bkg = nullptr;
  name = m_namePrefix + "_sig_" + name;
  for (auto& t : m_h1s) {
    if (get<0>(t)->GetName() == name) {
      sig = get<0>(t).GetPtr();
      bkg = get<1>(t).GetPtr();
      break;
    }
  }
  CHECKA(sig, name);
  CHECKA(bkg, name);

  // Build ROC curve
  vector<double> threshold, sigEff, bkgEff;
  const int nb = sig->GetNbinsX();
  threshold.reserve(nb);
  sigEff.reserve(nb);
  bkgEff.reserve(nb);
  const double sigInt = sig->Integral(0, nb + 1);
  const double bkgInt = bkg->Integral(0, nb + 1);
  int i1 = 0, i2 = nb + 1;
  for (int i = 1; i <= nb; i++) {
    threshold.push_back(sig->GetBinLowEdge(i)); // Cut on low edge of bin i
    if (keepLow)
      i2 = i - 1;
    else
      i1 = i;
    sigEff.push_back(sig->Integral(i1, i2) * 100.0 / sigInt);
    bkgEff.push_back(bkg->Integral(i1, i2) * 100.0 / bkgInt);
  }

  auto gSig = new TGraph(nb, threshold.data(), sigEff.data());
  gSig->SetName(GetUniqueName("gROCSig_" + name));
  gSig->SetTitle(sig->GetTitle() + " - ROC;"TS + sig->GetXaxis()->GetTitle() + ";Cut efficiency [%]");
  gSig->SetLineColor(MyBlue);
  gSig->SetLineWidth(2);
  auto gBkg = new TGraph(nb, threshold.data(), bkgEff.data());
  gBkg->SetName(GetUniqueName("gROCBkg_" + name));
  gBkg->SetTitle(bkg->GetTitle() + " - ROC;"TS + bkg->GetXaxis()->GetTitle() + ";Cut efficiency [%]");
  gBkg->SetLineColor(MyRed);
  gBkg->SetLineWidth(2);
  auto gROC = new TGraph(nb, sigEff.data(), bkgEff.data());
  gROC->SetName(GetUniqueName("gROC_" + name));
  gROC->SetTitle(sig->GetTitle() + " - ROC;Signal efficiency [%];Background efficiency [%]"TS);
  gROC->SetLineColor(kBlack);
  gROC->SetLineWidth(2);

  m_c->Clear();
  m_c->Divide(2);

  m_c->cd(1);
  gSig->SetMinimum(0);
  gSig->SetMaximum(100);
  gSig->Draw("AL");
  gBkg->Draw("L same");
  gPad->SetLeftMargin(0.16);
  gPad->SetLogy(0);
  gPad->SetGrid();
  TLegend leg(keepLow ? 0.2 : 0.7, 0.8, keepLow ? 0.45 : 0.95, 0.91);
  leg.AddEntry(gSig, "Signal", "L");
  leg.AddEntry(gBkg, "Background", "L");
  leg.Draw();

  m_c->cd(2);
  gROC->SetMinimum(0);
  gROC->SetMaximum(100);
  gROC->GetXaxis()->SetRangeUser(0, 100);
  gROC->Draw("AL");
  gPad->SetLeftMargin(0.16);
  gPad->SetLogy(0);
  gPad->SetGrid();

  m_c.PrintPage(sig->GetTitle() + " ROC"TS);
  m_c->Clear();

  if (removeFromList) CHECK(false); // Not implemented!
}

void SigBkgPlotter::PrintROC(
  std::initializer_list<TString> particles, TString name, bool keepLow, bool removeFromList)
{
  for (const TString& p : particles)
    PrintROC(p + "_" + name, keepLow, removeFromList);
}
