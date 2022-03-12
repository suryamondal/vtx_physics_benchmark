
#include "Utils.hh"

using namespace std;

Utils::Utils() {
  cout<<endl<<" Utils ready "<<endl;
}

Long64_t Utils::countTracks(TTree *tr, TTree *MCtr, TString trk) {

  /*
    function counts the number of unique tracks in all events
    tr   : input tree to calculate efficiency
    MCtr : input MC tree to calculate efficiency
    trk  : which track to look for, ie. pisoft, mu, K, etc.
    fromOtherBranch : tracks are reconstructed from other decay modes
  */

  const Long64_t maxexp = MCtr->GetMaximum("__experiment__") + 1;
  const Long64_t maxrun = MCtr->GetMaximum("__run__") + 1;
  const Long64_t maxevt = MCtr->GetMaximum("__event__") + 1;

  // bool isThisBranch[maxexp+1][maxrun+1][maxevt+1] = {0};
  vector<bool> isThisBranch(maxexp*maxrun*maxevt,0);
  // for(Long64_t ij=0;ij<int(isThisBranch.size());ij++) {
  //   cout<<ij<<" isThisBranch "<<isThisBranch[ij]<<endl;}
  
  Int_t exptn, runn, evtn;
  MCtr->SetBranchAddress("__experiment__",&exptn);
  MCtr->SetBranchAddress("__run__",&runn);
  MCtr->SetBranchAddress("__event__",&evtn);
  Long64_t MCentries = MCtr->GetEntries();
  for(Long64_t ij=0;ij<MCentries;ij++) {
    MCtr->GetEntry(ij);
    // cout<<ij<<" "<<runn<<" "<<evtn<<" "<<endl;
    Long64_t tpos = evtn + runn*maxevt + exptn*maxevt*maxrun;
    isThisBranch[tpos] = true;
    // cout<<"\ttpos "<<tpos<<" "<<isThisBranch[tpos]<<endl;
  }
  
  vector<int> expt, run, evt, index;
  int source = -10;
  Double_t indexn, sourcen;
  tr->SetBranchAddress("__experiment__",&exptn);
  tr->SetBranchAddress("__run__",&runn);
  tr->SetBranchAddress("__event__",&evtn);
  tr->SetBranchAddress(trk+"_mdstIndex",&indexn);
  tr->SetBranchAddress(trk+"_particleSource",&sourcen);

  // fromOtherBranch = 0;
  Long64_t sum = 0;
  Long64_t entries = tr->GetEntries();
  for(Long64_t ij=0;ij<entries;ij++) {
    tr->GetEntry(ij);
    // cout<<ij<<" "<<runn<<" "<<evtn<<" "<<indexn<<" "<<endl;
    
    Long64_t tpos = evtn + runn*maxevt + exptn*maxevt*maxrun;
    if(isThisBranch[tpos]==false) {continue;}
    
    int passORinsertORpush = -1;
    if(source<0) {source = sourcen; passORinsertORpush = 1;}

    Long64_t tsz = expt.size();
    if(tsz && (expt.back()!=exptn || run.back()!=runn)) {
      sum += tsz; tsz = 0; passORinsertORpush = 1;
      expt.clear(); run.clear(); evt.clear(); index.clear();
      // cout<<" sum: "<<sum<<endl;
    }
    
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
    
    // Long64_t tpos = evtn + runn*maxrun + exptn*maxexp;
    // // cout<<"\ttpos "<<tpos<<" "<<isThisBranch[tpos]<<endl;
    // if(passORinsertORpush>=0 && isThisBranch[tpos]==false) {
    //   fromOtherBranch++;
    //   // cout<<"\tfromOtherBranch "<<fromOtherBranch<<endl;
    // }
    
    if(passORinsertORpush<0) {
      continue;
    } else if(passORinsertORpush<tsz) {
      // cout<<ij<<" "<<runn<<" "<<evtn<<" "<<indexn<<" "<<endl;
      // cout<<"\t"<<tsz<<" "<<passORinsertORpush<<endl;
      // cout<<"\t\tinsert"<<endl;
      expt.insert(expt.begin()+passORinsertORpush,exptn);
      run.insert(run.begin()+passORinsertORpush,runn);
      evt.insert(evt.begin()+passORinsertORpush,evtn);
      index.insert(index.begin()+passORinsertORpush,indexn);
    } else {
      // cout<<ij<<" "<<runn<<" "<<evtn<<" "<<indexn<<" "<<endl;
      // cout<<"\t"<<tsz<<" "<<passORinsertORpush<<endl;
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
  // Long64_t nto=0, nso=0;
  
  if(trk=="") {
    nt  = tr->GetEntries(common);			// 
    ns  = tr->GetEntries(common+"&&"+signal);		// 
    nsb = tr->GetEntries(common+"&&"+signal+"&&"+rank); // 
    ntb = tr->GetEntries(common+"&&"+rank);	      // 
  } else {
    TTree *t_nt  = (TTree*)tr->CopyTree(common);
    cout<<" total copied entry: "<<(t_nt->GetEntries())<<endl;
    nt  = countTracks(t_nt,MCtr,trk);  // 
    TTree *t_ns  = (TTree*)tr->CopyTree(common+"&&"+signal);
    cout<<" signal copied entry: "<<(t_ns->GetEntries())<<endl;
    ns  = countTracks(t_ns,MCtr,trk);  // 
    TTree *t_nsb  = (TTree*)tr->CopyTree(common+"&&"+signal+"&&"+rank);
    cout<<" signal+rank copied entry: "<<(t_nsb->GetEntries())<<endl;
    nsb  = countTracks(t_nsb,MCtr,trk);  // 
    TTree *t_ntb  = (TTree*)tr->CopyTree(common+"&&"+rank);
    cout<<" rank copied entry: "<<(t_ntb->GetEntries())<<endl;
    ntb  = countTracks(t_ntb,MCtr,trk);  // 
  }
  // cout<<" nto "<<nto<<" nso "<<nso<<endl;
  
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

