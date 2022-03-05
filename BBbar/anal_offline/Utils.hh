
#include "GlobalDefs.hh"

class Utils {
public :
  Utils();
  ~Utils(){;};
  
  int printEffi(TTree *tr, TTree *MCtr, TString common,
		TString rank, TString signal, TString trk);
  Long64_t countTracks(TTree *tr, TString trk);
  
private :
  int test;
};
