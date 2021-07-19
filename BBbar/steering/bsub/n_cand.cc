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
#include <vector>
using namespace std;

typedef tuple<Int_t,Int_t,Int_t> EvtID;

typedef struct EvtStats {
  ULong64_t nSig, nBkg;
  EvtStats() : nSig(0), nBkg(0) {}
} EvtStats;

void printNCand(const char* title, ULong64_t nEvts, ULong64_t nSigTot, ULong64_t nBkgTot) {
  cout << TString::Format("#%s %llu events, %llu candidates (%llu sig, %llu bkg)",
    title, nEvts, nSigTot + nBkgTot, nSigTot, nBkgTot) << endl;
  cout << TString::Format("    Avg. %.1lf canidates/event (%.1lf sig, %.1lf bkg)",
    (double)(nSigTot + nBkgTot) / nEvts, (double)nSigTot / nEvts, (double)nBkgTot / nEvts) << endl;
}

bool all(const vector<TTreeReaderValue<Double_t>*>& v) {
  for (const auto& x : v)
    if (!(**x)) return false;
  return true;
}

void showNCand(TTree* tree, bool isKpi) {
  if (!tree) {
    cout << "TTree not found!" << endl;
    return;
  }

  TTreeReader rdr(tree);
  TTreeReaderValue<Int_t> exp(rdr, "__experiment__");
  TTreeReaderValue<Int_t> run(rdr, "__run__");
  TTreeReaderValue<Int_t> evt(rdr, "__event__");
  TTreeReaderValue<Double_t> isSig(rdr, "B0_isSignalAcceptMissingNeutrino");
  TTreeReaderValue<Double_t> isSigDst(rdr, "Dst_isSignal");
  TTreeReaderValue<Double_t> isSigD0(rdr, "D0_isSignal");
  TTreeReaderValue<Double_t> isSigMu(rdr, "mu_isSignal");
  TTreeReaderValue<Double_t> isSigK(rdr, "K_isSignal");
  TTreeReaderValue<Double_t> isSigPiSoft(rdr, "pisoft_isSignal");
  vector<TTreeReaderValue<Double_t>*> isSigPis;
  if (isKpi) {
    isSigPis.push_back(new TTreeReaderValue<Double_t>(rdr, "pi_isSignal"));
  } else {
    isSigPis.push_back(new TTreeReaderValue<Double_t>(rdr, "pi1_isSignal"));
    isSigPis.push_back(new TTreeReaderValue<Double_t>(rdr, "pi2_isSignal"));
    isSigPis.push_back(new TTreeReaderValue<Double_t>(rdr, "pi3_isSignal"));
  }
  map<EvtID,EvtStats> stats;
  ULong64_t nSigTot = 0, nBkgTot = 0;
  ULong64_t nSigMu = 0, nBkgMu = 0;
  ULong64_t nSigStrict = 0, nBkgStrict = 0;

  while (rdr.Next()) {
    EvtID id = make_tuple(*exp, *run, *evt);
    if (*isSig) {
      stats[id].nSig++;
      nSigTot++;
    } else {
      stats[id].nBkg++;
      nBkgTot++;
    }
    if (*isSig && *isSigMu) nSigMu++; else nBkgMu++;
    if (*isSig && *isSigMu && *isSigD0 && *isSigDst && *isSigK && *isSigPiSoft && all(isSigPis))
      nSigStrict++; else nBkgStrict++;
  }

  for (const auto& x : isSigPis) delete x;

  printNCand("B0", stats.size(), nSigTot, nBkgTot);
  printNCand("B0+mu", stats.size(), nSigMu, nBkgMu);
  printNCand("All", stats.size(), nSigStrict, nBkgStrict);
}

void showNMC(TTree* tree) {
  cout << "#MC " << tree->GetEntries() << " particles." << endl;
}

void n_cand(const char* filename) {
  auto inf = TFile::Open(filename);
  cout << " ========== Kpi ===========" << endl;
  showNCand(inf->Get<TTree>("Kpi"), true);
  showNMC(inf->Get<TTree>("MCKpi"));
  cout << " ========== K3pi ==========" << endl;
  showNCand(inf->Get<TTree>("K3pi"), false);
  showNMC(inf->Get<TTree>("MCK3pi"));
}
