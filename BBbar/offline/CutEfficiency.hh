#pragma once
#include "Constants.hh"
#include <ROOT/RDataFrame.hxx>
#include <tuple>
#include <vector>
#include <map>
class TH2D; // Forward declaration

class EvtID {
 public:
  EvtID() = delete;
  EvtID(Int_t expNo, Int_t runNo, Int_t eventNo) : exp(expNo), run(runNo), event(eventNo) {}
  Int_t exp, run, event;
};

inline bool operator<(const EvtID& a, const EvtID& b) { return std::tie(a.exp, a.run, a.event) < std::tie(b.exp, b.run, b.event); }

/** A class for cut efficiency analysis and more. */
class CutEfficiencyAccumulator {
 public:
  CutEfficiencyAccumulator(const CutEfficiencyAccumulator&) = delete;
  CutEfficiencyAccumulator(CutEfficiencyAccumulator&&) = delete;
  CutEfficiencyAccumulator& operator=(const CutEfficiencyAccumulator&) = delete;
  CutEfficiencyAccumulator& operator=(CutEfficiencyAccumulator&&) = delete;

  CutEfficiencyAccumulator(int nSlots = NThreads) { m_slots.resize(nSlots); }

  /** Should be called by RDataFrame::ForeachSlot with
   * columns = {"__experiment__", "__run__", "__event__", "isSignal"}
   */
  void Accumulate(UInt_t iSlot, Int_t exp, Int_t run, Int_t evt, double sig);

  /** Returns the nBkg vs nSig histogram and the sig and bkg counts. */
  std::tuple<TH2D*,UInt_t,UInt_t> GetResults() const;

 private:
  typedef struct EvtCnt { Int_t nCandidates, nSigCandidates, nBkgCandidates; } EvtCnt;
  typedef std::map<EvtID,EvtCnt> table_t;

  std::vector<table_t> m_slots;
};

/** Runs the cut efficiency analysis on the given dataframe.
 * This is an instant action and will trigger data processing.
 */
template <class T>
std::tuple<TH2D*,UInt_t,UInt_t> CutEfficiencyAnalysis(ROOT::RDF::RInterface<T,void>& df)
{
  CutEfficiencyAccumulator accu;
  df.ForeachSlot(
    [&accu](UInt_t iSlot, Int_t exp, Int_t run, Int_t evt, double sig) { accu.Accumulate(iSlot, exp, run, evt, sig); },
    {"__experiment__", "__run__", "__event__", "Dst_isSignal"});
  return accu.GetResults();
}

// TODO function to compare nCands sig/bkg from TH2D* of df w/ and w/o cuts
