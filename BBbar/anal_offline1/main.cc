
#include "GlobalDefs.hh"
#include "Utils.hh"

using namespace std;

template<typename T, typename Allocator>
void printVector(const vector<T, Allocator> &vect) {
  for (const auto &i : vect) {cout << i << " ";}
  cout<<endl;}

int main(int argc, char **argv) {

  if(argc<=1) {return -1;}

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
  // TFile *file1  = new TFile(outfile1,"recreate");
  TFile *file  = new TFile(outfile,"recreate");

  ROOT::RDataFrame MCKpi("MCKpi",   argv[1]);
  ROOT::RDataFrame MCK3pi("MCK3pi", argv[1]);
  ROOT::RDataFrame Kpi("Kpi",       argv[1]);
  ROOT::RDataFrame K3pi("K3pi",     argv[1]);
  
  /* mcPT */
  auto B0mcPTKpi = MCKpi.Histo1D({"B0mcPTKpi","B0mcPTKpi",binN,0.,0.7},"B0_mcPT");
  auto B0mcPTK3pi = MCK3pi.Histo1D({"B0mcPTK3pi","B0mcPTK3pi",binN,0.,0.7},"B0_mcPT");

  auto mumcPTKpi = MCKpi.Histo1D({"mumcPTKpi","mumcPTKpi",binN,0.,2.5},"mu_mcPT");
  auto mumcPTK3pi = MCK3pi.Histo1D({"mumcPTK3pi","mumcPTK3pi",binN,0.,2.5},"mu_mcPT");

  auto pisoftmcPTKpi = MCKpi.Histo1D({"pisoftmcPTKpi","pisoftmcPTKpi",binN,0.,0.25},"pisoft_mcPT");
  auto pisoftmcPTK3pi = MCK3pi.Histo1D({"pisoftmcPTK3pi","pisoftmcPTK3pi",binN,0.,0.25},"pisoft_mcPT");

  auto KmcPTKpi = MCKpi.Histo1D({"KmcPTKpi","KmcPTKpi",binN,0.,2.5},"K_mcPT");
  auto KmcPTK3pi = MCK3pi.Histo1D({"KmcPTK3pi","KmcPTK3pi",binN,0.,2.5},"K_mcPT");
  
  /* mcTheta */
  auto B0mcThetaKpi = MCKpi.Histo1D({"B0mcThetaKpi","B0mcThetaKpi",binN,0.,TMath::Pi()},"B0_mcTheta");
  auto B0mcThetaK3pi = MCK3pi.Histo1D({"B0mcThetaK3pi","B0mcThetaK3pi",binN,0.,TMath::Pi()},"B0_mcTheta");

  auto mumcThetaKpi = MCKpi.Histo1D({"mumcThetaKpi","mumcThetaKpi",binN,0.,TMath::Pi()},"mu_mcTheta");
  auto mumcThetaK3pi = MCK3pi.Histo1D({"mumcThetaK3pi","mumcThetaK3pi",binN,0.,TMath::Pi()},"mu_mcTheta");

  auto pisoftmcThetaKpi = MCKpi.Histo1D({"pisoftmcThetaKpi","pisoftmcThetaKpi",binN,0.,TMath::Pi()},"pisoft_mcTheta");
  auto pisoftmcThetaK3pi = MCK3pi.Histo1D({"pisoftmcThetaK3pi","pisoftmcThetaK3pi",binN,0.,TMath::Pi()},"pisoft_mcTheta");

  auto KmcThetaKpi = MCKpi.Histo1D({"KmcThetaKpi","KmcThetaKpi",binN,0.,TMath::Pi()},"K_mcTheta");
  auto KmcThetaK3pi = MCK3pi.Histo1D({"KmcThetaK3pi","KmcThetaK3pi",binN,0.,TMath::Pi()},"K_mcTheta");
  
  
  /* Cuts for analysis */
  TString cutsKpi  = CommonCuts + " && " + KpiCuts;
  TString cutsK3pi = CommonCuts + " && " + K3piCuts;
  
  Utils testUtils;
  
  for(int ij=0;ij<(argc-1);ij++) {
    TString particleName = (ij==0)?"B0":argv[ij+1];
    TString rank   = "B0_M_rank==1";
    TString signal = particleName+"_isSignal==1";
    signal += " && TMath::Abs(" + particleName + "_genMotherPDG)==" + motherMap[particleName];
    TString trk    = (ij==0)?"":particleName;
    cout<<endl<<" "<<particleName<<" efficiency and purity : signal cut:: "<<signal<<endl;
    
    cout<<" Kpi branch "<<endl;
    testUtils.printEffi(Kpi,MCKpi,cutsKpi,rank,signal,trk,false);
    cout<<" K3pi branch "<<endl;
    testUtils.printEffi(K3pi,MCK3pi,cutsK3pi,rank,signal,trk,true);
  }
  
  // f_sim->Close();
  // file1->Close();
  // system("rm -f "+outfile1);
  
  // TFile *file  = new TFile(outfile,"recreate");
  
  /* writting histograms */
  file->cd();
  
  B0mcPTKpi->Write();
  B0mcPTK3pi->Write();
  mumcPTKpi->Write();
  mumcPTK3pi->Write();
  pisoftmcPTKpi->Write(); 
  pisoftmcPTK3pi->Write(); 
  KmcPTKpi->Write(); 
  KmcPTK3pi->Write(); 
  
  B0mcThetaKpi->Write();
  B0mcThetaK3pi->Write();
  mumcThetaKpi->Write();
  mumcThetaK3pi->Write();
  pisoftmcThetaKpi->Write(); 
  pisoftmcThetaK3pi->Write(); 
  KmcThetaKpi->Write(); 
  KmcThetaK3pi->Write(); 
  
  testUtils.B0mcPTKpi_sig->Write();
  testUtils.B0mcPTK3pi_sig->Write();
  testUtils.mumcPTKpi_sig->Write();
  testUtils.mumcPTK3pi_sig->Write();
  testUtils.pisoftmcPTKpi_sig->Write(); 
  testUtils.pisoftmcPTK3pi_sig->Write(); 
  testUtils.KmcPTKpi_sig->Write(); 
  testUtils.KmcPTK3pi_sig->Write(); 
  
  testUtils.B0mcThetaKpi_sig->Write();
  testUtils.B0mcThetaK3pi_sig->Write();
  testUtils.mumcThetaKpi_sig->Write();
  testUtils.mumcThetaK3pi_sig->Write();
  testUtils.pisoftmcThetaKpi_sig->Write(); 
  testUtils.pisoftmcThetaK3pi_sig->Write(); 
  testUtils.KmcThetaKpi_sig->Write(); 
  testUtils.KmcThetaK3pi_sig->Write(); 
  
  TH1D *B0mcPTKpi_sig_effi = (TH1D*)testUtils.B0mcPTKpi_sig->Clone("B0mcPTKpi_sig_effi");
  TH1D *B0mcPTK3pi_sig_effi = (TH1D*)testUtils.B0mcPTK3pi_sig->Clone("B0mcPTK3pi_sig_effi");
  TH1D *mumcPTKpi_sig_effi = (TH1D*)testUtils.mumcPTKpi_sig->Clone("mumcPTKpi_sig_effi");
  TH1D *mumcPTK3pi_sig_effi = (TH1D*)testUtils.mumcPTK3pi_sig->Clone("mumcPTK3pi_sig_effi");
  TH1D *pisoftmcPTKpi_sig_effi = (TH1D*)testUtils.pisoftmcPTKpi_sig->Clone("pisoftmcPTKpi_sig_effi"); 
  TH1D *pisoftmcPTK3pi_sig_effi = (TH1D*)testUtils.pisoftmcPTK3pi_sig->Clone("pisoftmcPTK3pi_sig_effi"); 
  TH1D *KmcPTKpi_sig_effi = (TH1D*)testUtils.KmcPTKpi_sig->Clone("KmcPTKpi_sig_effi"); 
  TH1D *KmcPTK3pi_sig_effi = (TH1D*)testUtils.KmcPTK3pi_sig->Clone("KmcPTK3pi_sig_effi"); 
  
  TH1D *B0mcThetaKpi_sig_effi = (TH1D*)testUtils.B0mcThetaKpi_sig->Clone("B0mcThetaKpi_sig_effi");
  TH1D *B0mcThetaK3pi_sig_effi = (TH1D*)testUtils.B0mcThetaK3pi_sig->Clone("B0mcThetaK3pi_sig_effi");
  TH1D *mumcThetaKpi_sig_effi = (TH1D*)testUtils.mumcThetaKpi_sig->Clone("mumcThetaKpi_sig_effi");
  TH1D *mumcThetaK3pi_sig_effi = (TH1D*)testUtils.mumcThetaK3pi_sig->Clone("mumcThetaK3pi_sig_effi");
  TH1D *pisoftmcThetaKpi_sig_effi = (TH1D*)testUtils.pisoftmcThetaKpi_sig->Clone("pisoftmcThetaKpi_sig_effi"); 
  TH1D *pisoftmcThetaK3pi_sig_effi = (TH1D*)testUtils.pisoftmcThetaK3pi_sig->Clone("pisoftmcThetaK3pi_sig_effi"); 
  TH1D *KmcThetaKpi_sig_effi = (TH1D*)testUtils.KmcThetaKpi_sig->Clone("KmcThetaKpi_sig_effi"); 
  TH1D *KmcThetaK3pi_sig_effi = (TH1D*)testUtils.KmcThetaK3pi_sig->Clone("KmcThetaK3pi_sig_effi"); 
  
  B0mcPTKpi_sig_effi->Divide(B0mcPTKpi_sig_effi,(TH1D*)B0mcPTKpi->Clone(),1,1,"B");
  B0mcPTK3pi_sig_effi->Divide(B0mcPTK3pi_sig_effi,(TH1D*)B0mcPTK3pi->Clone(),1,1,"B");
  mumcPTKpi_sig_effi->Divide(mumcPTKpi_sig_effi,(TH1D*)mumcPTKpi->Clone(),1,1,"B");
  mumcPTK3pi_sig_effi->Divide(mumcPTK3pi_sig_effi,(TH1D*)mumcPTK3pi->Clone(),1,1,"B");
  pisoftmcPTKpi_sig_effi->Divide(pisoftmcPTKpi_sig_effi,(TH1D*)pisoftmcPTKpi->Clone(),1,1,"B"); 
  pisoftmcPTK3pi_sig_effi->Divide(pisoftmcPTK3pi_sig_effi,(TH1D*)pisoftmcPTK3pi->Clone(),1,1,"B"); 
  KmcPTKpi_sig_effi->Divide(KmcPTKpi_sig_effi,(TH1D*)KmcPTKpi->Clone(),1,1,"B"); 
  KmcPTK3pi_sig_effi->Divide(KmcPTK3pi_sig_effi,(TH1D*)KmcPTK3pi->Clone(),1,1,"B"); 
  
  B0mcThetaKpi_sig_effi->Divide(B0mcThetaKpi_sig_effi,(TH1D*)B0mcThetaKpi->Clone(),1,1,"B");
  B0mcThetaK3pi_sig_effi->Divide(B0mcThetaK3pi_sig_effi,(TH1D*)B0mcThetaK3pi->Clone(),1,1,"B");
  mumcThetaKpi_sig_effi->Divide(mumcThetaKpi_sig_effi,(TH1D*)mumcThetaKpi->Clone(),1,1,"B");
  mumcThetaK3pi_sig_effi->Divide(mumcThetaK3pi_sig_effi,(TH1D*)mumcThetaK3pi->Clone(),1,1,"B");
  pisoftmcThetaKpi_sig_effi->Divide(pisoftmcThetaKpi_sig_effi,(TH1D*)pisoftmcThetaKpi->Clone(),1,1,"B"); 
  pisoftmcThetaK3pi_sig_effi->Divide(pisoftmcThetaK3pi_sig_effi,(TH1D*)pisoftmcThetaK3pi->Clone(),1,1,"B"); 
  KmcThetaKpi_sig_effi->Divide(KmcThetaKpi_sig_effi,(TH1D*)KmcThetaKpi->Clone(),1,1,"B"); 
  KmcThetaK3pi_sig_effi->Divide(KmcThetaK3pi_sig_effi,(TH1D*)KmcThetaK3pi->Clone(),1,1,"B"); 
  
  B0mcPTKpi_sig_effi->Write();
  B0mcPTK3pi_sig_effi->Write();
  mumcPTKpi_sig_effi->Write();
  mumcPTK3pi_sig_effi->Write();
  pisoftmcPTKpi_sig_effi->Write(); 
  pisoftmcPTK3pi_sig_effi->Write(); 
  KmcPTKpi_sig_effi->Write(); 
  KmcPTK3pi_sig_effi->Write(); 
  
  B0mcThetaKpi_sig_effi->Write();
  B0mcThetaK3pi_sig_effi->Write();
  mumcThetaKpi_sig_effi->Write();
  mumcThetaK3pi_sig_effi->Write();
  pisoftmcThetaKpi_sig_effi->Write(); 
  pisoftmcThetaK3pi_sig_effi->Write(); 
  KmcThetaKpi_sig_effi->Write(); 
  KmcThetaK3pi_sig_effi->Write(); 

  testUtils.B0mcPTKpi_sig_bc->Write();
  testUtils.B0mcPTK3pi_sig_bc->Write();
  testUtils.mumcPTKpi_sig_bc->Write();
  testUtils.mumcPTK3pi_sig_bc->Write();
  testUtils.pisoftmcPTKpi_sig_bc->Write(); 
  testUtils.pisoftmcPTK3pi_sig_bc->Write(); 
  testUtils.KmcPTKpi_sig_bc->Write(); 
  testUtils.KmcPTK3pi_sig_bc->Write(); 
  
  testUtils.B0mcThetaKpi_sig_bc->Write();
  testUtils.B0mcThetaK3pi_sig_bc->Write();
  testUtils.mumcThetaKpi_sig_bc->Write();
  testUtils.mumcThetaK3pi_sig_bc->Write();
  testUtils.pisoftmcThetaKpi_sig_bc->Write(); 
  testUtils.pisoftmcThetaK3pi_sig_bc->Write(); 
  testUtils.KmcThetaKpi_sig_bc->Write(); 
  testUtils.KmcThetaK3pi_sig_bc->Write(); 
  
  TH1D *B0mcPTKpi_sig_bc_effi = (TH1D*)testUtils.B0mcPTKpi_sig_bc->Clone("B0mcPTKpi_sig_bc_effi");
  TH1D *B0mcPTK3pi_sig_bc_effi = (TH1D*)testUtils.B0mcPTK3pi_sig_bc->Clone("B0mcPTK3pi_sig_bc_effi");
  TH1D *mumcPTKpi_sig_bc_effi = (TH1D*)testUtils.mumcPTKpi_sig_bc->Clone("mumcPTKpi_sig_bc_effi");
  TH1D *mumcPTK3pi_sig_bc_effi = (TH1D*)testUtils.mumcPTK3pi_sig_bc->Clone("mumcPTK3pi_sig_bc_effi");
  TH1D *pisoftmcPTKpi_sig_bc_effi = (TH1D*)testUtils.pisoftmcPTKpi_sig_bc->Clone("pisoftmcPTKpi_sig_bc_effi"); 
  TH1D *pisoftmcPTK3pi_sig_bc_effi = (TH1D*)testUtils.pisoftmcPTK3pi_sig_bc->Clone("pisoftmcPTK3pi_sig_bc_effi"); 
  TH1D *KmcPTKpi_sig_bc_effi = (TH1D*)testUtils.KmcPTKpi_sig_bc->Clone("KmcPTKpi_sig_bc_effi"); 
  TH1D *KmcPTK3pi_sig_bc_effi = (TH1D*)testUtils.KmcPTK3pi_sig_bc->Clone("KmcPTK3pi_sig_bc_effi"); 
  
  TH1D *B0mcThetaKpi_sig_bc_effi = (TH1D*)testUtils.B0mcThetaKpi_sig_bc->Clone("B0mcThetaKpi_sig_bc_effi");
  TH1D *B0mcThetaK3pi_sig_bc_effi = (TH1D*)testUtils.B0mcThetaK3pi_sig_bc->Clone("B0mcThetaK3pi_sig_bc_effi");
  TH1D *mumcThetaKpi_sig_bc_effi = (TH1D*)testUtils.mumcThetaKpi_sig_bc->Clone("mumcThetaKpi_sig_bc_effi");
  TH1D *mumcThetaK3pi_sig_bc_effi = (TH1D*)testUtils.mumcThetaK3pi_sig_bc->Clone("mumcThetaK3pi_sig_bc_effi");
  TH1D *pisoftmcThetaKpi_sig_bc_effi = (TH1D*)testUtils.pisoftmcThetaKpi_sig_bc->Clone("pisoftmcThetaKpi_sig_bc_effi"); 
  TH1D *pisoftmcThetaK3pi_sig_bc_effi = (TH1D*)testUtils.pisoftmcThetaK3pi_sig_bc->Clone("pisoftmcThetaK3pi_sig_bc_effi"); 
  TH1D *KmcThetaKpi_sig_bc_effi = (TH1D*)testUtils.KmcThetaKpi_sig_bc->Clone("KmcThetaKpi_sig_bc_effi"); 
  TH1D *KmcThetaK3pi_sig_bc_effi = (TH1D*)testUtils.KmcThetaK3pi_sig_bc->Clone("KmcThetaK3pi_sig_bc_effi"); 
  
  B0mcPTKpi_sig_bc_effi->Divide(B0mcPTKpi_sig_bc_effi,(TH1D*)B0mcPTKpi->Clone(),1,1,"B");
  B0mcPTK3pi_sig_bc_effi->Divide(B0mcPTK3pi_sig_bc_effi,(TH1D*)B0mcPTK3pi->Clone(),1,1,"B");
  mumcPTKpi_sig_bc_effi->Divide(mumcPTKpi_sig_bc_effi,(TH1D*)mumcPTKpi->Clone(),1,1,"B");
  mumcPTK3pi_sig_bc_effi->Divide(mumcPTK3pi_sig_bc_effi,(TH1D*)mumcPTK3pi->Clone(),1,1,"B");
  pisoftmcPTKpi_sig_bc_effi->Divide(pisoftmcPTKpi_sig_bc_effi,(TH1D*)pisoftmcPTKpi->Clone(),1,1,"B"); 
  pisoftmcPTK3pi_sig_bc_effi->Divide(pisoftmcPTK3pi_sig_bc_effi,(TH1D*)pisoftmcPTK3pi->Clone(),1,1,"B"); 
  KmcPTKpi_sig_bc_effi->Divide(KmcPTKpi_sig_bc_effi,(TH1D*)KmcPTKpi->Clone(),1,1,"B"); 
  KmcPTK3pi_sig_bc_effi->Divide(KmcPTK3pi_sig_bc_effi,(TH1D*)KmcPTK3pi->Clone(),1,1,"B"); 
  
  B0mcThetaKpi_sig_bc_effi->Divide(B0mcThetaKpi_sig_bc_effi,(TH1D*)B0mcThetaKpi->Clone(),1,1,"B");
  B0mcThetaK3pi_sig_bc_effi->Divide(B0mcThetaK3pi_sig_bc_effi,(TH1D*)B0mcThetaK3pi->Clone(),1,1,"B");
  mumcThetaKpi_sig_bc_effi->Divide(mumcThetaKpi_sig_bc_effi,(TH1D*)mumcThetaKpi->Clone(),1,1,"B");
  mumcThetaK3pi_sig_bc_effi->Divide(mumcThetaK3pi_sig_bc_effi,(TH1D*)mumcThetaK3pi->Clone(),1,1,"B");
  pisoftmcThetaKpi_sig_bc_effi->Divide(pisoftmcThetaKpi_sig_bc_effi,(TH1D*)pisoftmcThetaKpi->Clone(),1,1,"B"); 
  pisoftmcThetaK3pi_sig_bc_effi->Divide(pisoftmcThetaK3pi_sig_bc_effi,(TH1D*)pisoftmcThetaK3pi->Clone(),1,1,"B"); 
  KmcThetaKpi_sig_bc_effi->Divide(KmcThetaKpi_sig_bc_effi,(TH1D*)KmcThetaKpi->Clone(),1,1,"B"); 
  KmcThetaK3pi_sig_bc_effi->Divide(KmcThetaK3pi_sig_bc_effi,(TH1D*)KmcThetaK3pi->Clone(),1,1,"B"); 
  
  B0mcPTKpi_sig_bc_effi->Write();
  B0mcPTK3pi_sig_bc_effi->Write();
  mumcPTKpi_sig_bc_effi->Write();
  mumcPTK3pi_sig_bc_effi->Write();
  pisoftmcPTKpi_sig_bc_effi->Write(); 
  pisoftmcPTK3pi_sig_bc_effi->Write(); 
  KmcPTKpi_sig_bc_effi->Write(); 
  KmcPTK3pi_sig_bc_effi->Write(); 
  
  B0mcThetaKpi_sig_bc_effi->Write();
  B0mcThetaK3pi_sig_bc_effi->Write();
  mumcThetaKpi_sig_bc_effi->Write();
  mumcThetaK3pi_sig_bc_effi->Write();
  pisoftmcThetaKpi_sig_bc_effi->Write(); 
  pisoftmcThetaK3pi_sig_bc_effi->Write(); 
  KmcThetaKpi_sig_bc_effi->Write(); 
  KmcThetaK3pi_sig_bc_effi->Write(); 
  
  file->Purge();
  file->Close();

  return 0;
}
