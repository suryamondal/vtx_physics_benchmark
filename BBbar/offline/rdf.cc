#include <TDirectory.h>
#include <TTree.h>
#include <TString.h>
#include <TCanvas.h>
#include <TH1.h>
#include <THStack.h>
#include <TStyle.h>
#include <TColor.h>
#include <TMath.h>
#include <TLegend.h>
#include <ROOT/RDataFrame.hxx>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>
#include <tuple>
using namespace ROOT;
using namespace std;

#define STRNG(x) #x
#define STRNG2(x) STRNG(x)
#define CHECK(assertion) if(!(assertion)) throw runtime_error("At " STRNG2(__FILE__) ":" STRNG2(__LINE__))

#define PLOTS_FILE "plots.pdf"

typedef ROOT::RDF::RResultPtr<TH1D> TH1DRes;

const auto mRed = TColor::GetColor("#E24A33");
const auto mBlue = TColor::GetColor("#348ABD");

TString operator"" TS(const char* s, size_t) { return TString(s); }

inline void SetColor(TH1* h, Color_t c)
{
  h->SetLineColor(c);
  h->SetMarkerColor(c);
  h->SetFillColor(c);
}

void drawSigBkg(TCanvas& c, TH1* sig, TH1* bkg)
{
  THStack s("ths", sig->GetTitle() + ";"TS + sig->GetXaxis()->GetTitle() + ";" + sig->GetYaxis()->GetTitle() + ";" + sig->GetZaxis()->GetTitle());
  SetColor(sig, mBlue);
  SetColor(bkg, mRed);
  bkg->SetFillStyle(3454);

  s.Add(bkg); s.Add(sig); // TODO swap these two when bkg is < sig
  s.Draw("nostack");

  TLegend leg(0.75, 0.8, 0.95, 0.91);
  leg.AddEntry(sig, "Dst_isSignal = 1", "F");
  leg.AddEntry(bkg, "Dst_isSignal = 0", "F");
  leg.Draw();

  c.SetLogy();
  c.Print(PLOTS_FILE, "Title:"TS + sig->GetTitle());
}

inline void drawSigBkg(TCanvas &c, ROOT::RDF::RResultPtr<TH1D>& sig, ROOT::RDF::RResultPtr<TH1D>& bkg) {
  drawSigBkg(c, (TH1*)sig.GetPtr(), (TH1*)bkg.GetPtr());
}

void plots(RDataFrame& df, TCanvas& c, TString name, TString title)
{
  auto df2 = df.Define("massDiffPreFit", "Dst_M_preFit-D0_M_preFit");
  auto sig = df2.Filter("Dst_isSignal==1", "SignalFilter");
  auto bkg = df2.Filter("Dst_isSignal==0", "BackgroundFilter");
  c.cd();

  // auto [sigHisto, bkgHist] = histoSigBkg(title, bins, xlow, xup, col)
  function<tuple<TH1DRes,TH1DRes>(const char*,TString,int,double,double)>
  histoSigBkg = [&sig, &bkg, &name, &title] (const char* v, TString tt, int nb, double xl, double xu) {
    return make_tuple(
      sig.Histo1D({name + "_sig_" + v, title + " - " + tt, nb, xl, xu}, v),
      bkg.Histo1D({name + "_bkg_" + v, title + " - " + tt, nb, xl, xu}, v)
    );
  };

  // Only booking here
  vector<tuple<TH1DRes,TH1DRes>> histos;

  histos.push_back(histoSigBkg("Dst_M", "M_{D*};M_{D*} [GeV];Events / bin", 100, 1, 3));
  histos.push_back(histoSigBkg("D0_M", "M_{D^{0}};M_{D^{0}} [GeV];Events / bin", 100, 1, 3));

  histos.push_back(histoSigBkg("Dst_M_preFit", "M_{D*} (pre-fit);M_{D*} [GeV];Events / bin", 100, 1, 3));
  histos.push_back(histoSigBkg("D0_M_preFit", "M_{D^{0}} (pre-fit);M_{D^{0}} [GeV];Events / bin", 100, 1, 3));

  histos.push_back(histoSigBkg("massDiffPreFit", "Mass difference (pre-fit);M_{D*} - M_{D^{0}} [GeV];Events / bin", 100, 0, 0.4));

  // TODO PXD, SVD, VTX Hits, repeat for pi, pi1, pi2, pi3, pisoft, K
  // TODO Need to add ifs for VTX and pi/pi123
  // auto sigNCDCHits = sig.Histo1D({name + "nCDCHits", title + "N. CDC Hits;CDC Hits;Events / bin", 100, -0.5, 99}, "");

  // TODO same as above for z0, d0

  // Processing happens here
  df.Report()->Print();
  for (auto& [hSig, hBkg] : histos)
    drawSigBkg(c, hSig, hBkg);
}

void rdf()
{
  EnableImplicitMT(8);
  TTree* tKpi = gDirectory->Get<TTree>("Dst_D0pi_Kpi");
  TTree* tK3pi = gDirectory->Get<TTree>("Dst_D0pi_K3pi");
  CHECK(tKpi);
  CHECK(tK3pi);
  RDataFrame dfKpi(*tKpi);
  RDataFrame dfK3pi(*tK3pi);

  gStyle->SetOptStat(0);
  TCanvas c("c", "c", 640, 480);
  c.Print(PLOTS_FILE "[");

  plots(dfKpi, c, "Kpi", "K#pi");
  plots(dfK3pi, c, "K3pi", "K#pi#pi#pi");

  c.Print(PLOTS_FILE "]");
}
