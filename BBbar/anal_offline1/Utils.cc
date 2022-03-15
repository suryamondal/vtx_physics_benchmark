
#include "Utils.hh"

using namespace std;

Utils::Utils() {
  cout<<endl<<" Utils ready "<<endl;
}

Long64_t Utils::countTracks(ROOT::RDataFrame &tr, ROOT::RDataFrame &MCtr, TString trk, TString cuts) {
  
  /*
    function counts the number of unique tracks in all events
    tr   : input tree to calculate efficiency
    MCtr : input MC tree to calculate efficiency
    trk  : which track to look for, ie. pisoft, mu, K, etc.
    fromOtherBranch : tracks are reconstructed from other decay modes
  */
  
  auto nMC  = MCtr.Count();

  auto maxexp = MCtr.Max("__experiment__");
  auto maxrun = MCtr.Max("__run__");
  auto maxevt = MCtr.Max("__event__");
  
  auto expvecMC = MCtr.Take<Int_t>("__experiment__");
  auto runvecMC = MCtr.Take<Int_t>("__run__");
  auto evtvecMC = MCtr.Take<Int_t>("__event__");

  vector<bool> isThisBranch((*maxexp +1)*(*maxrun +1)*(*maxevt +1),0);
  // for(Long64_t ij=0;ij<int(isThisBranch.size());ij++) {
  //   cout<<ij<<" isThisBranch "<<isThisBranch[ij]<<endl;}

  for(Long64_t ij=0;ij<Long64_t(*nMC);ij++) {
    // cout<<ij<<" "<<vector(*runvecMC)[ij]<<" "<<vector(*evtvecMC)[ij]<<" "<<endl;
    Long64_t tpos = ( vector(*evtvecMC)[ij] +
		      vector(*runvecMC)[ij]*(*maxevt +1) +
		      vector(*expvecMC)[ij]*(*maxevt +1)*(*maxrun +1));
    isThisBranch[tpos] = true;
    // cout<<"\ttpos "<<tpos<<" "<<isThisBranch[tpos]<<endl;
  }
  
  auto entries  = tr.Filter(cuts.Data()).Count();
  auto expvec = tr.Filter(cuts.Data()).Take<Int_t>("__experiment__");
  auto runvec = tr.Filter(cuts.Data()).Take<Int_t>("__run__");
  auto evtvec = tr.Filter(cuts.Data()).Take<Int_t>("__event__");
  auto indvec = tr.Filter(cuts.Data()).Take<Double_t>((trk+"_mdstIndex").Data());
  auto srcvec = tr.Filter(cuts.Data()).Take<Double_t>((trk+"_particleSource").Data());
  
  vector<int> expt, run, evt, index;
  expt.clear(); run.clear(); evt.clear(); index.clear();
  int source = -10;
  
  Long64_t sum = 0;
  for(Long64_t ij=0;ij<Long64_t(*entries);ij++) {
    
    Long64_t tpos = ( vector(*evtvec)[ij] +
		      vector(*runvec)[ij]*(*maxevt +1) +
		      vector(*expvec)[ij]*(*maxevt +1)*(*maxrun +1));
    // cout<<ij<<" "<<vector(*expvec)[ij]<<" "<<vector(*runvec)[ij]<<" "<<vector(*evtvec)[ij]<<" "<<endl;
    // cout<<"\ttpos "<<tpos<<" "<<isThisBranch[tpos]<<endl;
    if(isThisBranch[tpos]==false) {continue;}
    
    int passORinsertORpush = -1;
    if(source<0) {source = vector(*srcvec)[ij]; passORinsertORpush = 1;}
    
    Long64_t tsz = expt.size();
    if(tsz && (expt.back()!=vector(*expvec)[ij] || run.back()!=vector(*runvec)[ij])) {
      sum += tsz; tsz = 0; passORinsertORpush = 1;
      expt.clear(); run.clear(); evt.clear(); index.clear();
      // cout<<" sum: "<<sum<<endl;
    }
    
    if(source==vector(*srcvec)[ij]) {
      for(int jk=tsz-1;jk>=0;jk--) {
  	if(expt[jk]==vector(*expvec)[ij] && run[jk]==vector(*runvec)[ij] &&
  	   evt[jk]==vector(*evtvec)[ij] && index[jk]==vector(*indvec)[ij]) {
  	  passORinsertORpush = -1; break;
  	} else if(expt[jk]<vector(*expvec)[ij] ||
  		  (expt[jk]==vector(*expvec)[ij] && run[jk]<vector(*runvec)[ij]) ||
  		  (expt[jk]==vector(*expvec)[ij] && run[jk]==vector(*runvec)[ij] &&
		   evt[jk]<vector(*evtvec)[ij]) ||
  		  (expt[jk]==vector(*expvec)[ij] && run[jk]==vector(*runvec)[ij] &&
		   evt[jk]==vector(*evtvec)[ij] && index[jk]<vector(*indvec)[ij])) {
  	  passORinsertORpush = jk + 1; break;}
  	passORinsertORpush = jk;
      } // for(int jk=tsz-1;jk>=0;jk++) {
    }
    
    // cout<<"\t"<<tsz<<" "<<passORinsertORpush<<endl;
    
    if(passORinsertORpush<0) {
      continue;
    } else if(passORinsertORpush<tsz) {
      // cout<<ij<<" "<<vector(*runvec)[ij]<<" "<<vector(*evtvec)[ij]<<" "<<vector(*indvec)[ij]<<" "<<endl;
      // cout<<"\t"<<tsz<<" "<<passORinsertORpush<<endl;
      // cout<<"\t\tinsert"<<endl;
      expt.insert(expt.begin()+passORinsertORpush,vector(*expvec)[ij]);
      run.insert(run.begin()+passORinsertORpush,vector(*runvec)[ij]);
      evt.insert(evt.begin()+passORinsertORpush,vector(*evtvec)[ij]);
      index.insert(index.begin()+passORinsertORpush,vector(*indvec)[ij]);
    } else {
      // cout<<ij<<" "<<vector(*runvec)[ij]<<" "<<vector(*evtvec)[ij]<<" "<<vector(*indvec)[ij]<<" "<<endl;
      // cout<<"\t"<<tsz<<" "<<passORinsertORpush<<endl;
      // cout<<"\t\tpush"<<endl;
      expt.push_back(vector(*expvec)[ij]);
      run.push_back(vector(*runvec)[ij]);
      evt.push_back(vector(*evtvec)[ij]);
      index.push_back(vector(*indvec)[ij]);
    }
    
  } // for(Long64_t ij=0;ij<entries;ij++) {
  
  sum += expt.size();
  return sum;
  
  // return 1;
  // return expt.size();
}


