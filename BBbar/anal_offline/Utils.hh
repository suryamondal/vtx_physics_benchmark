
#include "GlobalLibs.hh"

class Utils {

public :
  Utils();
  ~Utils(){;};
  
  void Setup(std::map<TString, TString> motherMap,
	     std::vector<TString> &particleNames,
	     std::vector<TString> &histoNames,
	     std::vector<std::vector<std::vector<Double_t>>> &histoBins,
	     std::vector<TString> &particleResoNames,
	     std::vector<std::vector<TString>> &histoResoNames,
	     std::vector<std::vector<std::vector<Double_t>>> &histoResoBins,
	     std::vector<TString> &particleResoFromPullNames,
	     std::vector<std::vector<TString>> &histoResoFromPullNames,
	     std::vector<std::vector<std::vector<Double_t>>> &histoResoFromPullBins,
	     TString chnl,
	     std::vector<TString> &paramNames);
  
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

  std::vector<TString> paramList;
  std::map<TString, Int_t> paramMap;
  
  std::vector<TString> particleList;
  std::map<TString, Int_t> particleMap;
  
  std::vector<TString> histoList;
  std::map<TString, Int_t> histoMap;
  std::vector<std::vector<std::vector<Double_t>>> histoBn;

  std::vector<TString> particleResoList;
  std::map<TString, Int_t> particleResoMap;
  
  std::vector<std::vector<TString>> histoResoList;
  std::map<TString, Int_t> histoResoMap;
  std::vector<std::vector<std::vector<Double_t>>> histoResoBn;
  
  std::vector<TString> particleResoFromPullList;
  std::map<TString, Int_t> particleResoFromPullMap;
  
  std::vector<std::vector<TString>> histoResoFromPullList;
  std::map<TString, Int_t> histoResoFromPullMap;
  std::vector<std::vector<std::vector<Double_t>>> histoResoFromPullBn;
  
  TString channelName;
  
  TH1D *histo_mc[20][20];	// [nparticle][histo]
  TH1D *histo_sig[20][20][4];	// [nparticles][histo][bc]
  
  TH1D *histo_effi[20][20][4];	// [nparticles][histo][bc]
  TH1D *histo_purity[20][20][4];	// [nparticles][histo][bc]

  ROOT::RDF::RResultPtr<std::vector<Double_t>> parVecList[50];

  TH2D *histo_reso_sig[20][20][4]; // [nparticles][histo][bc]

  TH1D *histo_resofrompull_sig[20][20][4]; // [nparticles][histo][bc]
  
  std::vector<TString> histoTypes;
  
private :
  int test;
};
