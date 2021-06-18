#include "SigBkgPlotter.hh"
#include "PDFCanvas.hh"
#include "Utils.hh"
#include "ArgParser.hh"
#include "Constants.hh"
#include <TString.h>
#include <TStyle.h>
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
  plt.Histo1D("massDiff", "#DeltaM;M_{D*} - M_{D^{0}} [GeV/c^{2}];Events / bin", 100, 0.13, 0.16);

  plt.Histo1D(CompParts, "M_preFit", "M_{$p} (pre-fit);M_{$p} [GeV/c^{2}];Events / bin", 100, 1, 3);
  plt.Histo1D("massDiffPreFit", "#DeltaM (pre-fit);M_{D*} - M_{D^{0}} [GeV/c^{2}];Events / bin", 100, 0.1, 0.3);

  plt.Histo1D(FSParts, "dr", "dr_{$p};dr_{$p} [cm];Events / bin", 100, 0, 3);
  plt.Histo1D(FSParts, "dz", "dz_{$p};dz_{$p} [cm];Events / bin", 100, -3, 3);

  plt.Histo1D(FSParts, "nCDCHits", "CDC Hits_{$p};CDC Hits_{$p};Events / bin", 101, -0.5, 100.5);
  plt.Histo1D(FSParts, "nVXDHits", "VXD Hits_{$p};VXD Hits_{$p};Events / bin", 25, -0.5, 24.5);
  plt.Histo1D(FSParts, "nVXDHits", "VXD Hits_{$p};VXD Hits_{$p};Events / bin", 25, -0.5, 24.5);
  plt.Histo1D(FSParts, "firstVXDLayer", "First VXD layer for $p;Layer;Events / bin", 11, -0.5, 10.5);

  plt.Histo1D("Dst_p_CMS", "p_{CM,D*};P_{CM,D*} [GeV/c];Events / bin", 100, 0, 3);

  plt.Histo1D(CompParts, "significanceOfDistance", "Significance of distance $p;Significance of distance $p;Events / bin", 100, 0, 10);
  plt.Histo1D(CompParts, "flightDistance", "Flight distance $p;Flight distance $p [cm];Events / bin", 100, -10, 10);

  // plt.Histo2D("D0_M", "Dst_M", "D* vs D^{0} masses;M_{D^{0}} [GeV/c^{2}];M_{D*} [GeV/c^{2}];Events / bin",
  //             50, 1.66, 2.06, 50, 1.8, 2.2);

  plt.Histo1D(FSParts, "pionID", "#pi_{ID} for $p;#pi_{ID};Events / bin", 100, 0, 1);
  plt.Histo1D(FSParts, "kaonID", "K_{ID} for $p;K_{ID};Events / bin", 100, 0, 1);
  plt.Histo2D(FSParts, "pionID", "kaonID", "K_{ID} vs #pi_{ID} for $p;#pi_{ID};K_{ID};Events / bin", 100, 0, 1, 100, 0, 1);
  // plt.Histo1D({"pisoft"}, "piVsKID", "K vs #pi ID for $p;K_{ID}/(K_{ID}+#pi_{ID});Events / bin", 100, 0, 0.02);
}

void makeHistosAndPlot(RDataFrame& df, SigBkgPlotter& plt, bool isK3pi)
{
  bookHistos(plt, isK3pi);
  cout << "Processing " << (isK3pi ? "K3pi" : "Kpi") << "..." << endl;
  df.Report()->Print();
  // Plots
  plt.PrintAll(false);
  // Fits
  plt.FitAndPrint("Dst_residualDecayX", "gaus");
  plt.FitAndPrint("Dst_residualDecayY", "gaus");
  plt.FitAndPrint("Dst_residualDecayZ", "gaus");
  plt.FitAndPrint("D0_residualDecayX", "gaus");
  plt.FitAndPrint("D0_residualDecayY", "gaus");
  plt.FitAndPrint("D0_residualDecayZ", "gaus");
}

int main(int argc, char* argv[])
{
  ArgParser parser("Analysis program for D* -> [D0 -> K pi (pi pi)] pi.");
  parser.AddPositionalArg("inputRootFile");
  parser.AddFlag("--offline-cuts");
  parser.AddFlag("--normalize-histos");
  auto args = parser.ParseArgs(argc, argv);
  bool offlineCuts = args.find("offline-cuts") != args.end();
  bool normalizeHistos = args.find("normalize-histos") != args.end();

  TString inFileName = args["inputRootFile"];
  if (!inFileName.EndsWith(".root")) {
    cout << "Invalid input file (expected to end with \".root\")." << endl;
    return 1;
  }
  TString outFileSuffix = "";
  if (offlineCuts) outFileSuffix += "_offline-cuts";
  if (normalizeHistos) outFileSuffix += "_norm-hist";
  TString outFileName = inFileName(0, inFileName.Length() - 5) + outFileSuffix + ".pdf";

  RDataFrame dfKpi("Dst_D0pi_Kpi", inFileName.Data());
  RDataFrame dfK3pi("Dst_D0pi_K3pi", inFileName.Data());

  auto dfDefKpi = defineVariables(dfKpi, false);
  auto dfDefK3pi = defineVariables(dfK3pi, true);

  gStyle->SetOptStat(0); // TODO If more style lines appear, make a function
  PDFCanvas canvas(outFileName); // Default size is fine (I wrote it!)

  if (offlineCuts) {
    auto dfCutKpi = applyOfflineCuts(dfDefKpi, false);
    auto dfCutK3pi = applyOfflineCuts(dfDefK3pi, true);
    SigBkgPlotter plotterKpi(dfCutKpi, "Dst_isSignal==1", canvas, "Kpi", "K#pi", normalizeHistos);
    SigBkgPlotter plotterK3pi(dfCutK3pi, "Dst_isSignal==1", canvas, "K3pi", "K3#pi", normalizeHistos);
    makeHistosAndPlot(dfKpi, plotterKpi, false);
    makeHistosAndPlot(dfK3pi, plotterK3pi, true);
  } else {
    SigBkgPlotter plotterKpi(dfDefKpi, "Dst_isSignal==1", canvas, "Kpi", "K#pi", normalizeHistos);
    SigBkgPlotter plotterK3pi(dfDefK3pi, "Dst_isSignal==1", canvas, "K3pi", "K3#pi", normalizeHistos);
    makeHistosAndPlot(dfKpi, plotterKpi, false);
    makeHistosAndPlot(dfK3pi, plotterK3pi, true);
  }
}
