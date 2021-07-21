#include "CutEfficiency.hh" // Own include
#include "Utils.hh"
#include <TH2.h> // TH2D is forward declared
using namespace std;

const Double_t YBins[] = {
  -0.5, 0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5,
  20.5, 30.5, 40.5, 50.5, 60.5, 70.5, 80.5, 90.5, 100.5,
  200.5, 300.5, 400.5, 500.5, 600.5, 700.5, 800.5, 900.5, 1000.5,
  2000.5, 3000.5, 4000.5, 5000.5, 6000.5, 7000.5, 8000.5, 9000.5, 10000.5
};
const Int_t NBinsY = sizeof(YBins) / sizeof(Double_t) - 1;

void CutEfficiencyAccumulator::Accumulate(UInt_t iSlot, Int_t exp, Int_t run, Int_t evt, bool sig)
{
  auto& slot = m_slots[iSlot];
  EvtID id(exp, run, evt);
  auto& cnt = slot[id];
  cnt.nCandidates++;
  if (sig)
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
                    25, -0.5, 24.5, NBinsY, YBins);
  UInt_t nSig = 0, nBkg = 0;
  for (const auto& t : res) {
    const auto& cnt = t.second;
    h->Fill(cnt.nSigCandidates, cnt.nBkgCandidates);
    nSig += cnt.nSigCandidates;
    nBkg += cnt.nBkgCandidates;
  }
  return make_tuple(h, nSig, nBkg);
}
