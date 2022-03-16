
#include "GlobalLibs.hh"

class Utils {

public :
  Utils();
  ~Utils(){;};
  
  int printEffi(ROOT::RDataFrame &tr, ROOT::RDataFrame &MCtr, TString common,
		TString rank, TString signal, TString trk, bool isK3pi);
  Long64_t countTracks(ROOT::RDataFrame &tr, ROOT::RDataFrame &MCtr, TString trk,
		       TString cuts, bool isK3pi, int isEffi);
  
public:
  TH1D *B0mcPTKpi_sig;
  TH1D *B0mcPTK3pi_sig;
  TH1D *mumcPTKpi_sig;
  TH1D *mumcPTK3pi_sig;
  TH1D *pisoftmcPTKpi_sig; 
  TH1D *pisoftmcPTK3pi_sig; 
  TH1D *KmcPTKpi_sig; 
  TH1D *KmcPTK3pi_sig; 

  TH1D *B0mcThetaKpi_sig;
  TH1D *B0mcThetaK3pi_sig;
  TH1D *mumcThetaKpi_sig;
  TH1D *mumcThetaK3pi_sig;
  TH1D *pisoftmcThetaKpi_sig; 
  TH1D *pisoftmcThetaK3pi_sig; 
  TH1D *KmcThetaKpi_sig; 
  TH1D *KmcThetaK3pi_sig; 

  TH1D *B0mcPTKpi_sig_bc;
  TH1D *B0mcPTK3pi_sig_bc;
  TH1D *mumcPTKpi_sig_bc;
  TH1D *mumcPTK3pi_sig_bc;
  TH1D *pisoftmcPTKpi_sig_bc; 
  TH1D *pisoftmcPTK3pi_sig_bc; 
  TH1D *KmcPTKpi_sig_bc; 
  TH1D *KmcPTK3pi_sig_bc; 

  TH1D *B0mcThetaKpi_sig_bc;
  TH1D *B0mcThetaK3pi_sig_bc;
  TH1D *mumcThetaKpi_sig_bc;
  TH1D *mumcThetaK3pi_sig_bc;
  TH1D *pisoftmcThetaKpi_sig_bc; 
  TH1D *pisoftmcThetaK3pi_sig_bc; 
  TH1D *KmcThetaKpi_sig_bc; 
  TH1D *KmcThetaK3pi_sig_bc; 
  
private :
  int test;
};
