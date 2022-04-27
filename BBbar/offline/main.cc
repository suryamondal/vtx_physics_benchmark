#include "SigBkgPlotter.hh"
#include "PDFCanvas.hh"
#include "Utils.hh"
#include "ArgParser.hh"
#include "CutEfficiency.hh"
#include "Constants.hh"
#include <TString.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <ROOT/RDataFrame.hxx>
#include <TFile.h>
#include <iostream>
using namespace std;
using namespace ROOT;

/** For each particle and for each {aVar, bVar, newVar} tuple, this
 * function defines a new variable
 * "particle_newVar = f(particle_aVar,particle_bVar)"
 * where f is obtained by replacing "$a" and "$b" in exprTemplate
 * (which is "$a - $b" by default).
 */
SigBkgPlotter::DefineDF defineVarsForParticles(
  SigBkgPlotter::DefineDF source, initializer_list<TString> particles,
  initializer_list<TString> aVars, initializer_list<TString> bVars,
  initializer_list<TString> newVars, TString exprTemplate = "$a - $b")
{
  auto df = source;
  for (const TString& p : particles) {
    for (auto av = aVars.begin(), bv = bVars.begin(), nv = newVars.begin();
         av != aVars.end() && bv != bVars.end() && nv != newVars.end();
         av++, bv++, nv++) {
      TString pav = p + "_" + *av, pbv = p + "_" + *bv, pnv = p + "_" + *nv;
      TString expr = exprTemplate;
      expr = expr.ReplaceAll("$a", pav).ReplaceAll("$b", pbv);
      df = df.Define(pnv.Data(), expr.Data());
      // cout << pnv << " = " << expr << endl;
    }
  }
  return df;
}

/** Define expression variables. */
SigBkgPlotter::DefineDF defineVariables(RDataFrame& df, bool isK3pi)
{
  const auto& CompParts = CompositeParticles;
  const auto& FSParts = isK3pi ? K3PiFSParticles : KPiFSParticles;

  auto ddf = defineVarsForParticles(df, CompParts,
    {"mcDecayVertexX", "mcDecayVertexY", "mcDecayVertexZ"},
    {"x",              "y",              "z"},
    {"residualDecayX", "residualDecayY", "residualDecayZ"});
  ddf = defineVarsForParticles(ddf, CompParts,
    {"residualDecayX", "residualDecayY", "residualDecayZ"},
    {"x_uncertainty",  "y_uncertainty",  "z_uncertainty"},
    {"pullDecayX", "pullDecayY", "pullDecayZ"},
    "$a / $b");
  ddf = defineVarsForParticles(ddf, {"D0"},
    {"mcFlightDistance",       "mcFlightTime"},
    {"flightDistance",         "flightTime"},
    {"residualFlightDistance", "residualFlightTime"});
  ddf = defineVarsForParticles(ddf, {"D0"},
    {"residualFlightDistance", "residualFlightTime"},
    {"flightDistanceErr",      "flightTimeErr"},
    {"pullFlightDistance",     "pullFlightTime"},
    "$a / $b");
  ddf = defineVarsForParticles(ddf, FSParts,
    {"d0Err",      "z0Err"},
    {"d0Pull",     "z0Pull"},
    {"d0Residual", "z0Residual"},
    "$a * $b"); // Residuals from pulls, not straightforward but works
  ddf = defineVarsForParticles(ddf, FSParts,
    {"mcPT",       "mcP",        "mcTheta",       "mcPhi"},
    {"pt",         "p",          "theta",         "phi"},
    {"ptResidual", "pResidual",  "thetaResidual", "phiResidual"});
  ddf = ddf.Define("B0_M_rank_percent", "(B0_M_rank - 1) * 100 / (__ncandidates__ == 1 ? 1 : __ncandidates__ - 1)")
           .Define("B0_chiProb_rank_percent", "(B0_chiProb_rank - 1) * 100 / (__ncandidates__ == 1 ? 1 : __ncandidates__ - 1)")
           .Define("Dst_dM_rank_percent", "(Dst_dM_rank - 1) * 100 / (__ncandidates__ == 1 ? 1 : __ncandidates__ - 1)")
           .Define("D0_dM_rank_percent", "(D0_dM_rank - 1) * 100 / (__ncandidates__ == 1 ? 1 : __ncandidates__ - 1)");
  for (const TString& p : FSParts)
    ddf = ddf.Alias(
      (p + "_firstVXDLayer").Data(),
      ddf.HasColumn((p + "_firstVTXLayer").Data()) ? (p + "_firstVTXLayer").Data()
                                                   : (p + "_firstPXDLayer").Data()
    );

  // Define variables for piH and piL, which are pi1 and pi2 sorted by pT
  if (isK3pi) {
    for (const auto& sCol : df.GetColumnNames()) {
      TString col = sCol;
      if (!col.BeginsWith("pi1_")) continue;
      col = col(4, col.Length() - 4);
      TString newName, newExpr;
      newName.Form("piH_%s", col.Data());
      newExpr.Form("pi1_pt < pi2_pt ? pi2_%s : pi1_%s", col.Data(), col.Data());
      // cout << newName << " = " << newExpr << endl;
      ddf = ddf.Define(newName.Data(), newExpr.Data());
      newName.Form("piL_%s", col.Data());
      newExpr.Form("pi1_pt < pi2_pt ? pi1_%s : pi2_%s", col.Data(), col.Data());
      // cout << newName << " = " << newExpr << endl;
      ddf = ddf.Define(newName.Data(), newExpr.Data());
    }
    ddf = defineVarsForParticles(ddf, {"piH", "piL"},
      {"d0Err",      "z0Err"},
      {"d0Pull",     "z0Pull"},
      {"d0Residual", "z0Residual"},
      "$a * $b"); // Residuals from pulls, not straightforward but works
    ddf = defineVarsForParticles(ddf, {"piH", "piL"},
      {"mcPT",       "mcP",        "mcTheta",       "mcPhi"},
      {"pt",         "p",          "theta",         "phi"},
      {"ptResidual", "pResidual",  "thetaResidual", "phiResidual"});
  }

  return ddf.Define("massDiffPreFit", "Dst_M_preFit-D0_M_preFit")
            .Define("massDiff", "Dst_M-D0_M");
}

