
#include "GlobalLibs.hh"

class Utils {

public :
  Utils();
  ~Utils(){;};
  
  void Setup(const std::map<TString, TString> motherMap,
	     const std::vector<TString> &particleNames,
	     const std::vector<TString> &histoNames,
	     const std::vector<std::vector<std::vector<Double_t>>> &histoBins,
	     const std::vector<TString> &particleResoNames,
	     const std::vector<std::vector<TString>> &histoResoNames,
	     const std::vector<std::vector<std::vector<Double_t>>> &histoResoBins,
	     const std::vector<TString> &particleResoFromPullNames,
	     const std::vector<std::vector<TString>> &histoResoFromPullNames,
	     const std::vector<std::vector<std::vector<Double_t>>> &histoResoFromPullBins,
	     const TString chnl,
	     const TTree *tree,
	     const TTree *mctree,
	     const std::map<TString, TString> &paramNames);
  
  int printEffi(const TString common,
		const TString rank);
  Long64_t countTracks(const TString trk,
		       const TString cuts,
		       const int isBC);

  void DivideHisto();
  void makeBranch(const TString partname,
		  const TString parname,
		  const TString type,
		  int *cnt);
  
public:

  std::map<TString, TString> parMotherMap;

  std::map<TString, TString> paramList;
  std::map<TString, std::pair<TString,Int_t>> paramMap; // branchname, type, location
  
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
  
  TH2D *histo_reso_sig[20][20][4]; // [nparticles][histo][bc]

  TH1D *histo_resofrompull_sig[20][20][4]; // [nparticles][histo][bc]
  
  std::vector<TString> histoTypes;
  
private :

  TTree *MCtr, *RCtr;
  
  Int_t expData, runData, evtData, expDataMC, runDataMC, evtDataMC;
  std::vector<Double_t> branchDouble;
  std::vector<Int_t>    branchInt;
  std::vector<Char_t>   branchBool;

  double getDataValue(const TString brdetails);
  
};
