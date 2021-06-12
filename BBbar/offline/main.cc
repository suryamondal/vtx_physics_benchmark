#include "SigBkgPlotter.hh"
#include "PDFCanvas.hh"
#include "Utils.hh"
#include <TString.h>
#include <TStyle.h>
#include <ROOT/RDataFrame.hxx>
#include <iostream>
using namespace std;
using namespace ROOT;

/** Define expression variables. */
SigBkgPlotter::DefineDF defineVariables(RDataFrame& df, bool isK3pi)
{
  return df.Define("massDiffPreFit", "Dst_M_preFit-D0_M_preFit");
}

/** Books plots. */
void bookHistos(SigBkgPlotter& plt, bool isK3pi)
{
  plt.Histo1D({"Dst", "D0"}, "M", "M_{$p};M_{$p} [GeV];Events / bin", 100, 1, 3);
  plt.Histo1D({"Dst", "D0"}, "M_preFit", "M_{$p} (pre-fit);M_{$p} [GeV];Events / bin", 100, 1, 3);
  plt.Histo1D("massDiffPreFit", "#DeltaM (pre-fit);M_{D*} - M_{D^{0}} [GeV];Events / bin", 100, 0, 0.4);

  // TODO PXD, SVD, VTX Hits, repeat for pi, pi1, pi2, pi3, pisoft, K
  // TODO Need to add ifs for VTX and pi/pi123

  // TODO same as above for z0, d0
}

int main(int argc, char* argv[])
{
  if (argc != 2) {
    const char* prog = argc > 0 ? argv[0] : "./ana";
    cout << "Analysis program for D* -> [D0 -> K pi (pi pi)] pi." << endl;
    cout << "  Usage: " << prog << " input_ntuple.root" << endl;
    return 1;
  }

  TString inFileName = argv[1];
  if (!inFileName.EndsWith(".root")) {
    cout << "Invalid input file (expected to end with \".root\")." << endl;
    return 1;
  }
  TString outFileName = inFileName(0, inFileName.Length() - 5) + ".pdf";

  RDataFrame dfKpi("Dst_D0pi_Kpi", inFileName.Data());
  RDataFrame dfK3pi("Dst_D0pi_K3pi", inFileName.Data());

  auto dfDefKpi = defineVariables(dfKpi, false);
  auto dfDefK3pi = defineVariables(dfK3pi, true);

  gStyle->SetOptStat(0); // TODO If more style lines appear, make a function
  PDFCanvas canvas(outFileName); // Default size is fine (I wrote it!)
  SigBkgPlotter plotterKpi(dfDefKpi, "Dst_isSignal==1", canvas, "Kpi", "K#pi");
  SigBkgPlotter plotterK3pi(dfDefK3pi, "Dst_isSignal==1", canvas, "K3pi", "K3#pi");

  bookHistos(plotterKpi, false);
  bookHistos(plotterK3pi, true);

  cout << "Processing Kpi..." << endl;
  dfKpi.Report()->Print();
  cout << "Processing K3pi..." << endl;
  dfK3pi.Report()->Print();

  plotterKpi.PrintAll();
  plotterK3pi.PrintAll();
}
