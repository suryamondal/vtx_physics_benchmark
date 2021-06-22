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

  auto ddf = defineVarsForParticles(
    df, FSParts, {"kaonID"}, {"pionID"}, {"piVsKID"}, "$a/($a+$b)");
  ddf = defineVarsForParticles(
    ddf, CompParts,
    {"x",              "y",              "z"},
    {"mcDecayVertexX", "mcDecayVertexY", "mcDecayVertexZ"},
    {"residualDecayX", "residualDecayY", "residualDecayZ"});
  for (const TString& p : FSParts)
    ddf = ddf.Define(
      (p + "_firstVXDLayer").Data(),
      ddf.HasColumn((p + "_firstVTXLayer").Data())
      ? (p + "_firstVTXLayer").Data() : (p + "_firstPXDLayer").Data());
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
  const auto& FSParts = isK3pi ? K3PiFSParticles : KPiFSParticles;
  // const auto& AllParts = isK3pi ? K3PiAllParticles : KPiAllParticles;

  plt.Histo1D("Dst_residualDecayX", "x_{decay,D*} residual;x_{decay,meas} - x_{decay,MC} [cm];Events / bin", 100, -2, 2);
  plt.Histo1D("Dst_residualDecayY", "y_{decay,D*} residual;y_{decay,meas} - y_{decay,MC} [cm];Events / bin", 100, -2, 2);
  plt.Histo1D("Dst_residualDecayZ", "z_{decay,D*} residual;z_{decay,meas} - z_{decay,MC} [cm];Events / bin", 100, -2, 2);
  plt.Histo1D("D0_residualDecayX", "x_{decay,D^{0}} residual;x_{decay,meas} - x_{decay,MC} [cm];Events / bin", 100, -0.1, 0.1);
  plt.Histo1D("D0_residualDecayY", "y_{decay,D^{0}} residual;y_{decay,meas} - y_{decay,MC} [cm];Events / bin", 100, -0.1, 0.1);
  plt.Histo1D("D0_residualDecayZ", "z_{decay,D^{0}} residual;z_{decay,meas} - z_{decay,MC} [cm];Events / bin", 100, -0.1, 0.1);

  plt.Histo1D(CompParts, "M", "M_{$p};M_{$p} [GeV/c^{2}];Events / bin", 100, 1, 3);
  plt.Histo1D("massDiff", "#DeltaM;M_{D*} - M_{D^{0}} [GeV/c^{2}];Events / bin", 100, 0.1, 0.3);
  plt.Histo1D(CompParts, "M_preFit", "M_{$p} (pre-fit);M_{$p} [GeV/c^{2}];Events / bin", 100, 1, 3);
  plt.Histo1D("massDiffPreFit", "#DeltaM (pre-fit);M_{D*} - M_{D^{0}} [GeV/c^{2}];Events / bin", 100, 0.1, 0.3);
  // Post-fit masses and diff also plotted with greater resolution
  plt.Histo1D({"Dst"}, "M", "M_{$p};M_{$p} [GeV/c^{2}];Events / bin", 120, 1.98, 2.04);
  plt.Histo1D({"D0"}, "M", "M_{$p};M_{$p} [GeV/c^{2}];Events / bin", 120, 1.834, 1.894);
  plt.Histo1D("massDiff", "#DeltaM;M_{D*} - M_{D^{0}} [GeV/c^{2}];Events / bin", 50, 0.143, 0.148);

  plt.Histo1D(FSParts, "dr", "dr_{$p};dr_{$p} [cm];Events / bin", 100, 0, 3);
  plt.Histo1D(FSParts, "dz", "dz_{$p};dz_{$p} [cm];Events / bin", 100, -3, 3);

  plt.Histo1D(FSParts, "nCDCHits", "CDC Hits_{$p};CDC Hits_{$p};Events / bin", 101, -0.5, 100.5);
  plt.Histo1D(FSParts, "nVXDHits", "VXD Hits_{$p};VXD Hits_{$p};Events / bin", 25, -0.5, 24.5);
  plt.Histo1D(FSParts, "nVXDHits", "VXD Hits_{$p};VXD Hits_{$p};Events / bin", 25, -0.5, 24.5);
  plt.Histo1D(FSParts, "firstVXDLayer", "First VXD layer for $p;Layer;Events / bin", 11, -0.5, 10.5);

  plt.Histo1D("Dst_p_CMS", "p_{CM,D*};P_{CM,D*} [GeV/c];Events / bin", 100, 0, 3);

  plt.Histo1D(CompParts, "significanceOfDistance", "Significance of distance $p;Significance of distance $p;Events / bin", 100, 0, 10);
  plt.Histo1D(CompParts, "flightDistance", "Flight distance $p;Flight distance $p [cm];Events / bin", 100, -10, 10);

  plt.Histo1D(FSParts, "pionID", "#pi_{ID} for $p;#pi_{ID};Events / bin", 100, 0, 1);
  plt.Histo1D(FSParts, "kaonID", "K_{ID} for $p;K_{ID};Events / bin", 100, 0, 1);
  plt.Histo2D(FSParts, "pionID", "kaonID", "K_{ID} vs #pi_{ID} for $p;#pi_{ID};K_{ID};Events / bin", 20, 0, 1, 20, 0, 1);
}

