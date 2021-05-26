/** Usage: root path/to/file.root mc_mass_fit.cc */
#include <TTree.h>
#include <TH1.h>
#include <TF1.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TPaveText.h>
#include <TMath.h>

void mc_mass_fit()
{
  gStyle->SetOptStat("ourme");
  gStyle->SetOptFit(0);
  TCanvas *c = new TCanvas;

  TTree *t = gDirectory->Get<TTree>("DstMC");
  t->Draw("M>>htemp(100,2.003,2.012)");
  auto h = (TH1F *)gPad->GetPrimitive("htemp");
  h->SetLineWidth(2);
  h->SetFillStyle(3144);
  h->SetFillColor(kBlue);

  // Breit-Wigner
  TF1 ff("ff", "[k]/(pow((pow(x,2)-pow([m],2)),2)+pow([m],2)*pow([#Gamma],2))", 1, 3);
  ff.SetParameter("k", 2e-5);
  ff.SetParameter("m", 2.01);
  ff.SetParameter("#Gamma", 7e-5);
  ff.SetParLimits(ff.GetParNumber("k"), 1e-5, 1e-3);
  ff.SetParLimits(ff.GetParNumber("m"), 2.009, 2.011);
  ff.SetParLimits(ff.GetParNumber("#Gamma"), 1e-5, 1e-3);
  h->Fit(&ff);

  TPaveText *pt = new TPaveText(0.15, 0.4, 0.5, 0.8, "brNDC");
  pt->AddText("f(x)=#frac{k}{(x^{2}-m^{2})^{2}+m^{2}#Gamma^{2}}");
  TString s;
  for (int i = 0; i < ff.GetNpar(); i++) {
    double p = ff.GetParameter(i), e = ff.GetParError(i);
    int oom = TMath::FloorNint(TMath::Log10(p));
    int ndig = oom - TMath::FloorNint(TMath::Log10(e)) + 1;
    if (ndig < 2) ndig = 2;
    if (oom) {
      double ef = TMath::Power(10, oom);
      p /= ef;
      e /= ef;
      s.Form("%s = (%.*lf #pm %.*lf)e%d", ff.GetParName(i), ndig, p, ndig, e, oom);
    } else // No exponent
      s.Form("%s = %.*lf #pm %.*lf", ff.GetParName(i), ndig, p, ndig, e);
    pt->AddText(s);
  }
  pt->SetBorderSize(0);
  pt->Draw();

  c->Print("mc_mass_fit.pdf");
}