int Utils::printEffi(ROOT::RDataFrame &tr, ROOT::RDataFrame &MCtr, TString common,
		     TString rank, TString signal, TString trk) {
  
  /*
    tr    : input tree to calculate efficiency
    MCtr  : input MC tree to calculate efficiency
    common: common cuts
    rank  : rank cut
    signal: signal cut
    trk   : which track to look for, ie. pisoft, mu, K, etc.
  */
  
  auto nMC  = MCtr.Count();
  
  Long64_t nt, ns, nsb, ntb;
  
  if(trk=="") {
    auto nt1  = tr.Filter(common.Data()).Count();
    auto ns1  = tr.Filter((common+" && "+signal).Data()).Count();
    auto nsb1 = tr.Filter((common+" && "+signal+" && "+rank).Data()).Count();
    auto ntb1 = tr.Filter((common+" && "+rank).Data()).Count();
    nt = *nt1; ns = *ns1; nsb = *nsb1; ntb = *ntb1;
  } else {
    nt  = countTracks(tr,MCtr,trk,common);
    ns  = countTracks(tr,MCtr,trk,common+" && "+signal);
    nsb = countTracks(tr,MCtr,trk,common+" && "+signal+" && "+rank);
    ntb = countTracks(tr,MCtr,trk,common+" && "+rank);
  }
    
  if(nt<=0 || *nMC<=0) {return -1;}
  
  TString fs;
  cout << "             |  w/o bc  | w/ b.c.  " << endl;
  cout << "   ----------+----------+----------" << endl;
  fs.Form("     Total   |%10u|%10u", int(nt), int(ntb));
  cout << fs << endl;
  fs.Form("     Signal  |%10u|%10u", int(ns), int(nsb));
  cout << fs << endl;
  fs.Form("      MC     |%10u|%10u", int(*nMC), int(*nMC));
  cout << fs << endl;
  fs.Form("  Efficiency |%9.4g%%|%9.4g%%", 100.0 * (ns) / (*nMC), 100.0 * (nsb) / (*nMC));
  cout << fs << endl;
  fs.Form("    Purity   |%9.4g%%|%9.4g%%", 100.0 * (ns) / (nt), 100.0 * (nsb) / (ntb));
  cout << fs << endl;
  
  return 0;
} // void printEffi(TTree *tr, TTree *MCtr, TString common, TString rank, TString signal) {

