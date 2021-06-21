#include "CutEfficiency.hh" // Own include
#include "Utils.hh"
#include <TH2.h> // TH2D is forward declared
using namespace std;

void CutEfficiencyAccumulator::Accumulate(UInt_t iSlot, Int_t exp, Int_t run, Int_t evt, double sig)
{
  auto& slot = m_slots[iSlot];
  EvtID id(exp, run, evt);
  auto& cnt = slot[id];
  cnt.nCandidates++;
  if (sig == 1.0)
    cnt.nSigCandidates++;
  else
    cnt.nBkgCandidates++;
}

tuple<TH2D*,UInt_t,UInt_t> CutEfficiencyAccumulator::GetResults() const
{
  table_t res;
  for (const auto& slot : m_slots) {
    for (const auto& t : slot) {
      const auto& id = t.first;
      const auto& cnt = t.second;
      res[id].nCandidates += cnt.nCandidates;
      res[id].nSigCandidates += cnt.nSigCandidates;
      res[id].nBkgCandidates += cnt.nBkgCandidates;
    }
  }

  auto h = new TH2D(GetUniqueName("hCandidates"),
                    "Candidates;Signal candidates;Background candidates;Events / bin",
                    11, -0.5, 10.5, 11, -0.5, 10.5);
  UInt_t nSig = 0, nBkg = 0;
  for (const auto& t : res) {
    const auto& cnt = t.second;
    h->Fill(cnt.nSigCandidates, cnt.nBkgCandidates);
    nSig += cnt.nSigCandidates;
    nBkg += cnt.nBkgCandidates;
  }
  return make_tuple(h, nSig, nBkg);
}
