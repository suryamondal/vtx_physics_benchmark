/** Root macro for checking the number of candidates in an analysis
 * output rootfile.
 * Usage: root -l -b -q n_cand.cc(\"file.root\")
 */
#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TString.h>
#include <iostream>
#include <map>
#include <tuple>
using namespace std;

typedef tuple<Int_t,Int_t,Int_t> EvtID;

typedef struct EvtStats {
  ULong64_t nSig, nBkg;
  EvtStats() : nSig(0), nBkg(0) {}
} EvtStats;

void showNCand(TTree* tree) {
  if (!tree) {
    cout << "TTree not found!" << endl;
    return;
  }

  TTreeReader rdr(tree);
  TTreeReaderValue<Int_t> exp(rdr, "__experiment__");
  TTreeReaderValue<Int_t> run(rdr, "__run__");
  TTreeReaderValue<Int_t> evt(rdr, "__event__");
  TTreeReaderValue<Double_t> isSig(rdr, "B0_isSignalAcceptMissingNeutrino");
  map<EvtID,EvtStats> stats;
  ULong64_t nSigTot = 0, nBkgTot = 0;

  while (rdr.Next()) {
    EvtID id = make_tuple(*exp, *run, *evt);
    if (*isSig) {
      stats[id].nSig++;
      nSigTot++;
    } else {
      stats[id].nBkg++;
      nBkgTot++;
    }
  }

  ULong64_t nEvts = stats.size();
  cout << TString::Format("%llu events, %llu candidates (%llu sig, %llu bkg)",
    nEvts, nSigTot + nBkgTot, nSigTot, nBkgTot) << endl;
  cout << TString::Format("Avg. %.1lf canidates/event (%.1lf sig, %.1lf bkg)",
    (double)(nSigTot + nBkgTot) / nEvts, (double)nSigTot / nEvts, (double)nBkgTot / nEvts) << endl;
}

void n_cand(const char* filename) {
  auto inf = TFile::Open(filename);
  cout << " ========== Kpi ===========" << endl;
  showNCand(inf->Get<TTree>("Kpi"));
  cout << " ========== K3pi ==========" << endl;
  showNCand(inf->Get<TTree>("K3pi"));
}