SigBkgPlotter::FilterDF applyOfflineCuts(SigBkgPlotter::DefineDF& df, bool isK3pi)
{
  TString cuts = CommonCuts + " && " + (isK3pi ? K3piCuts : KpiCuts);
  return df.Filter(cuts.Data(), "Offline Cuts");
}

/** Books plots. */
void bookHistos(SigBkgPlotter& plt, bool isK3pi)
{
  const auto& CompParts = CompositeParticles;
  const auto& FSParts = isK3pi ? K3PiFSParticlesSorted : KPiFSParticles;
  const auto& FSHParts = isK3pi ? K3PiFSHParticlesSorted : KPiFSHParticles;
  // const auto& AllParts = isK3pi ? K3PiAllParticlesSorted : KPiAllParticles;
  // const auto& Pions = isK3pi ? K3PiPionsSorted : KPiPions;

  // ==== Masses (and cuts variables)
  // Pre-fit
  plt.Histo1D({"B0"}, "M_preFit", "M_{$p} (pre-fit);M_{$p} [GeV/c^{2}]", 100, 1.9, 5.5);
  plt.Histo1D({"Dst"}, "M_preFit", "M_{$p} (pre-fit);M_{$p} [GeV/c^{2}]", 110, 1.9, 2.12);
  plt.Histo1D({"D0"}, "M_preFit", "M_{$p} (pre-fit);M_{$p} [GeV/c^{2}]", 110, 1.75, 1.97);
  plt.Histo1D("massDiffPreFit", "#DeltaM (pre-fit);M_{D*} - M_{D^{0}} [GeV/c^{2}]", 110, 0.14, 0.151);
  // Post-fit
  plt.Histo1D({"B0"}, "M", "M_{$p};M_{$p} [GeV/c^{2}]", 100, 1.9, 5.5);
  plt.Histo1D({"Dst"}, "M", "M_{$p};M_{$p} [GeV/c^{2}]", 110, 1.9, 2.12);
  plt.Histo1D({"D0"}, "M", "M_{$p};M_{$p} [GeV/c^{2}]", 110, 1.75, 1.97);
  plt.Histo1D("massDiff", "#DeltaM;M_{D*} - M_{D^{0}} [GeV/c^{2}]", 110, 0.14, 0.151);
  plt.Histo1D("Dst_p_CMS", "p_{CM,D*};p^{CM}_{D*} [GeV/c]", 120, 0, 3);

  // ==== Vertices
  plt.Histo1D({"D0"}, "flightDistance", "Flight distance of $p;$p flight distance [cm]", 100, -0.05, 0.1);
  // Residuals
  plt.Histo1D(CompParts, "residualDecayX", "x_{decay,$p} residual;MC - meas [#mum]", 100, -250, 250, 1e4);
  plt.Histo1D(CompParts, "residualDecayY", "y_{decay,$p} residual;MC - meas [#mum]", 100, -250, 250, 1e4);
  plt.Histo1D(CompParts, "residualDecayZ", "z_{decay,$p} residual;MC - meas [#mum]", 100, -250, 250, 1e4);
  plt.Histo1D({"D0"}, "residualFlightDistance", "Residual of flight distance of $p;$p MC - meas [#mum]", 100, -500, 500, 1e4);
  plt.Histo1D(CompParts, "residualDecayX", "x_{decay,$p} residual;MC - meas [#mum]", 500, -250, 250, 1e4, true);
  plt.Histo1D(CompParts, "residualDecayY", "y_{decay,$p} residual;MC - meas [#mum]", 500, -250, 250, 1e4, true);
  plt.Histo1D(CompParts, "residualDecayZ", "z_{decay,$p} residual;MC - meas [#mum]", 500, -250, 250, 1e4, true);
  plt.Histo1D({"D0"}, "residualFlightDistance", "Residual of flight distance of $p;$p MC - meas [#mum]", 500, -500, 500, 1e4, true);
  // Pulls
  plt.Histo1D(CompParts, "pullDecayX", "x_{decay,$p} pull;(MC - meas) / #sigma_{meas}", 100, -10, 10);
  plt.Histo1D(CompParts, "pullDecayY", "y_{decay,$p} pull;(MC - meas) / #sigma_{meas}", 100, -10, 10);
  plt.Histo1D(CompParts, "pullDecayZ", "z_{decay,$p} pull;(MC - meas) / #sigma_{meas}", 100, -10, 10);
  plt.Histo1D({"D0"}, "pullFlightDistance", "Pull of flight distance of $p;(MC - meas) / #sigma_{meas}", 100, -10, 10);

  // ==== Impact parameters
  plt.Histo1D(FSHParts, "d0", "d_{0$p};d_{0,$p} [mm]", 100, -2, 2, 10);
  plt.Histo1D({"pisoft"}, "d0", "d_{0$p};d_{0,$p} [mm]", 100, -4, 4, 10);
  plt.Histo1D(FSHParts, "z0", "z_{0$p};z_{0,$p} [mm]", 100, -2, 2, 10);
  plt.Histo1D({"pisoft"}, "z0", "z_{0$p};z_{0,$p} [mm]", 100, -4, 4, 10);
  // Residuals
  plt.Histo1D(FSHParts, "d0Residual", "d_{0$p} residual;MC - meas [#mum]", 100, -200, 200, 1e4);
  plt.Histo1D({"pisoft"}, "d0Residual", "d_{0$p} residual;MC - meas [#mum]", 100, -2000, 2000, 1e4);
  plt.Histo1D(FSHParts, "z0Residual", "z_{0$p} residual;MC - meas [#mum]", 100, -200, 200, 1e4);
  plt.Histo1D({"pisoft"}, "z0Residual", "z_{0$p} residual;MC - meas [#mum]", 100, -4000, 4000, 1e4);
  plt.Histo1D(FSHParts, "d0Residual", "d_{0$p} residual;MC - meas [#mum]", 500, -250, 250, 1e4, true);
  plt.Histo1D({"pisoft"}, "d0Residual", "d_{0$p} residual;MC - meas [#mum]", 500, -2500, 2500, 1e4, true);
  plt.Histo1D(FSHParts, "z0Residual", "z_{0$p} residual;MC - meas [#mum]", 500, -250, 250, 1e4, true);
  plt.Histo1D({"pisoft"}, "z0Residual", "z_{0$p} residual;MC - meas [#mum]", 500, -5000, 5000, 1e4, true);
  // fit parameters : 2D
  plt.Histo2D(FSHParts,   "mcPT",    "d0Residual", "d0_{$p} residual;pt_{$p};MC - meas", 25, 0., 2.5,  300, -0.06, 0.06);
  plt.Histo2D({"pisoft"}, "mcPT",    "d0Residual", "d0_{$p} residual;pt_{$p};MC - meas", 25, 0., 0.25, 500, -1., 1.);
  plt.Histo2D(FSHParts,   "mcPT",    "z0Residual", "z0_{$p} residual;pt_{$p};MC - meas", 25, 0., 2.5,  300, -0.06, 0.06);
  plt.Histo2D({"pisoft"}, "mcPT",    "z0Residual", "z0_{$p} residual;pt_{$p};MC - meas", 25, 0., 0.25, 500, -1., 1.);
  // Pulls
  plt.Histo1D(FSParts, "d0Pull", "d_{0$p} pull;(MC - meas) / #sigma_{meas}", 100, -10, 10);
  plt.Histo1D(FSParts, "z0Pull", "z_{0$p} pull;(MC - meas) / #sigma_{meas}", 100, -10, 10);
  plt.Histo1D(FSParts, "d0Pull", "d_{0$p} pull;(MC - meas) / #sigma_{meas}", 500, -10, 10, 1, true);
  plt.Histo1D(FSParts, "z0Pull", "z_{0$p} pull;(MC - meas) / #sigma_{meas}", 500, -10, 10, 1, true);

  // fit parameters
  plt.Histo1D(FSHParts,   "ptResidual", "pt_{$p} residual;MC - meas [MeV]", 100, -0.025, 0.025);
  plt.Histo1D({"pisoft"}, "ptResidual", "pt_{$p} residual;MC - meas [MeV]", 100, -0.025, 0.025);
  plt.Histo1D(FSHParts,   "pResidual", "p_{$p} residual;MC - meas [MeV]",   100, -0.025, 0.025);
  plt.Histo1D({"pisoft"}, "pResidual", "p_{$p} residual;MC - meas [MeV]",   100, -0.025, 0.025);
  plt.Histo1D(FSHParts,   "thetaResidual", "theta_{$p} residual;MC - meas", 100, -0.005, 0.005);
  plt.Histo1D({"pisoft"}, "thetaResidual", "theta_{$p} residual;MC - meas", 100, -0.01, 0.01);
  plt.Histo1D(FSHParts,   "phiResidual", "phi_{$p} residual;MC - meas",     100, -0.005, 0.005);
  plt.Histo1D({"pisoft"}, "phiResidual", "phi_{$p} residual;MC - meas",     100, -0.01, 0.01);
  // fit parameters : 2D
  plt.Histo2D(FSHParts,   "mcPT",    "ptResidual",    "pt_{$p} residual;pt_{$p};MC - meas", 25, 0., 2.5, 100, -0.015, 0.015);
  plt.Histo2D({"pisoft"}, "mcPT",    "ptResidual",    "pt_{$p} residual;pt_{$p};MC - meas", 25, 0., 0.25, 100, -0.015, 0.015);
  plt.Histo2D(FSHParts,   "mcP",     "pResidual",     "p_{$p} residual;p_{$p};MC - meas",   25, 0., 2.5, 100, -0.015, 0.015);
  plt.Histo2D({"pisoft"}, "mcP",     "pResidual",     "p_{$p} residual;p_{$p};MC - meas",   25, 0., 0.25, 100, -0.015, 0.015);
  plt.Histo2D(FSHParts,   "mcTheta", "thetaResidual", "theta_{$p} residual;theta_{$p};MC - meas", 25, 0., TMath::Pi(), 100, -0.0025, 0.0025);
  plt.Histo2D({"pisoft"}, "mcTheta", "thetaResidual", "theta_{$p} residual;theta_{$p};MC - meas", 25, 0., TMath::Pi(), 100, -0.01, 0.01);
  plt.Histo2D(FSHParts,   "mcPhi",   "phiResidual",   "phi_{$p} residual;phi_{$p};MC - meas",     25, -TMath::Pi(), TMath::Pi(), 100, -0.0025, 0.0025);
  plt.Histo2D({"pisoft"}, "mcPhi",   "phiResidual",   "phi_{$p} residual;phi_{$p};MC - meas",     25, -TMath::Pi(), TMath::Pi(), 100, -0.01, 0.01);

  // ==== Efficiency
  // Best-candidates selection/ranking
  plt.Histo1D({"B0"}, "M_rank", "Rank by max M_{$p};Rank", 30, 0.5, 30.5);
  plt.Histo1D({"B0"}, "M_rank_percent", "Rank by max M_{$p};Rank [%]", 101, -0.5, 100.5);
  plt.Histo1D({"B0"}, "chiProb_rank", "Rank by max #chi^{2}_{$p};Rank", 30, 0.5, 30.5);
  plt.Histo1D({"B0"}, "chiProb_rank_percent", "Rank by max #chi^{2}_{$p};Rank [%]", 101, -0.5, 100.5);
  plt.Histo1D({"Dst", "D0"}, "dM_rank", "Rank by min |#deltaM_{$p}|;Rank", 30, 0.5, 30.5);
  plt.Histo1D({"Dst", "D0"}, "dM_rank_percent", "Rank by min |#deltaM_{$p}|;Rank [%]", 101, -0.5, 100.5);
  // p
  plt.EffH1D({"B0"}, "mcP", "Efficiency vs true p_{$p};True p_{$p} [GeV/c]", 20, 1, 2);
  plt.EffH1D({"Dst", "D0"}, "mcP", "Efficiency vs true p_{$p};True p_{$p} [GeV/c]", 20, 0, 3.5);
  plt.EffH1D({"pisoft"}, "mcP", "Efficiency vs true p_{$p};True p_{$p} [GeV/c]", 20, 0, 0.35);
  // pT
  plt.EffH1D({"B0"}, "mcPT", "Efficiency vs true p_{T,$p};True p_{T,$p} [GeV/c]", 20, 0, 0.7);
  plt.EffH1D({"Dst", "D0"}, "mcPT", "Efficiency vs true p_{T,$p};True p_{T,$p} [GeV/c]", 20, 0, 2.6);
  plt.EffH1D({"pisoft"}, "mcPT", "Efficiency vs true p_{T,$p};True p_{T,$p} [GeV/c]", 20, 0, 0.25);
  // pz
  plt.EffH1D({"B0"}, "mcPZ", "Efficiency vs true p_{Z,$p};True p_{Z,$p} [GeV/c]", 20, 1, 2);
  plt.EffH1D({"Dst", "D0"}, "mcPZ", "Efficiency vs true p_{Z,$p};True p_{Z,$p} [GeV/c]", 20, -1.5, 3.5);
  plt.EffH1D({"pisoft"}, "mcPZ", "Efficiency vs true p_{Z,$p};True p_{Z,$p} [GeV/c]", 20, -0.2, 0.4);
  // Angles
  plt.EffH1D({"B0"}, "mcTheta", "Efficiency vs true #theta_{$p};True #theta_{$p} [#circ]", 20, 0, 25, 180 / M_PI);
  plt.EffH1D({"Dst", "D0"}, "mcTheta", "Efficiency vs true #theta_{$p};True #theta_{$p} [#circ]", 20, 0, 180, 180 / M_PI);
  plt.EffH1D(CompParts, "mcPhi", "Efficiency vs true #phi_{$p};True #phi_{$p} [#circ]", 20, -180, 180, 180 / M_PI);
  if (!isK3pi)
    plt.EffH1D("Kpi_MCAngle", "Eff. vs true K-to-#pi angle;True angle between K and #pi [#circ]", 20, 40, 180, 180 / M_PI);
}

