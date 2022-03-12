
#include "GlobalDefs.hh"
#include "Utils.hh"

using namespace std;

int main(int argc, char **argv) {

  if(argc==1) {return -1;}

  // cout << motherMap["pisoft"] << endl;

  cout<<endl<<" particles: ";
  for(int ij=0;ij<(argc-1);ij++) {
    TString particleName = (ij==0)?"B0":argv[ij+1];
    cout<<" "<<particleName;} cout<<endl<<endl;

  TString outfile = argv[1];
  outfile.ReplaceAll(".root","_myout.root");
  cout<<" infile: "<<argv[1]<<endl<<" outfile "<<outfile<<endl;

  TString outfile1 = argv[1];
  outfile1.ReplaceAll(".root","_myout1.root");
  
  TFile *f_sim = new TFile(argv[1],"READ");
  TFile *file1  = new TFile(outfile1,"recreate");
  
  TTree *MCKpi = (TTree*)f_sim->Get("MCKpi");
  TTree *MCK3pi = (TTree*)f_sim->Get("MCK3pi");
  TTree *Kpi = (TTree*)f_sim->Get("Kpi");
  TTree *K3pi = (TTree*)f_sim->Get("K3pi");
  
  TString cutsKpi  = CommonCuts + " && " + KpiCuts;
  TString cutsK3pi = CommonCuts + " && " + K3piCuts;

  Utils testUtils;
  
  for(int ij=0;ij<(argc-1);ij++) {
    TString particleName = (ij==0)?"B0":argv[ij+1];
    TString rank   = "B0_M_rank==1";
    TString signal = particleName+"_isSignal==1";
    if(ij) {signal += " && TMath::Abs(" + particleName + "_genMotherPDG)==" + motherMap[particleName];}
    TString trk    = (ij==0)?"":particleName;
    cout<<endl<<" "<<particleName<<" efficiency and purity : signal cut "<<signal<<endl;
    
    // TString motherCheck = "TMath::Abs(" + trk + "_genMotherPDG)==" + motherMap["pisoft"];
    // cout<<motherMap["pisoft"]<<endl;
    
    cout<<" Kpi branch "<<endl;
    testUtils.printEffi(Kpi,MCKpi,cutsKpi,rank,signal,trk);
    cout<<" K3pi branch "<<endl;
    testUtils.printEffi(K3pi,MCK3pi,cutsK3pi,rank,signal,trk);
  }

  f_sim->Close();
  file1->Close();
  system("rm -f "+outfile1);
  
  TFile *file  = new TFile(outfile,"recreate");
  file->Purge();
  file->Close();

  return 0;
}