void DoPlot(SigBkgPlotter& plt, bool isK3pi)
{
  const auto& CompParts = CompositeParticles;
  const auto& FSParts = isK3pi ? K3PiFSParticles : KPiFSParticles;

  // Histograms
  plt.PrintAll(false);

  // ROC curves
  plt.PrintROC(CompParts, "M_2", true);
  plt.PrintROC(CompParts, "M_2", false);
  plt.PrintROC("massDiff_2", true);
  plt.PrintROC("massDiff_2", false);
  plt.PrintROC("Dst_p_CMS", true);
  plt.PrintROC(FSParts, "dr", true);
  plt.PrintROC(FSParts, "dz", true);
  plt.PrintROC(FSParts, "dz", false);
  plt.PrintROC(FSParts, "nVXDHits", false);
  plt.PrintROC(FSParts, "nCDCHits", false);
  plt.PrintROC(FSParts, "pionID", false);
  plt.PrintROC(FSParts, "kaonID", false);

  // Fits
  plt.FitAndPrint("Dst_residualDecayX", "gaus");
  plt.FitAndPrint("Dst_residualDecayY", "gaus");
  plt.FitAndPrint("Dst_residualDecayZ", "gaus");
  plt.FitAndPrint("D0_residualDecayX", "gaus");
  plt.FitAndPrint("D0_residualDecayY", "gaus");
  plt.FitAndPrint("D0_residualDecayZ", "gaus");
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
  ArgParser parser("Analysis program for D* -> [D0 -> K pi (pi pi)] pi.");
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

  RDataFrame dfKpi("Dst_D0pi_Kpi", inFileName.Data());
  RDataFrame dfK3pi("Dst_D0pi_K3pi", inFileName.Data());

  auto dfDefKpi = defineVariables(dfKpi, false);
  auto dfDefK3pi = defineVariables(dfK3pi, true);
  auto dfCutKpi = applyOfflineCuts(dfDefKpi, false);
  auto dfCutK3pi = applyOfflineCuts(dfDefK3pi, true);

  gStyle->SetOptStat(0); // TODO If more style lines appear, make a function
  PDFCanvas canvas(outFileName + ".pdf", "c"); // Default size is fine (I wrote it!)
  PDFCanvas canvasCuts(outFileNameCuts + ".pdf", "cc");
  PDFCanvas canvasCand(outFileName + "_candidates.pdf", "ccc");

  SigBkgPlotter plotterKpi(dfDefKpi, "Dst_isSignal==1", canvas, "Kpi", "K#pi");
  SigBkgPlotter plotterK3pi(dfDefK3pi, "Dst_isSignal==1", canvas, "K3pi", "K3#pi");
  SigBkgPlotter plotterKpiCuts(dfCutKpi, "Dst_isSignal==1", canvasCuts, "KpiCuts", "K#pi");
  SigBkgPlotter plotterK3piCuts(dfCutK3pi, "Dst_isSignal==1", canvasCuts, "K3piCuts", "K3#pi");

  bookHistos(plotterKpi, false);
  bookHistos(plotterK3pi, true);
  bookHistos(plotterKpiCuts, false);
  bookHistos(plotterK3piCuts, true);

  cout << "Processing Kpi..." << endl;
  auto hCandKpi = CutEfficiencyAnalysis(dfDefKpi);
  auto hCandKpiCuts = CutEfficiencyAnalysis(dfCutKpi);
  DoCandAna(hCandKpi, hCandKpiCuts, canvasCand, "K#pi");
  //dfKpi.Report()->Print();
  cout << "Processing K3pi..." << endl;
  auto hCandK3pi = CutEfficiencyAnalysis(dfDefK3pi);
  auto hCandK3piCuts = CutEfficiencyAnalysis(dfCutK3pi);
  DoCandAna(hCandK3pi, hCandK3piCuts, canvasCand, "K3#pi");
  //dfK3pi.Report()->Print();

  for (bool normalizeHistos : {false, true}) {
    for (bool logScale : {true, false}) {
      TString outFileSuffix = "";
      if (normalizeHistos) outFileSuffix += "_norm-hist";
      if (!logScale) outFileSuffix += "_no-log-scale";
      canvas.SetPDFFileName(outFileName + outFileSuffix + ".pdf");
      canvasCuts.SetPDFFileName(outFileNameCuts + outFileSuffix + ".pdf");
      plotterKpi.SetNormalizeHistos(normalizeHistos);
      plotterKpi.SetLogScale(logScale);
      plotterK3pi.SetNormalizeHistos(normalizeHistos);
      plotterK3pi.SetLogScale(logScale);
      plotterKpiCuts.SetNormalizeHistos(normalizeHistos);
      plotterKpiCuts.SetLogScale(logScale);
      plotterK3piCuts.SetNormalizeHistos(normalizeHistos);
      plotterK3piCuts.SetLogScale(logScale);
      DoPlot(plotterKpi, false);
      DoPlot(plotterK3pi, true);
      DoPlot(plotterKpiCuts, false);
      DoPlot(plotterK3piCuts, true);
    }
  }
}
