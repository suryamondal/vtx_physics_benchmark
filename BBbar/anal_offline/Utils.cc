
#include "Utils.hh"

using namespace std;

Utils::Utils() {}

void Utils::Setup(std::map<TString, TString> motherMap,
		  std::vector<TString> &particleNames,
		  std::vector<TString> &histoNames,
		  std::vector<std::vector<Int_t>> &histoBins,
		  std::vector<std::vector<Double_t>> &histoXmin,
		  std::vector<std::vector<Double_t>> &histoXmax,
		  TString chnl) {
  
  parMotherMap = motherMap;
  
  particleList.clear(); particleMap.clear();
  histoBn.clear(); histoXmn.clear(); histoXmx.clear();
  for(int ij=0;ij<int(particleNames.size());ij++) {
    particleList.push_back(particleNames[ij]);
    // cout<<particleList.back()<<endl;
    particleMap.insert(pair<TString, Int_t>(particleNames[ij], ij));
    histoBn.push_back(histoBins[ij]);
    histoXmn.push_back(histoXmin[ij]);
    histoXmx.push_back(histoXmax[ij]);}
  
  histoList.clear(); histoMap.clear();
  for(int ij=0;ij<int(histoNames.size());ij++) {
    histoList.push_back(histoNames[ij]);
    histoMap.insert(pair<TString, Int_t>(histoNames[ij], ij));}
  
  channelName = chnl;
  
  for(int ijp=0;ijp<int(particleNames.size());ijp++) {
    for(int ijh=0;ijh<int(histoNames.size());ijh++) {
      TString name = (particleNames[ijp] + "_" + histoNames[ijh] + "_" + channelName);
      cout<<" mc new "<<name<<endl;
      histo_mc[ijp][ijh] = new TH1D(name.Data(),name.Data(),
				    histoBins[ijp][ijh],histoXmin[ijp][ijh],histoXmax[ijp][ijh]);
    }}
  
  for(int ijp=0;ijp<int(particleNames.size());ijp++) {
    for(int ijh=0;ijh<int(histoNames.size());ijh++) {
      for(int ijb=0;ijb<2;ijb++) {
	TString name = (particleNames[ijp] + "_" + histoNames[ijh] + "_" + channelName
			+ "_sig" + (ijb==0?"":"_bc"));
	cout<<" mc new "<<name<<endl;
	histo_sig[ijp][ijh][ijb] = new TH1D(name.Data(),name.Data(),
					     histoBins[ijp][ijh],histoXmin[ijp][ijh],histoXmax[ijp][ijh]);
      }}}
  
  cout<<" Utils ready "<<endl<<endl;
  
}

Long64_t Utils::countTracks(ROOT::RDataFrame &tr,
			    ROOT::RDataFrame &MCtr,
			    TString trk,
			    TString cuts,
			    int isBC) {
  
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
  
  for(int ijh=0;ijh<int(histoList.size());ijh++) {
    parVecList[ijh] = tr.Filter(cuts.Data()).Take<Double_t>((trk + "_" + histoList[ijh]).Data());
  }
  
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
    
    if(isBC>=0) {
      for(int ijh=0;ijh<int(histoList.size());ijh++) {
	histo_sig[particleMap[trk]][ijh][isBC]->Fill(vector(*parVecList[ijh])[ij]);
      }}
    
  } // for(Long64_t ij=0;ij<entries;ij++) {
  
  sum += expt.size();
  return sum;
  
  // return 1;
  // return expt.size();
}


