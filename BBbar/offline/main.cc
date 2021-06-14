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

/** Define expression variables. */
SigBkgPlotter::DefineDF defineVariables(RDataFrame& df, bool isK3pi)
{
  return df.Define("massDiffPreFit", "Dst_M_preFit-D0_M_preFit")
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
  const auto& particlesFS = isK3pi ? K3PiFSParticles : KPiFSParticles;

  plt.Histo1D({"Dst", "D0"}, "M", "M_{$p};M_{$p} [GeV];Events / bin", 100, 1, 3);
  plt.Histo1D("massDiff", "#DeltaM;M_{D*} - M_{D^{0}} [GeV];Events / bin", 100, 0, 0.4);

  plt.Histo1D({"Dst", "D0"}, "M_preFit", "M_{$p} (pre-fit);M_{$p} [GeV];Events / bin", 100, 1, 3);
  plt.Histo1D("massDiffPreFit", "#DeltaM (pre-fit);M_{D*} - M_{D^{0}} [GeV];Events / bin", 100, 0, 0.4);

  plt.Histo1D(particlesFS, "dr", "dr_{$p};dr_{$p} [cm];Events / bin", 100, 0, 20);
  plt.Histo1D(particlesFS, "dz", "dz_{$p};dz_{$p} [cm];Events / bin", 100, -10, 10);

  plt.Histo1D(particlesFS, "nCDCHits", "CDC Hits_{$p};CDC Hits_{$p};Events / bin", 51, -0.5, 50.5);
  plt.Histo1D(particlesFS, "nVXDHits", "VXD Hits_{$p};VXD Hits_{$p};Events / bin", 51, -0.5, 50.5);
  if (plt.HasVTX()) {
    plt.Histo1D(particlesFS, "nVTXHits", "VTX Hits_{$p};VTX Hits_{$p};Events / bin", 11, -0.5, 10.5);
  } else {
    plt.Histo1D(particlesFS, "nSVDHits", "SVD Hits_{$p};SVD Hits_{$p};Events / bin", 11, -0.5, 10.5);
    plt.Histo1D(particlesFS, "nPXDHits", "PXD Hits_{$p};PXD Hits_{$p};Events / bin", 11, -0.5, 10.5);
  }
}

void makeHistosAndPlot(RDataFrame& df, SigBkgPlotter& plt, bool isK3pi)
{
  bookHistos(plt, isK3pi);
  cout << "Processing " << (isK3pi ? "K3pi" : "Kpi") << "..." << endl;
  df.Report()->Print();
  plt.PrintAll();
}

int main(int argc, char* argv[])
{
  ArgParser parser("Analysis program for D* -> [D0 -> K pi (pi pi)] pi.");
  parser.AddPositionalArg("inputRootFile");
  parser.AddFlag("--offline-cuts");
  auto args = parser.ParseArgs(argc, argv);
  bool offlineCuts = args.find("offline-cuts") != args.end();

  TString inFileName = args["inputRootFile"];
  if (!inFileName.EndsWith(".root")) {
    cout << "Invalid input file (expected to end with \".root\")." << endl;
    return 1;
  }
  TString outFileSuffix = offlineCuts ? "_offline_cuts" : "";
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
    SigBkgPlotter plotterKpi(dfCutKpi, "Dst_isSignal==1", canvas, "Kpi", "K#pi");
    SigBkgPlotter plotterK3pi(dfCutK3pi, "Dst_isSignal==1", canvas, "K3pi", "K3#pi");
    makeHistosAndPlot(dfKpi, plotterKpi, false);
    makeHistosAndPlot(dfK3pi, plotterK3pi, true);
  } else {
    SigBkgPlotter plotterKpi(dfDefKpi, "Dst_isSignal==1", canvas, "Kpi", "K#pi");
    SigBkgPlotter plotterK3pi(dfDefK3pi, "Dst_isSignal==1", canvas, "K3pi", "K3#pi");
    makeHistosAndPlot(dfKpi, plotterKpi, false);
    makeHistosAndPlot(dfK3pi, plotterK3pi, true);
  }
}
