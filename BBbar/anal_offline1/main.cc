
#include "GlobalDefs.hh"
#include "Utils.hh"

using namespace std;

template<typename T, typename Allocator>
void printVector(const vector<T, Allocator> &vect) {
  for (const auto &i : vect) {cout << i << " ";}
  cout<<endl;}

int main(int argc, char **argv) {

  if(argc<=2) {return -1;}

  // cout << motherMap["pisoft"] << endl;

  cout<<endl<<" particles: ";
  for(int ij=0;ij<(argc-2);ij++) {
    TString particleName = argv[ij+2];
    cout<<" "<<particleName;} cout<<endl<<endl;

  TString outfile = argv[1];
  outfile.ReplaceAll(".root","_myout.root");
  cout<<" infile: "<<argv[1]<<endl<<" outfile "<<outfile<<endl;

  TString outfile1 = argv[1];
  outfile1.ReplaceAll(".root","_myout1.root");
  
  // TFile *f_sim = new TFile(argv[1],"READ");
  TFile *file1  = new TFile(outfile1,"recreate");

  ROOT::RDataFrame MCKpi("MCKpi",   argv[1]);
  ROOT::RDataFrame MCK3pi("MCK3pi", argv[1]);
  ROOT::RDataFrame Kpi("Kpi",       argv[1]);
  ROOT::RDataFrame K3pi("K3pi",     argv[1]);
  
  /* Cuts for analysis */
  TString cutsKpi  = CommonCuts + " && " + KpiCuts;
  TString cutsK3pi = CommonCuts + " && " + K3piCuts;
  
  Utils testUtils;
  
  for(int ij=0;ij<(argc-2);ij++) {
    TString particleName = argv[ij+2];
    TString rank   = "B0_M_rank==1";
    TString signal = particleName+"_isSignal==1";
    signal += " && TMath::Abs(" + particleName + "_genMotherPDG)==" + motherMap[particleName];
    TString trk    = (particleName=="B0")?"":particleName;
    cout<<endl<<" "<<particleName<<" efficiency and purity : signal cut:: "<<signal<<endl;
    
    cout<<" Kpi branch "<<endl;
    testUtils.printEffi(Kpi,MCKpi,cutsKpi,rank,signal,trk);
    cout<<" K3pi branch "<<endl;
    testUtils.printEffi(K3pi,MCK3pi,cutsK3pi,rank,signal,trk);
  }
  
  // f_sim->Close();
  file1->Close();
  system("rm -f "+outfile1);
  
  TFile *file  = new TFile(outfile,"recreate");
  file->Purge();
  file->Close();

  return 0;
}