void DoPlot(SigBkgPlotter& plt, bool isK3pi)
{
  const auto& CompParts = CompositeParticles;
  const auto& FSParts = isK3pi ? K3PiFSParticlesSorted : KPiFSParticles;
  // const auto& Pions = isK3pi ? K3PiPionsSorted : KPiPions;

  // ==== Histograms
  plt.PrintAll(true);

  // ==== sigma68
  // Vertices
  for (const TString& part : CompParts) {
    plt.SigmaAndPrint(part + "_residualDecayX_2", 68, 5);
    plt.SigmaAndPrint(part + "_residualDecayY_2", 68, 5);
    plt.SigmaAndPrint(part + "_residualDecayZ_2", 68, 5);
  }
  plt.SigmaAndPrint("D0_residualFlightDistance_2", 68, 5);
  // Impact parameters
  for (const TString &part : FSParts) {
    plt.SigmaAndPrint(part + "_d0Residual_2", 68, 5);
    plt.SigmaAndPrint(part + "_z0Residual_2", 68, 5);
  }
  for (const TString &part : FSParts) {
    plt.SigmaAndPrint(part + "_d0Pull_2", 68, 5);
    plt.SigmaAndPrint(part + "_z0Pull_2", 68, 5);
  }
}

void DoCandAna(tuple<TH2D*,UInt_t,UInt_t> noCuts, tuple<TH2D*,UInt_t,UInt_t> cuts,
               tuple<TH2D*,UInt_t,UInt_t> bc, UInt_t nMC, PDFCanvas& c, TString title)
{
  auto h = get<0>(noCuts), hCuts = get<0>(cuts);
  if (h && hCuts) {
    h->SetTitle(title + " - " + h->GetTitle() + " (no cuts)");
    hCuts->SetTitle(title + " - " + hCuts->GetTitle() + " (with cuts)");
    c->cd();
    c->SetRightMargin(0.16);
    h->Draw("COLZ");
    c->SetLogy();
    c.PrintPage(h->GetTitle());
    hCuts->Draw("COLZ");
    c->SetLogy();
    c.PrintPage(hCuts->GetTitle());
  }

  double ns = get<1>(noCuts), nb = get<2>(noCuts);
  double nsc = get<1>(cuts), nbc = get<2>(cuts);
  double nsb = get<1>(bc), nbb = get<2>(bc);
  double nt = ns + nb, ntc = nsc + nbc, ntb = nsb + nbb;
  TString fs;
  cout << "             | w/o cuts | w/ cuts  | Cut eff. | w/ b.c.  | b.c. eff." << endl;
  cout << "   ----------+----------+----------+----------+----------+----------" << endl;
  fs.Form("     Total   |%10.0f|%10.0f|%9.4g%%|%10.0f|%9.4g%%", nt, ntc, 100.0 * ntc / nt, ntb, 100.0 * ntb / ntc);
  cout << fs << endl;
  fs.Form("     Signal  |%10.0f|%10.0f|%9.4g%%|%10.0f|%9.4g%%", ns, nsc, 100.0 * nsc / ns, nsb, 100.0 * nsb / nsc);
  cout << fs << endl;
  fs.Form("   Background|%10.0f|%10.0f|%9.4g%%|%10.0f|%9.4g%%", nb, nbc, 100.0 * nbc / nb, nbb, 100.0 * nbb / nbc);
  cout << fs << endl;
  fs.Form("      MC     |%10u|%10u|          |%10u|", nMC, nMC, nMC);
  cout << fs << endl;
  fs.Form("  Efficiency |%9.4g%%|%9.4g%%|          |%9.4g%%|", 100.0 * ns / nMC, 100.0 * nsc / nMC, 100.0 * nsb / nMC);
  cout << fs << endl;
  fs.Form("    Purity   |%9.4g%%|%9.4g%%|          |%9.4g%%|", 100.0 * ns / nt, 100.0 * nsc / ntc, 100.0 * nsb / ntb);
  cout << fs << endl;
}

