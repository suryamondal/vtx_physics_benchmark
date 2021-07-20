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
  for (const TString& p : FSParts)
    ddf = ddf.Alias(
      (p + "_firstVXDLayer").Data(),
      ddf.HasColumn((p + "_firstVTXLayer").Data()) ? (p + "_firstVTXLayer").Data()
                                                   : (p + "_firstPXDLayer").Data()
    );
  return ddf.Alias("Dst_M_preFit", "Dst_extraInfo_M_preFit")
            .Alias("D0_M_preFit", "D0_extraInfo_M_preFit")
            .Alias("B0_M_preFit", "B0_extraInfo_M_preFit")
            .Define("massDiffPreFit", "Dst_M_preFit-D0_M_preFit")
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
  const auto& FSParts = isK3pi ? K3PiFSParticles : KPiFSParticles;
  // const auto& AllParts = isK3pi ? K3PiAllParticles : KPiAllParticles;
  // const auto& Pions = isK3pi ? K3PiPions : KPiPions;

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
  plt.Histo1D({"D0"}, "flightDistance", "Flight distance of $p;$p flight distance [cm]", 100, -0.2, 0.2);
  // Residuals
  plt.Histo1D(CompParts, "residualDecayX", "x_{decay,$p} residual;MC - meas [#mum]", 100, -500, 500, 1e4);
  plt.Histo1D(CompParts, "residualDecayY", "y_{decay,$p} residual;MC - meas [#mum]", 100, -500, 500, 1e4);
  plt.Histo1D(CompParts, "residualDecayZ", "z_{decay,$p} residual;MC - meas [#mum]", 100, -500, 500, 1e4);
  plt.Histo1D({"D0"}, "residualFlightDistance", "Residual of flight distance of $p;$p MC - meas [#mum]", 100, -2e3, 2e3, 1e4);
  // Pulls
  plt.Histo1D(CompParts, "pullDecayX", "x_{decay,$p} pull;(MC - meas) / #sigma_{meas}", 100, -5, 5);
  plt.Histo1D(CompParts, "pullDecayY", "y_{decay,$p} pull;(MC - meas) / #sigma_{meas}", 100, -5, 5);
  plt.Histo1D(CompParts, "pullDecayZ", "z_{decay,$p} pull;(MC - meas) / #sigma_{meas}", 100, -5, 5);
  plt.Histo1D({"D0"}, "pullFlightDistance", "Pull of flight distance of $p;(MC - meas) / #sigma_{meas}", 100, -5, 5);

  // ==== Impact parameters
  plt.Histo1D(FSParts, "d0", "d_{0$p} (aka dr);d_{0,$p} [#mum]", 100, 0, 1e3, 1e4);
  plt.Histo1D(FSParts, "z0", "z_{0$p} (aka dz);z_{0,$p} [#mum]", 100, -2e3, 2e3, 1e4);
  // Residuals
  plt.Histo1D(FSParts, "d0Residual", "d_{0$p} residual;MC - meas [#mum]", 100, -100, 100, 1e4);
  plt.Histo1D(FSParts, "z0Residual", "z_{0$p} residual;MC - meas [#mum]", 100, -200, 200, 1e4);
  // Pulls
  plt.Histo1D(FSParts, "d0Pull", "d_{0$p} pull;(MC - meas) / #sigma_{meas}", 100, -5, 5);
  plt.Histo1D(FSParts, "z0Pull", "z_{0$p} pull;(MC - meas) / #sigma_{meas}", 100, -5, 5);
}

