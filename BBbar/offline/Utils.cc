#include "Utils.hh" // Own include
#include <map>
using namespace std;

TString GetUniqueName(TString baseName)
{
  static map<TString,int> names;
  int n = ++names[baseName];
  if (n == 1) return baseName;
  return TString::Format("%s_%d", baseName.Data(), n);
}
