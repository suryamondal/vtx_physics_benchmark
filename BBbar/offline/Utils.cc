#include "Utils.hh" // Own include
#include <TH2.h>
#include <map>
#include <stdexcept>
using namespace std;

TString GetUniqueName(TString baseName)
{
  static map<TString,int> names;
  int n = ++names[baseName];
  if (n == 1) return baseName;
  return TString::Format("%s_%d", baseName.Data(), n);
}

TH2UO Get2DHistUnderOverFlows(TH1* h)
{
  if (!h || h->GetDimension() != 2)
    return {0, 0, 0, 0, 0, 0, 0, 0, 0};
  TH2* h2 = (TH2*)h;

  // Code below is copied from THistPainter.cxx line 7911 (ROOT v608)
  Int_t cellsX = h2->GetXaxis()->GetNbins() + 1;
  Int_t cellsY = h2->GetYaxis()->GetNbins() + 1;
  Int_t firstX = std::max(1, h2->GetXaxis()->GetFirst());
  Int_t firstY = std::max(1, h2->GetYaxis()->GetFirst());
  Int_t lastX = std::min(h2->GetXaxis()->GetLast(), h2->GetXaxis()->GetNbins());
  Int_t lastY = std::min(h2->GetYaxis()->GetLast(), h2->GetYaxis()->GetNbins());

  return {
    h2->Integral(0, firstX - 1, lastY + 1, cellsY),
    h2->Integral(firstX, lastX, lastY + 1, cellsY),
    h2->Integral(lastX + 1, cellsX, lastY + 1, cellsY),
    h2->Integral(0, firstX - 1, firstY, lastY),
    h2->Integral(firstX, lastX, firstY, lastY),
    h2->Integral(lastX + 1, cellsX, firstY, lastY),
    h2->Integral(0, firstX - 1, 0, firstY - 1),
    h2->Integral(firstX, lastX, 0, firstY - 1),
    h2->Integral(lastX + 1, cellsX, 0, firstY - 1)
  };
}
