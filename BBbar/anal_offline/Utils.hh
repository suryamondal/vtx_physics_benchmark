
#include "GlobalLibs.hh"

class Utils {

public :
  Utils();
  ~Utils(){;};
  
  void Setup(std::map<TString, TString> motherMap,
	     std::vector<TString> &particleNames,
	     std::vector<TString> &histoNames,
	     std::vector<std::vector<Int_t>> &histoBins,
	     std::vector<std::vector<Double_t>> &histoXmin,
	     std::vector<std::vector<Double_t>> &histoXmax,
	     TString chnl);
  
  int printEffi(ROOT::RDataFrame &tr,
		ROOT::RDataFrame &MCtr,
		TString common,
		TString rank);
  Long64_t countTracks(ROOT::RDataFrame &tr,
		       ROOT::RDataFrame &MCtr,
		       TString trk,
		       TString cuts,
		       int isBC);

  void DivideHisto();
  
public:

  std::map<TString, TString> parMotherMap;
  
  std::vector<TString> particleList;
  std::map<TString, Int_t> particleMap;
  
  // std::vector<TString> channelList;
  // std::map<TString, Int_t> channelMap;

  std::vector<TString> histoList;
  std::map<TString, Int_t> histoMap;
  std::vector<std::vector<Int_t>> histoBn;
  std::vector<std::vector<Double_t>> histoXmn;
  std::vector<std::vector<Double_t>> histoXmx;
  
  TString channelName;
  
  TH1D *histo_mc[20][20];	 // [nparticle][histo]
  TH1D *histo_sig[20][20][2]; // [nparticles][histo][bc]
  
  TH1D *histo_effi[20][20][2]; // [nparticles][histo][bc]
  
  ROOT::RDF::RResultPtr<std::vector<Double_t>> parVecList[20];
  
  
private :
  int test;
};
