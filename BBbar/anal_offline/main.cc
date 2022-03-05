
#include "GlobalDefs.hh"

#include "Utils.hh"

using namespace std;


int main(int argc, char **argv) {

  if(argc==1) {return -1;}
  
  // flow2 bkg->0
  // TString infile = "rel_v6.00.03/20220301aj_vtx/reco/test_reco_snm.root";
  // flow2 bkg->0.3
  // TString infile = "rel_v6.00.03/20220221ac_vtx/reco/test_reco_snm.root"; 
  // flow1 bkg->0
  // TString infile = "rel_v6.00.03/20220301ah_vtx/reco/test_reco_snm.root"; 
  
  // TString outfile = infile;
  TString outfile = argv[1];
  outfile.ReplaceAll(".root","_myout.root");
  cout<<argv[1]<<" "<<outfile<<endl;
  
  TFile *f_sim = new TFile(argv[1],"READ");
  TFile *file  = new TFile(outfile,"recreate");

  TTree *Kpi = (TTree*)f_sim->Get("Kpi");
  TTree *K3pi = (TTree*)f_sim->Get("K3pi");
  TTree *MCKpi = (TTree*)f_sim->Get("MCKpi");
  TTree *MCK3pi = (TTree*)f_sim->Get("MCK3pi");
  
  TString cutsKpi  = CommonCuts + " && " + KpiCuts;
  TString cutsK3pi = CommonCuts + " && " + K3piCuts;
  
  Utils testUtils;
  
  cout<<endl<<" B0 efficiency and purity "<<endl;
  cout<<" Kpi branch "<<endl;
  testUtils.printEffi(Kpi,MCKpi,cutsKpi,"B0_M_rank==1","B0_isSignal==1","");
  cout<<" K3pi branch "<<endl;
  testUtils.printEffi(K3pi,MCK3pi,cutsK3pi,"B0_M_rank==1","B0_isSignal==1","");

  cout<<endl<<" pisoft efficiency and purity "<<endl;
  cout<<" Kpi branch "<<endl;
  testUtils.printEffi(Kpi,MCKpi,cutsKpi,"B0_M_rank==1","pisoft_isSignal==1","pisoft");
  cout<<" K3pi branch "<<endl;
  testUtils.printEffi(K3pi,MCK3pi,cutsK3pi,"B0_M_rank==1","pisoft_isSignal==1","pisoft");
  
  cout<<endl<<" mu efficiency and purity "<<endl;
  cout<<" Kpi branch "<<endl;
  testUtils.printEffi(Kpi,MCKpi,cutsKpi,"B0_M_rank==1","mu_isSignal==1","mu");
  cout<<" K3pi branch "<<endl;
  testUtils.printEffi(K3pi,MCK3pi,cutsK3pi,"B0_M_rank==1","mu_isSignal==1","mu");
    
  return 0;
}