int main(int argc, char* argv[])
{
  ArgParser parser("Analysis program for B0 -> [D* -> [D0 -> K pi (pi pi)] pi] mu nu.");
  parser.AddPositionalArg("inputRootFile");
  auto args = parser.ParseArgs(argc, argv);

  EnableImplicitMT(NThreads);

  TString inFileName = args["inputRootFile"];
  if (!inFileName.EndsWith(".root")) {
    cout << "Invalid input file (expected to end with \".root\")." << endl;
    return 1;
  }
  TString outFileName = inFileName(0, inFileName.Length() - 5);
  TString outFileNameCuts = inFileName(0, inFileName.Length() - 5) + "_offline_cuts";
  TString outFileNameBC = inFileName(0, inFileName.Length() - 5) + "_best_candidate";

  RDataFrame dfKpi("Kpi", inFileName.Data());
  RDataFrame dfK3pi("K3pi", inFileName.Data());
  RDataFrame dfMCKpi("MCKpi", inFileName.Data());
  RDataFrame dfMCK3pi("MCK3pi", inFileName.Data());

  auto dfDefKpi = defineVariables(dfKpi, false);
  auto dfDefK3pi = defineVariables(dfK3pi, true);
  auto dfCutKpi = applyOfflineCuts(dfDefKpi, false);
  auto dfCutK3pi = applyOfflineCuts(dfDefK3pi, true);
  auto dfBCKpi = dfCutKpi.Filter("B0_M_rank == 1", "Best Candidate");
  auto dfBCK3pi = dfCutK3pi.Filter("B0_M_rank == 1", "Best Candidate");

  gStyle->SetOptStat(0); // TODO If more style lines appear, make a function
  PDFCanvas canvas(outFileName + ".pdf", "c"); // Default size is fine (I wrote it!)
  PDFCanvas canvasCuts(outFileNameCuts + ".pdf", "cc");
  PDFCanvas canvasBC(outFileNameBC + ".pdf", "ccb");
  PDFCanvas canvasCand(outFileName + "_candidates.pdf", "ccc");

  SigBkgPlotter plotterKpi(dfDefKpi, dfMCKpi, SignalCondition, canvas, "Kpi", "K#pi");
  SigBkgPlotter plotterK3pi(dfDefK3pi, dfMCK3pi, SignalCondition, canvas, "K3pi", "K3#pi");
  SigBkgPlotter plotterKpiCuts(dfCutKpi, dfMCKpi, SignalCondition, canvasCuts, "KpiCuts", "K#pi");
  SigBkgPlotter plotterK3piCuts(dfCutK3pi, dfMCK3pi, SignalCondition, canvasCuts, "K3piCuts", "K3#pi");
  SigBkgPlotter plotterKpiBC(dfBCKpi, dfMCKpi, SignalCondition, canvasBC, "KpiBC", "K#pi");
  SigBkgPlotter plotterK3piBC(dfBCK3pi, dfMCK3pi, SignalCondition, canvasBC, "K3piBC", "K3#pi");

  bookHistos(plotterKpi, false);
  bookHistos(plotterK3pi, true);
  bookHistos(plotterKpiCuts, false);
  bookHistos(plotterK3piCuts, true);
  bookHistos(plotterKpiBC, false);
  bookHistos(plotterK3piBC, true);

  cout << "Processing Kpi..." << endl;
  auto nMCKpi = dfMCKpi.Count();
  auto hCandKpi = CutEfficiencyAnalysis(dfDefKpi);
  auto hCandKpiCuts = CutEfficiencyAnalysis(dfCutKpi);
  auto hCandKpiBC = CutEfficiencyAnalysis(dfBCKpi);
  DoCandAna(hCandKpi, hCandKpiCuts, hCandKpiBC, *nMCKpi, canvasCand, "K#pi");

  cout << "Processing K3pi..." << endl;
  auto nMCK3pi = dfMCK3pi.Count();
  auto hCandK3pi = CutEfficiencyAnalysis(dfDefK3pi);
  auto hCandK3piCuts = CutEfficiencyAnalysis(dfCutK3pi);
  auto hCandK3piBC = CutEfficiencyAnalysis(dfBCK3pi);
  DoCandAna(hCandK3pi, hCandK3piCuts, hCandK3piBC, *nMCK3pi, canvasCand, "K3#pi");

  cout << "Total" << endl;
  DoCandAna(
    make_tuple(nullptr, get<1>(hCandKpi) + get<1>(hCandK3pi), get<2>(hCandKpi) + get<2>(hCandK3pi)),
    make_tuple(nullptr, get<1>(hCandKpiCuts) + get<1>(hCandK3piCuts), get<2>(hCandKpiCuts) + get<2>(hCandK3piCuts)),
    make_tuple(nullptr, get<1>(hCandKpiBC) + get<1>(hCandK3piBC), get<2>(hCandKpiBC) + get<2>(hCandK3piBC)),
    *nMCKpi + *nMCK3pi, canvasCand, "");

  // Factors determined empirically, use 1 (or comment lines) for auto
  plotterKpi.SetBkgDownScaleFactor(1);
  plotterKpiCuts.SetBkgDownScaleFactor(1);
  plotterKpiBC.SetBkgDownScaleFactor(1);
  plotterK3pi.SetBkgDownScaleFactor(1);
  plotterK3piCuts.SetBkgDownScaleFactor(1);
  plotterK3piBC.SetBkgDownScaleFactor(1);

  TFile outRootFile(outFileName + "_efficiency.root", "recreate");

  outRootFile.mkdir("Kpi", "Kpi", true)->cd();
  DoPlot(plotterKpi, false);
  outRootFile.mkdir("K3pi", "K3pi", true)->cd();
  DoPlot(plotterK3pi, true);
  outRootFile.mkdir("KpiCuts", "KpiCuts", true)->cd();
  DoPlot(plotterKpiCuts, false);
  outRootFile.mkdir("K3piCuts", "K3piCuts", true)->cd();
  DoPlot(plotterK3piCuts, true);
  outRootFile.mkdir("KpiBC", "KpiBC", true)->cd();
  DoPlot(plotterKpiBC, false);
  outRootFile.mkdir("K3piBC", "K3piBC", true)->cd();
  DoPlot(plotterK3piBC, true);
}
