#include "SigBkgPlotter.hh" // Own include
#include "Utils.hh"
#include "Constants.hh"
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TF1.h>
#include <TMath.h>
#include <TLine.h>
#include <TPaveText.h>
#include <TGraph.h>
using namespace std;

SigBkgPlotter::TRRes1D SigBkgPlotter::Histo1D(
  const char* variable, TString title, int nBins, double xLow, double xUp, double scale)
{
  TString nameSig = GetUniqueName(m_namePrefix + "_sig_" + variable);
  TString nameBkg = GetUniqueName(m_namePrefix + "_bkg_" + variable);
  title = m_titlePrefix + " - " + title;
  if (title.CountChar(';') < 2) {
    for (int i = title.CountChar(';'); i < 2; i++) title += ";";
    title += "Candidates / bin";
  }

  TRRes1D res;
  if (scale == 1.0) {
    res = make_tuple(
      m_sig.Histo1D({nameSig, title, nBins, xLow, xUp}, variable),
      m_bkg.Histo1D({nameBkg, title, nBins, xLow, xUp}, variable));
  } else {
    TString expr = TString::Format("%s*%.18lg", variable, scale);
    res = make_tuple(
      m_sig.Define("h1dtmp", expr.Data()).Histo1D({nameSig, title, nBins, xLow, xUp}, "h1dtmp"),
      m_bkg.Define("h1dtmp", expr.Data()).Histo1D({nameBkg, title, nBins, xLow, xUp}, "h1dtmp"));
  }
  m_h1s.push_back(res);
  return res;
}