int Utils::printEffi(ROOT::RDataFrame &tr,
		     ROOT::RDataFrame &MCtr,
		     TString common,
		     TString rank) {
  
  /*
    tr    : input tree to calculate efficiency
    MCtr  : input MC tree to calculate efficiency
    common: common cuts
    rank  : rank cut
    signal: signal cut
    trk   : which track to look for, ie. pisoft, mu, K, etc.
  */
  
  auto nMC  = MCtr.Count();
  
  for(int ijp=0;ijp<int(particleList.size());ijp++) {
    for(int ijh=0;ijh<int(histoList.size());ijh++) {
      TString name = (particleList[ijp] + "_" + histoList[ijh] + "_" + channelName);
      TString parname = (particleList[ijp] + "_" + histoList[ijh]);
      cout<<" mc fill "<<name<<" "<<parname<<endl;
      cout<<"\t"<<histoBn[ijp][ijh]<<" "<<histoXmn[ijp][ijh]<<" "<<histoXmx[ijp][ijh]<<endl;
      auto thisto = MCtr.Histo1D({name.Data(),name.Data(),histoBn[ijp][ijh],histoXmn[ijp][ijh],histoXmx[ijp][ijh]},
				 parname.Data());
      histo_mc[ijp][ijh] = (TH1D*)thisto->Clone();
    }}
  
  Long64_t nt, ns, nsb, ntb;
  
  for(int ijp=0;ijp<int(particleList.size());ijp++) {
    
    TString trk = particleList[ijp];
    TString signal = trk + "_isSignal==1";
    signal += " && TMath::Abs(" + trk + "_genMotherPDG)==" + parMotherMap[trk];
    cout<<endl<<" "<<trk<<" "<<particleMap[trk]<<" efficiency and purity : signal cut:: "<<signal<<endl;
    
    if(trk=="B0") {
      auto nt1  = tr.Filter(common.Data()).Count();
      auto ns1  = tr.Filter((common+" && "+signal).Data()).Count();
      auto nsb1 = tr.Filter((common+" && "+signal+" && "+rank).Data()).Count();
      auto ntb1 = tr.Filter((common+" && "+rank).Data()).Count();
      
      for(int ijh=0;ijh<int(histoList.size());ijh++) {
	for(int ijb=0;ijb<2;ijb++) {
	  TString name = ( trk + "_" + histoList[ijh] + "_" + channelName
			   + "_sig" + (ijb==0?"":"_bc"));
	  cout<<" reco fill "<<name<<endl;
	  auto thisto = tr.Filter((common+" && "+signal).Data())
	    .Histo1D({name.Data(),name.Data(),histoBn[ijp][ijh],histoXmn[ijp][ijh],histoXmx[ijp][ijh]},
		     (trk + "_" + histoList[ijh]).Data());
	  histo_sig[ijp][ijh][ijb] = (TH1D*)thisto->Clone();
	}}
      
      nt = *nt1; ns = *ns1; nsb = *nsb1; ntb = *ntb1;
    } else {
      nt  = countTracks(tr,MCtr,trk,common, -1);
      ns  = countTracks(tr,MCtr,trk,common+" && "+signal, 0);
      nsb = countTracks(tr,MCtr,trk,common+" && "+signal+" && "+rank, 1);
      ntb = countTracks(tr,MCtr,trk,common+" && "+rank, -1);
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

  } // for(int ijp=0;ijp<int(particleList.size());ijp++) {
  
  return 0;
}


void Utils::DivideHisto() {
  for(int ijp=0;ijp<int(particleList.size());ijp++) {
    for(int ijh=0;ijh<int(histoList.size());ijh++) {
      for(int ijb=0;ijb<2;ijb++) {
	TString name = (particleList[ijp] + "_" + histoList[ijh] + "_" + channelName
			+ "_sig" + (ijb==0?"":"_bc") + "_effi");
	cout<<" divide "<<name<<endl;
	histo_effi[ijp][ijh][ijb] = (TH1D*)histo_sig[ijp][ijh][ijb]->Clone(name);
	histo_effi[ijp][ijh][ijb]->SetTitle(name);
	histo_effi[ijp][ijh][ijb]->Divide(histo_effi[ijp][ijh][ijb],histo_mc[ijp][ijh],1,1,"b");
      }}}
}
