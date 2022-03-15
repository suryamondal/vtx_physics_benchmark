
#include "GlobalLibs.hh"

class Utils {
public :
  Utils();
  ~Utils(){;};
  
  int printEffi(ROOT::RDataFrame &tr, ROOT::RDataFrame &MCtr, TString common,
		TString rank, TString signal, TString trk);
  Long64_t countTracks(ROOT::RDataFrame &tr, ROOT::RDataFrame &MCtr, TString trk, TString cuts);
  
private :
  int test;
};