void DoPlot(SigBkgPlotter& plt, bool isK3pi)
{
  const auto& CompParts = CompositeParticles;
  const auto& FSParts = isK3pi ? K3PiFSParticles : KPiFSParticles;
  // const auto& Pions = isK3pi ? K3PiPions : KPiPions;

  // ==== Histograms
  plt.PrintAll(false);

  // ==== Fits
  // Vertices
  for (const TString& part : CompParts) {
    plt.FitAndPrint(part + "_residualDecayX", "gaus");
    plt.FitAndPrint(part + "_residualDecayY", "gaus");
    plt.FitAndPrint(part + "_residualDecayZ", "gaus");
  }
  plt.FitAndPrint("D0_residualFlightDistance", "gaus");
  // Impact parameters
  for (const TString &part : FSParts) {
    plt.FitAndPrint(part + "_d0Residual", "gaus");
    plt.FitAndPrint(part + "_z0Residual", "gaus");
  }
}

void DoCandAna(tuple<TH2D*,UInt_t,UInt_t> noCuts, tuple<TH2D*,UInt_t,UInt_t> cuts,
               PDFCanvas& c, TString title)
{
  auto h = get<0>(noCuts), hCuts = get<0>(cuts);
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

  double ns = get<1>(noCuts), nb = get<2>(noCuts), nsc = get<1>(cuts), nbc = get<2>(cuts);
  double nt = ns + nb, ntc = nsc + nbc;
  TString fs;
  cout << "             | w/o cuts | w/ cuts  |Efficiency" << endl;
  cout << "   ----------+----------+----------+----------" << endl;
  fs.Form("     Total   |%10.5g|%10.5g|%9.4g%%", nt, ntc, 100.0 * ntc / nt);
  cout << fs << endl;
  fs.Form("     Signal  |%10.5g|%10.5g|%9.4g%%", ns, nsc, 100.0 * nsc / ns);
  cout << fs << endl;
  fs.Form("   Background|%10.5g|%10.5g|%9.4g%%", nb, nbc, 100.0 * nbc / nb);
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

  RDataFrame dfKpi("Kpi", inFileName.Data());
  RDataFrame dfK3pi("K3pi", inFileName.Data());

  auto dfDefKpi = defineVariables(dfKpi, false);
  auto dfDefK3pi = defineVariables(dfK3pi, true);
  auto dfCutKpi = applyOfflineCuts(dfDefKpi, false);
  auto dfCutK3pi = applyOfflineCuts(dfDefK3pi, true);

  gStyle->SetOptStat(0); // TODO If more style lines appear, make a function
  PDFCanvas canvas(outFileName + ".pdf", "c"); // Default size is fine (I wrote it!)
  PDFCanvas canvasCuts(outFileNameCuts + ".pdf", "cc");
  PDFCanvas canvasCand(outFileName + "_candidates.pdf", "ccc");

  SigBkgPlotter plotterKpi(dfDefKpi, SignalCondition, canvas, "Kpi", "K#pi");
  SigBkgPlotter plotterK3pi(dfDefK3pi, SignalCondition, canvas, "K3pi", "K3#pi");
  SigBkgPlotter plotterKpiCuts(dfCutKpi, SignalCondition, canvasCuts, "KpiCuts", "K#pi");
  SigBkgPlotter plotterK3piCuts(dfCutK3pi, SignalCondition, canvasCuts, "K3piCuts", "K3#pi");

  bookHistos(plotterKpi, false);
  bookHistos(plotterK3pi, true);
  bookHistos(plotterKpiCuts, false);
  bookHistos(plotterK3piCuts, true);

  cout << "Processing Kpi..." << endl;
  auto hCandKpi = CutEfficiencyAnalysis(dfDefKpi);
  auto hCandKpiCuts = CutEfficiencyAnalysis(dfCutKpi);
  DoCandAna(hCandKpi, hCandKpiCuts, canvasCand, "K#pi");

  cout << "Processing K3pi..." << endl;
  auto hCandK3pi = CutEfficiencyAnalysis(dfDefK3pi);
  auto hCandK3piCuts = CutEfficiencyAnalysis(dfCutK3pi);
  DoCandAna(hCandK3pi, hCandK3piCuts, canvasCand, "K3#pi");

  DoPlot(plotterKpi, false);
  DoPlot(plotterK3pi, true);
  DoPlot(plotterKpiCuts, false);
  DoPlot(plotterK3piCuts, true);
}
