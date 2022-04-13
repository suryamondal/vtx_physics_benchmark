
#include "Utils.hh"

using namespace std;

Utils::Utils() {}

void Utils::Setup(std::map<TString, TString> motherMap,

		  std::vector<TString> &particleNames,
		  std::vector<TString> &histoNames,
		  std::vector<std::vector<std::vector<Double_t>>> &histoBins,

		  std::vector<TString> &particleResoNames,
		  std::vector<std::vector<TString>> &histoResoNames,
		  std::vector<std::vector<std::vector<Double_t>>> &histoResoBins,

		  std::vector<TString> &particleResoFromPullNames,
		  std::vector<std::vector<TString>> &histoResoFromPullNames,
		  std::vector<std::vector<std::vector<Double_t>>> &histoResoFromPullBins,

		  // std::vector<TString> &particleResoFlightResoNames,
		  // std::vector<std::vector<TString>> &histoResoFlightResoNames,
		  // std::vector<std::vector<std::vector<Double_t>>> &histoResoFlightResoBins,

		  // std::vector<TString> &particleResoDecayResoNames,
		  // std::vector<std::vector<TString>> &histoResoDecayResoNames,
		  // std::vector<std::vector<std::vector<Double_t>>> &histoResoDecayResoBins,

		  TString chnl,
		  std::vector<TString> &paramNames
		  ) {
  
  histoTypes = {"tot","sig","sig_bc","bc"};
  
  parMotherMap = motherMap;
  
  paramList = paramNames;
  paramMap.clear();
  for(int ij=0;ij<int(paramNames.size());ij++) {
    paramMap.insert(pair<TString, Int_t>(paramNames[ij], ij));
  }
  
  particleList = particleNames;
  particleMap.clear();
  for(int ij=0;ij<int(particleNames.size());ij++) {
    particleMap.insert(pair<TString, Int_t>(particleNames[ij], ij));
  }
  
  histoList = histoNames;
  histoMap.clear();
  for(int ij=0;ij<int(histoNames.size());ij++) {
    histoMap.insert(pair<TString, Int_t>(histoNames[ij], ij));}
  histoBn = histoBins;
  
  particleResoList = particleResoNames;
  particleResoMap.clear();
  for(int ij=0;ij<int(particleResoNames.size());ij++) {
    particleResoMap.insert(pair<TString, Int_t>(particleResoNames[ij], ij));
  }
  
  histoResoList = histoResoNames;
  histoResoMap.clear();
  for(int ij=0;ij<int(histoResoNames.size());ij++) {
    histoResoMap.insert(pair<TString, Int_t>(histoResoNames[ij][0], ij));}
  histoResoBn = histoResoBins;

  particleResoFromPullList = particleResoFromPullNames;
  particleResoFromPullMap.clear();
  for(int ij=0;ij<int(particleResoFromPullNames.size());ij++) {
    particleResoFromPullMap.insert(pair<TString, Int_t>(particleResoFromPullNames[ij], ij));
  }
  
  histoResoFromPullList = histoResoFromPullNames;
  histoResoFromPullMap.clear();
  for(int ij=0;ij<int(histoResoFromPullNames.size());ij++) {
    histoResoFromPullMap.insert(pair<TString, Int_t>(histoResoFromPullNames[ij][0], ij));}
  histoResoFromPullBn = histoResoFromPullBins;
  
  channelName = chnl;
  
  for(int ijp=0;ijp<int(particleNames.size());ijp++) {
    for(int ijh=0;ijh<int(histoNames.size());ijh++) {
      TString name = (particleNames[ijp] + "_" + histoNames[ijh] + "_" + channelName);
      cout<<" mc new "<<name<<endl;
      histo_mc[ijp][ijh] = new TH1D(name.Data(),name.Data(),
				    int(histoBins[ijp][ijh][0]),histoBins[ijp][ijh][1],histoBins[ijp][ijh][2]);
    }}
  
  for(int ijp=0;ijp<int(particleNames.size());ijp++) {
    for(int ijh=0;ijh<int(histoNames.size());ijh++) {
      for(int ijb=0;ijb<int(histoTypes.size());ijb++) {
	TString name = (particleNames[ijp] + "_" + histoNames[ijh] + "_" + channelName
			+ "_" + histoTypes[ijb]);
	cout<<" mc new "<<name<<endl;
	histo_sig[ijp][ijh][ijb] = new TH1D(name.Data(),name.Data(),
					    int(histoBins[ijp][ijh][0]),histoBins[ijp][ijh][1],histoBins[ijp][ijh][2]);
      }}}
  
  for(int ijp=0;ijp<int(particleResoNames.size());ijp++) {
    for(int ijh=0;ijh<int(histoResoNames.size());ijh++) {
      for(int ijb=0;ijb<int(histoTypes.size());ijb++) {
	TString name = (particleResoNames[ijp] + "_" + histoResoNames[ijh][0] + "_" + channelName
			+ "_" + histoTypes[ijb] + "_reso");
	cout<<" mc reso new "<<name<<endl;
	histo_reso_sig[ijp][ijh][ijb] = new TH2D(name.Data(),name.Data(),
						 int(histoResoBins[ijp][ijh][0]),histoResoBins[ijp][ijh][1],histoResoBins[ijp][ijh][2],
						 int(histoResoBins[ijp][ijh][3]),histoResoBins[ijp][ijh][4],histoResoBins[ijp][ijh][5]);
      }}}

  for(int ijp=0;ijp<int(particleResoFromPullNames.size());ijp++) {
    for(int ijh=0;ijh<int(histoResoFromPullNames.size());ijh++) {
      for(int ijb=0;ijb<int(histoTypes.size());ijb++) {
	TString name = (particleResoFromPullNames[ijp] + "_" + histoResoFromPullNames[ijh][0] + "_" + channelName
			+ "_" + histoTypes[ijb] + "_reso");
	name.ReplaceAll("Pull","");
	cout<<" mc reso pull new "<<name<<endl;
	histo_resofrompull_sig[ijp][ijh][ijb] = new TH1D(name.Data(),name.Data(),
							 int(histoResoFromPullBins[ijp][ijh][0]),
							 histoResoFromPullBins[ijp][ijh][1],
							 histoResoFromPullBins[ijp][ijh][2]);
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
  
  for(int ijh=0;ijh<int(paramList.size());ijh++) {
    parVecList[ijh] = tr.Filter(cuts.Data()).Take<Double_t>((trk + "_" + paramList[ijh]).Data());
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
	if(particleMap.count(trk)==0) {continue;}
	histo_sig[particleMap[trk]][ijh][isBC]->Fill(vector(*parVecList[paramMap[histoList[ijh]]])[ij]);
      }
      
      for(int ijh=0;ijh<int(histoResoList.size());ijh++) {
	if(particleResoMap.count(trk)==0) {continue;}
	histo_reso_sig[particleResoMap[trk]][ijh][isBC]->Fill(vector(*parVecList[paramMap[histoResoList[ijh][0]]])[ij],
							      (vector(*parVecList[paramMap[histoResoList[ijh][0]]])[ij] -
							       vector(*parVecList[paramMap[histoResoList[ijh][1]]])[ij]));
      }
     
      for(int ijh=0;ijh<int(histoResoFromPullList.size());ijh++) {
	if(particleResoFromPullMap.count(trk)==0) {continue;}
	histo_resofrompull_sig[particleResoFromPullMap[trk]][ijh][isBC]->Fill(vector(*parVecList[paramMap[histoResoFromPullList[ijh][0]]])[ij] *
									      vector(*parVecList[paramMap[histoResoFromPullList[ijh][1]]])[ij]);
      }
      
    }
    
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
      if(name.Contains("_mc")) {
	TString parname = (particleList[ijp] + "_" + histoList[ijh]);
	cout<<" mc fill "<<name<<" "<<parname<<endl;
	cout<<"\t"<<histoBn[ijp][ijh][0]<<" "<<histoBn[ijp][ijh][1]<<" "<<histoBn[ijp][ijh][2]<<endl;
	auto thisto = MCtr.Histo1D({name.Data(),name.Data(),int(histoBn[ijp][ijh][0]),histoBn[ijp][ijh][1],histoBn[ijp][ijh][2]},
				   parname.Data());
	histo_mc[ijp][ijh] = (TH1D*)thisto->Clone();
      }}}
  
  Long64_t nt, ns, nsb, ntb;
  
  for(int ijp=0;ijp<int(particleList.size());ijp++) {
    
    TString trk = particleList[ijp];
    // TString signal = trk + "_isSignal==1";
    TString signal = "B0_isSignal==1";
    signal += " && TMath::Abs(" + trk + "_genMotherPDG)==" + parMotherMap[trk];
    cout<<endl<<" "<<trk<<" "<<particleMap[trk]<<" efficiency and purity : signal cut:: "<<signal<<endl;
    
    vector<TString> cutlist = {common,
			       common+" && "+signal,
			       common+" && "+signal+" && "+rank,
			       common+" && "+rank};
    if(trk=="B0") {
      auto nt1  = tr.Filter(cutlist[0].Data()).Count();
      auto ns1  = tr.Filter(cutlist[1].Data()).Count();
      auto nsb1 = tr.Filter(cutlist[2].Data()).Count();
      auto ntb1 = tr.Filter(cutlist[3].Data()).Count();
      
      // for(int ijh=0;ijh<int(histoList.size());ijh++) {
      // 	for(int ijb=0;ijb<int(histoTypes.size());ijb++) {
      // 	  TString name = ( trk + "_" + histoList[ijh] + "_" + channelName
      // 			   + "_" + histoTypes[ijb]);
      // 	  cout<<" reco fill "<<name<<endl;
      // 	  auto thisto = tr.Filter(cutlist[ijb].Data())
      // 	    .Histo1D({name.Data(),name.Data(),int(histoBn[ijp][ijh][0]),histoBn[ijp][ijh][1],histoBn[ijp][ijh][2]},
      // 		     (trk + "_" + histoList[ijh]).Data());
      // 	  histo_sig[ijp][ijh][ijb] = (TH1D*)thisto->Clone();
      // 	}}
      
      nt = *nt1; ns = *ns1; nsb = *nsb1; ntb = *ntb1;
    } else {
      nt  = countTracks(tr,MCtr,trk,cutlist[0],0);
      ns  = countTracks(tr,MCtr,trk,cutlist[1],1);
      nsb = countTracks(tr,MCtr,trk,cutlist[2],2);
      ntb = countTracks(tr,MCtr,trk,cutlist[3],3);
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
      for(int ijb=0;ijb<int(histoTypes.size());ijb++) {
	TString name = (particleList[ijp] + "_" + histoList[ijh] + "_" + channelName
			+ "_" + histoTypes[ijb]);
	cout<<" divide "<<name<<endl;
	histo_effi[ijp][ijh][ijb] = (TH1D*)histo_sig[ijp][ijh][ijb]->Clone(name+"_effi");
	histo_purity[ijp][ijh][ijb] = (TH1D*)histo_sig[ijp][ijh][ijb]->Clone(name+"_purity");
	histo_effi[ijp][ijh][ijb]->SetTitle(name+"_effi");
	histo_purity[ijp][ijh][ijb]->SetTitle(name+"_purity");
	if(name.Contains("_mc")) {
	  histo_effi[ijp][ijh][ijb]->Divide(histo_effi[ijp][ijh][ijb],histo_mc[ijp][ijh],1,1,"b");
	} else {
	  if(name.Contains("_sig_bc")) {
	    histo_purity[ijp][ijh][ijb]->Divide(histo_purity[ijp][ijh][ijb],histo_sig[ijp][ijh][3],1,1,"b");
	  } else if(name.Contains("_sig")) {
	    histo_purity[ijp][ijh][ijb]->Divide(histo_purity[ijp][ijh][ijb],histo_sig[ijp][ijh][0],1,1,"b");}}}}}
}