void SigBkgPlotter::Histo1D(
  std::initializer_list<TString> particles, const char* variable,
  TString title, int nBins, double xLow, double xUp, double scale)
{
  for (const TString& p : particles) {
    TString t = title; // Replacement happens in-place :(
    TString v = p + "_" + variable;
    Histo1D(v, t.ReplaceAll("$p", ParticlesTitles.at(p)), nBins, xLow, xUp, scale);
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

SigBkgPlotter::TRRes1D SigBkgPlotter::EffH1D(
  const char* variable, TString title, int nBins, double xLow, double xUp, double scale)
{
  TString nameSig = GetUniqueName(m_namePrefix + "_effSig_" + variable);
  TString nameMC = GetUniqueName(m_namePrefix + "_effMC_" + variable);
  title = m_titlePrefix + " - " + title;
  TString titleSig = title, titleMC = title;
  if (title.CountChar(';') < 2) {
    for (int i = title.CountChar(';'); i < 2; i++) { titleSig += ";"; titleMC += ";"; }
    titleSig += "Candidates / bin"; titleMC += "MC particles / bin";
  }

  TRRes1D res;
  if (scale == 1.0) {
    res = make_tuple(
      m_sig.Histo1D({nameSig, titleSig, nBins, xLow, xUp}, variable),
      m_mc.Histo1D({nameMC, titleMC, nBins, xLow, xUp}, variable));
  } else {
    TString expr = TString::Format("%s*%.18lg", variable, scale);
    res = make_tuple(
      m_sig.Define("h1dtmp", expr.Data()).Histo1D({nameSig, titleSig, nBins, xLow, xUp}, "h1dtmp"),
      m_mc.Define("h1dtmp", expr.Data()).Histo1D({nameMC, titleMC, nBins, xLow, xUp}, "h1dtmp"));
  }
  m_effh1s.push_back(res);
  return res;
}

void SigBkgPlotter::EffH1D(
  std::initializer_list<TString> particles, const char *variable,
  TString title, int nBins, double xLow, double xUp, double scale)
{
  for (const TString& p : particles) {
    TString t = title; // Replacement happens in-place :(
    TString v = p + "_" + variable;
    EffH1D(v, t.ReplaceAll("$p", ParticlesTitles.at(p)), nBins, xLow, xUp, scale);
  }
}

void SigBkgPlotter::PrintAll(bool clearInternalList, bool saveEff)
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
  for (const auto& t : m_effh1s)
    DrawEff(t, saveEff);
  if (clearInternalList) {
    m_h1s.clear();
    m_h2s.clear();
    m_effh1s.clear();
  }
}

void SigBkgPlotter::DrawSigBkg(TH1 *sig, TH1 *bkg)
{
  CHECK(sig);
  CHECK(bkg);
  if (sig->GetDimension() == 2) {
    TString sigTitle = sig->GetTitle(), bkgTitle = bkg->GetTitle();
    sig->SetTitle(sig->GetTitle() + " - Signal"TS);
    bkg->SetTitle(bkg->GetTitle() + " - Misreco"TS);

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

    // Scale down bkg if too high
    TString bkgLabel = "Misreco";
    int scale = 1;
    if (!m_normalizeHistos) {
      if (m_bkgDownScale == 1) {
        const double b2sr = bkg->GetBinContent(bkg->GetMaximumBin()) / sig->GetBinContent(sig->GetMaximumBin());
        if (b2sr > 1.8)
          scale = TMath::CeilNint(b2sr);
      } else if (m_bkgDownScale != 0) {
        scale = m_bkgDownScale;
      }
    }
    if (scale != 1) {
      bkg->Scale(1.0 / scale);
      bkgLabel.Form("Misreco#divide%d", scale);
    }

    s.Add(sig);
    s.Add(bkg);
    m_c->cd();
    s.Draw(m_normalizeHistos ? "nostack" : "nostack hist");

    TLegend leg(0.8, 0.8, 0.95, 0.91);
    leg.AddEntry(sig, "Signal", m_normalizeHistos ? "PLE" : "F");
    leg.AddEntry(bkg, bkgLabel, m_normalizeHistos ? "PLE" : "F");
    leg.Draw();

    TPaveText oufSig(0.8, 0.68, 0.95, 0.79, "brNDC");
    TPaveText oufBkg(0.8, 0.56, 0.95, 0.67, "brNDC");
    if (!m_normalizeHistos) {
      TString sf;
      double ovf = sig->GetBinContent(sig->GetNbinsX() + 1);
      double unf = sig->GetBinContent(0);
      double ent = sig->GetEntries();
      sf.Form("Overflow %.0lf (%.0lf%%)", ovf, ovf * 100.0 / ent);
      oufSig.AddText(sf);
      sf.Form("Underflow %.0lf (%.0lf%%)", unf, unf * 100.0 / ent);
      oufSig.AddText(sf);

      ovf = bkg->GetBinContent(bkg->GetNbinsX() + 1) * scale;
      unf = bkg->GetBinContent(0) * scale;
      ent = bkg->GetEntries();
      sf.Form("Overflow %.0lf (%.0lf%%)", ovf, ovf * 100.0 / ent);
      oufBkg.AddText(sf);
      sf.Form("Underflow %.0lf (%.0lf%%)", unf, unf * 100.0 / ent);
      oufBkg.AddText(sf);

      SetPaveStyle(oufSig, MyBlue);
      SetPaveStyle(oufBkg, MyRed);
      oufSig.Draw();
      oufBkg.Draw();
    }

    m_c->SetLogy(m_logScale ? 1 : 0);
    m_c->SetLogz(0);
    m_c.PrintPage(sig->GetTitle());

    bkg->Scale(scale); // Restore
  }
}

void SigBkgPlotter::DrawEff(TH1* sig, TH1* mc, bool save)
{
  CHECK(sig);
  CHECK(mc);
  TH1* eff = ComputeEfficiency(sig, mc);

  m_c->cd();
  eff->SetMinimum(0);
  eff->SetMaximum(1);
  eff->SetLineColor(kBlack);
  eff->SetLineWidth(2);
  eff->Draw();
  eff->GetYaxis()->SetTitle("Signal efficiency");

  TPaveText ouf(0.8, 0.58, 0.95, 0.91, "brNDC");
  const double ovfSig = sig->GetBinContent(sig->GetNbinsX() + 1);
  const double unfSig = sig->GetBinContent(0);
  const double entSig = sig->GetEntries();
  const double ovfMC = mc->GetBinContent(mc->GetNbinsX() + 1);
  const double unfMC = mc->GetBinContent(0);
  const double entMC = mc->GetEntries();
  ouf.AddText("Overflow");
  ouf.AddText(TString::Format("Sig. %.0lf (%.0lf%%)", ovfSig, ovfSig * 100.0 / entSig));
  ouf.AddText(TString::Format("MC %.0lf (%.0lf%%)", ovfMC, ovfMC * 100.0 / entMC));
  ouf.AddText("Underflow");
  ouf.AddText(TString::Format("Sig. %.0lf (%.0lf%%)", unfSig, unfSig * 100.0 / entSig));
  ouf.AddText(TString::Format("MC %.0lf (%.0lf%%)", unfMC, unfMC * 100.0 / entMC));
  SetPaveStyle(ouf);
  if (ovfSig > 0 || unfSig > 0 || ovfMC > 0 || unfMC > 0)
    ouf.Draw();

  m_c->SetGrid();
  m_c.PrintPage(mc->GetTitle());
  m_c->SetGrid(0, 0);

  if (save) eff->Write(sig->GetName());
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
  CHECKA(h, name);
  SetColor(h, kBlack, MyBlue);

  TF1 ff("ff", func, h->GetBinLowEdge(1), h->GetBinLowEdge(h->GetNbinsX() + 1), TF1::EAddToList::kNo);
  for (const auto& pp0 : p0)
    ff.SetParameter(pp0.first, pp0.second);
  ff.SetLineColor(MyRed);
  ff.SetLineWidth(2);
  ff.SetNpx(500);

  m_c->cd();
  h->Draw();
  h->Fit(&ff, "QI");

  TLegend leg(0.8, 0.8, 0.95, 0.91);
  leg.AddEntry(h, "Signal", "F");
  leg.AddEntry(&ff, "Fit function", "L");
  leg.Draw();

  TPaveText fr(0.77, 0.79 - 0.055 * (ff.GetNpar() + 1), 0.98, 0.79, "brNDC");
  TString sf;
  for (int i = 0; i < ff.GetNpar(); i++) {
    double p = ff.GetParameter(i), e = ff.GetParError(i);
    int poom = TMath::Max(TMath::FloorNint(TMath::Log10(TMath::Abs(p))), -10);
    int eoom = TMath::Max(TMath::FloorNint(TMath::Log10(TMath::Abs(e) / 2.0)), -10);
    int exp = TMath::Max(poom, eoom) / 3 * 3; if (exp < 0) exp += 3;
    int nf = TMath::Min(TMath::Max(exp - eoom, 0), 6);
    if (exp) {
      p /= TMath::Power(10, exp);
      e /= TMath::Power(10, exp);
      sf.Form("%s = (%.*lf #pm %.*lf)#times10^{%d}", ff.GetParName(i), nf, p, nf, e, exp);
    } else {
      sf.Form("%s = %.*lf#pm%.*lf", ff.GetParName(i), nf, p, nf ,e);
    }
    fr.AddText(sf);
  }
  sf.Form("#chi^{2}/NDF = %.0lf/%d", ff.GetChisquare(), ff.GetNDF());
  fr.AddText(sf);
  SetPaveStyle(fr);
  fr.Draw();

  if (m_logScale) m_c->SetLogy();
  m_c.PrintPage(h->GetTitle());

  h->GetListOfFunctions()->Delete();

  if (removeFromList) CHECK(false); // Not implemented!
}

void SigBkgPlotter::PrintROC(TString name, bool keepLow, bool excludeOUF, bool removeFromList)
{
  static double linePoints[2] = {0.0, 100.0};
  static TGraph gLine(2, linePoints, linePoints);

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
  int i1 = excludeOUF ? 1 : 0, i2 = excludeOUF ? nb : (nb + 1);
  const double sigInt = sig->Integral(i1, i2);
  const double bkgInt = bkg->Integral(i1, i2);
  for (int i = 1; i <= nb; i++) {
    threshold.push_back(sig->GetBinLowEdge(i)); // Cut on low edge of bin i
    if (keepLow)
      i2 = i - 1;
    else
      i1 = i;
    if (i2 < i1) {
      sigEff.push_back(0);
      bkgEff.push_back(0);
    } else {
      sigEff.push_back(sig->Integral(i1, i2) * 100.0 / sigInt);
      bkgEff.push_back(bkg->Integral(i1, i2) * 100.0 / bkgInt);
    }
  }

  TString lEff = excludeOUF ? " efficiency (excl. o/u-flows) [%]" : " efficiency [%]";
  auto gSig = new TGraph(nb, threshold.data(), sigEff.data());
  gSig->SetName(GetUniqueName("gROCSig_" + name));
  gSig->SetTitle(sig->GetTitle() + " - ROC;"TS + sig->GetXaxis()->GetTitle() + ";Cut" + lEff);
  gSig->SetLineColor(MyBlue);
  gSig->SetLineWidth(2);
  auto gBkg = new TGraph(nb, threshold.data(), bkgEff.data());
  gBkg->SetName(GetUniqueName("gROCBkg_" + name));
  gBkg->SetTitle(bkg->GetTitle() + " - ROC;"TS + bkg->GetXaxis()->GetTitle() + ";Cut" + lEff);
  gBkg->SetLineColor(MyRed);
  gBkg->SetLineWidth(2);
  auto gROC = new TGraph(nb, sigEff.data(), bkgEff.data());
  gROC->SetName(GetUniqueName("gROC_" + name));
  gROC->SetTitle(sig->GetTitle() + " - ROC;Signal"TS + lEff + ";Misreco" + lEff);
  gLine.SetTitle(sig->GetTitle() + " - ROC;Signal"TS + lEff + ";Misreco" + lEff);
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
  TLegend leg(keepLow ? 0.8 : 0.2, 0.2, keepLow ? 0.95 : 0.35, 0.3);
  leg.AddEntry(gSig, "Sig.", "L");
  leg.AddEntry(gBkg, "Bkg.", "L");
  leg.SetTextSize(0.042);
  leg.Draw();

  m_c->cd(2);
  gLine.SetMinimum(0);
  gLine.SetMaximum(100);
  gLine.SetLineColor(kRed);
  gLine.SetLineWidth(1);
  gLine.SetLineStyle(kDashed);
  gLine.Draw("AL");
  gLine.GetXaxis()->SetRangeUser(0, 100);
  gROC->Draw("L same");
  gPad->SetLeftMargin(0.16);
  gPad->SetLogy(0);
  gPad->SetGrid();

  m_c.PrintPage(sig->GetTitle() + " ROC"TS);
  m_c->Clear();

  if (removeFromList) CHECK(false); // Not implemented!
}

void SigBkgPlotter::PrintROC(
  std::initializer_list<TString> particles, TString name, bool keepLow,
  bool excludeOUF, bool removeFromList)
{
  for (const TString& p : particles)
    PrintROC(p + "_" + name, keepLow, excludeOUF, removeFromList);
}
