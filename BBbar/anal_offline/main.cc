
#include "GlobalDefs.hh"
#include "Utils.hh"

using namespace std;

template<typename T, typename Allocator>
void printVector(const vector<T, Allocator> &vect) {
  for (const auto &i : vect) {cout << i << " ";}
  cout<<endl;}

int main(int argc, char **argv) {

  if(argc<=1) {return -1;}
  
  TString outfile = argv[1];
  outfile.ReplaceAll(".root","_myout.root");
  cout<<" infile: "<<argv[1]<<endl<<" outfile "<<outfile<<endl;

  TString outfile1 = argv[1];
  outfile1.ReplaceAll(".root","_myout1.root");
  
  TFile *file  = new TFile(outfile,"recreate");
  
  /* Cuts for analysis */
  TString cutsKpi  = CommonCuts + " && " + KpiCuts;
  TString cutsK3pi = CommonCuts + " && " + K3piCuts;
  
  TString cutsList[10];
  cutsList[0] = cutsKpi;
  cutsList[1] = cutsK3pi;

  TFile *f_sim = new TFile(argv[1],"READ");
  
  TString rank   = "B0_M_rank==1";
  
  Utils testUtils[10];
  for(int cn=0;cn<int(channelList.size());cn++) {
    vector<TString> tParList = particleList[cn];
    printVector(tParList);
    vector<TString> tParResoList = particleResoList[cn];
    printVector(tParResoList);
    vector<TString> tParResoFromPullList = particleResoFromPullList[cn];
    printVector(tParResoFromPullList);
    TTree *MCtr = (TTree*)f_sim->Get(("MC"+channelList[cn]).Data());
    TTree *RCtr = (TTree*)f_sim->Get((     channelList[cn]).Data());
    testUtils[cn].Setup(motherMap,
			tParList,histoList,histoBn,
			tParResoList,histoResoList,histoResoBn,
			tParResoFromPullList,histoResoFromPullList,histoResoFromPullBn,
			channelList[cn],
			RCtr, MCtr,
			parList);
    testUtils[cn].printEffi(RCtr,MCtr,cutsList[cn],rank);
  }
  
  /* writting histograms */
  file->cd();
  
  for(int cn=0;cn<int(channelList.size());cn++) {
    for(int ijp=0;ijp<int(testUtils[cn].particleList.size());ijp++) {
      for(int ijh=0;ijh<int(testUtils[cn].histoList.size());ijh++) {
	TString name = testUtils[cn].histo_mc[ijp][ijh]->GetName();
	if(name.Contains("_sig")) {
	  testUtils[cn].histo_mc[ijp][ijh]->Write();}}}
    
    for(int ijp=0;ijp<int(testUtils[cn].particleList.size());ijp++) {
      for(int ijh=0;ijh<int(testUtils[cn].histoList.size());ijh++) {
	for(int ijb=0;ijb<int(testUtils[cn].histoTypes.size());ijb++) {
	  TString name = testUtils[cn].histo_sig[ijp][ijh][ijb]->GetName();
	  if((name.Contains("_sig") && name.Contains("_mc")) || 
	     (!name.Contains("_sig") && !name.Contains("_mc"))) {
	    testUtils[cn].histo_sig[ijp][ijh][ijb]->Write();}}}}
    
    testUtils[cn].DivideHisto();
    for(int ijp=0;ijp<int(testUtils[cn].particleList.size());ijp++) {
      for(int ijh=0;ijh<int(testUtils[cn].histoList.size());ijh++) {
	for(int ijb=0;ijb<int(testUtils[cn].histoTypes.size());ijb++) {
	  TString name = testUtils[cn].histo_effi[ijp][ijh][ijb]->GetName();
	  if(name.Contains("_sig")) {
	    if(name.Contains("_mc")) {testUtils[cn].histo_effi[ijp][ijh][ijb]->Write();}
	    else {testUtils[cn].histo_purity[ijp][ijh][ijb]->Write();}}}}}
    
    for(int ijp=0;ijp<int(testUtils[cn].particleResoList.size());ijp++) {
      for(int ijh=0;ijh<int(testUtils[cn].histoResoList.size());ijh++) {
	for(int ijb=0;ijb<int(testUtils[cn].histoTypes.size());ijb++) {
	  TString name = testUtils[cn].histo_reso_sig[ijp][ijh][ijb]->GetName();
	  if(name.Contains("_sig")) {
	    testUtils[cn].histo_reso_sig[ijp][ijh][ijb]->Write();}}}}

    for(int ijp=0;ijp<int(testUtils[cn].particleResoFromPullList.size());ijp++) {
      for(int ijh=0;ijh<int(testUtils[cn].histoResoFromPullList.size());ijh++) {
	for(int ijb=0;ijb<int(testUtils[cn].histoTypes.size());ijb++) {
	  TString name = testUtils[cn].histo_resofrompull_sig[ijp][ijh][ijb]->GetName();
	  if(name.Contains("_sig")) {
	    testUtils[cn].histo_resofrompull_sig[ijp][ijh][ijb]->Write();}}}}
    
  }
  
  file->Purge();
  file->Close();

  return 0;
}
