
#include "Utils.hh"

using namespace std;

Utils::Utils() {
  cout<<endl<<" Utils ready "<<endl;
}

Long64_t Utils::countTracks(TTree *tr, TString trk) {

  /*
    tr   : input tree to calculate efficiency
    trk  : which track to look for, ie. pisoft, mu, K, etc.
  */
  
  vector<int> expt, run, evt, index;
  int source = -10;
  Double_t indexn, sourcen;
  Int_t exptn, runn, evtn;
  tr->SetBranchAddress("__experiment__",&exptn);
  tr->SetBranchAddress("__run__",&runn);
  tr->SetBranchAddress("__event__",&evtn);
  tr->SetBranchAddress(trk+"_mdstIndex",&indexn);
  tr->SetBranchAddress(trk+"_particleSource",&sourcen);

  Long64_t sum = 0;
  Long64_t entries = tr->GetEntries();
  for(Long64_t ij=0;ij<entries;ij++) {
    tr->GetEntry(ij);
    // cout<<ij<<" "<<runn<<" "<<evtn<<" "<<indexn<<" "<<endl;

    int passORinsertORpush = -1;
    if(source<0) {source = sourcen; passORinsertORpush = 1;}

    Long64_t tsz = expt.size();
    if(tsz && (expt.back()!=exptn || run.back()!=runn)) {
      sum += tsz; tsz = 0; passORinsertORpush = 1;
      expt.clear(); run.clear(); evt.clear(); index.clear();}

    if(source==sourcen) {
      for(int jk=tsz-1;jk>=0;jk--) {
	if(expt[jk]==exptn && run[jk]==runn &&
	   evt[jk]==evtn && index[jk]==indexn) {
	  passORinsertORpush = -1; break;
	} else if(expt[jk]<exptn ||
		  (expt[jk]==exptn && run[jk]<runn) ||
		  (expt[jk]==exptn && run[jk]==runn && evt[jk]<evtn) ||
		  (expt[jk]==exptn && run[jk]==runn && evt[jk]==evtn && index[jk]<indexn)) {
	  passORinsertORpush = jk + 1; break;}
	passORinsertORpush = jk;
      } // for(int jk=tsz-1;jk>=0;jk++) {
    } 

    // cout<<"\t"<<tsz<<" "<<passORinsertORpush<<endl;
    if(passORinsertORpush<0) {
      continue;
    } else if(passORinsertORpush<tsz) {
      // cout<<"\t\tinsert"<<endl;
      expt.insert(expt.begin()+passORinsertORpush,exptn);
      run.insert(run.begin()+passORinsertORpush,runn);
      evt.insert(evt.begin()+passORinsertORpush,evtn);
      index.insert(index.begin()+passORinsertORpush,indexn);
    } else {
      // cout<<"\t\tpush"<<endl;
      expt.push_back(exptn);
      run.push_back(runn);
      evt.push_back(evtn);
      index.push_back(indexn);
    }

  } // for(Long64_t ij=0;ij<entries;ij++) {

  sum += expt.size();
  return sum;

  // return expt.size();
}

int Utils::printEffi(TTree *tr, TTree *MCtr, TString common,
		     TString rank, TString signal, TString trk) {

  /*
    tr    : input tree to calculate efficiency
    MCtr  : input MC tree to calculate efficiency
    common: common cuts
    rank  : rank cut
    signal: signal cut
    trk   : which track to look for, ie. pisoft, mu, K, etc.
  */

  double nMC  = MCtr->GetEntries(); // 
  double nt, ns, nsb, ntb;	    // 

  if(trk=="") {
    nt  = tr->GetEntries(common);			// 
    ns  = tr->GetEntries(common+"&&"+signal);		// 
    nsb = tr->GetEntries(common+"&&"+signal+"&&"+rank); // 
    ntb = tr->GetEntries(common+"&&"+rank);		// 
  } else {
    TTree *t_nt  = (TTree*)tr->CopyTree(common);
    nt  = countTracks(t_nt,trk);  // 
    t_nt->Delete("all");
    TTree *t_ns  = (TTree*)tr->CopyTree(common+"&&"+signal);
    ns  = countTracks(t_ns,trk);  // 
    t_ns->Delete("all");
    TTree *t_nsb = (TTree*)tr->CopyTree(common+"&&"+signal+"&&"+rank);
    nsb = countTracks(t_nsb,trk); // 
    t_nsb->Delete("all");
    TTree *t_ntb = (TTree*)tr->CopyTree(common+"&&"+rank);
    ntb = countTracks(t_ntb,trk); // 
    t_ntb->Delete("all");
  }
  
  if(nt<=0 || nMC<=0) {return -1;}
  
  TString fs;
  cout << "             |  w/o bc  | w/ b.c.  " << endl;
  cout << "   ----------+----------+----------" << endl;
  fs.Form("     Total   |%10.0f|%10.0f", nt, ntb);
  cout << fs << endl;
  fs.Form("     Signal  |%10.0f|%10.0f", ns, nsb);
  cout << fs << endl;
  fs.Form("      MC     |%10u|%10u", int(nMC), int(nMC));
  cout << fs << endl;
  fs.Form("  Efficiency |%9.4g%%|%9.4g%%", 100.0 * ns / nMC, 100.0 * nsb / nMC);
  cout << fs << endl;
  fs.Form("    Purity   |%9.4g%%|%9.4g%%", 100.0 * ns / nt, 100.0 * nsb / ntb);
  cout << fs << endl;
  
  return 0;
} // void printEffi(TTree *tr, TTree *MCtr, TString common, TString rank, TString signal) {

