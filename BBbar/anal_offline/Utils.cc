
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
		  TString chnl,
		  TTree *tree,
		  TTree *mctree,
		  std::map<TString, TString> &paramNames
		  ) {

  RCtr = (TTree*)tree->Clone("tree");
  MCtr = (TTree*)mctree->Clone("mctree");
  
  RCtr->SetBranchAddress("__experiment__",&expData);
  RCtr->SetBranchAddress("__run__",       &runData);
  RCtr->SetBranchAddress("__event__",     &evtData);

  MCtr->SetBranchAddress("__experiment__",&expDataMC);
  MCtr->SetBranchAddress("__run__",       &runDataMC);
  MCtr->SetBranchAddress("__event__",     &evtDataMC);
  
  histoTypes = {"tot","sig","sig_bc","bc"};
  
  parMotherMap = motherMap;

  int maxBranchNo =
    int(paramNames.size())*
    int(particleNames.size())*
    int(particleResoNames.size())*
    int(particleResoFromPullNames.size());

  branchDouble.reserve(maxBranchNo);
  branchInt.reserve(maxBranchNo);
  branchBool.reserve(maxBranchNo);

  paramList = paramNames;
  // paramList.clear();
  paramMap.clear();
  int cnt[10] = {0};
  for(auto it = paramNames.begin(); it!=paramNames.end(); it++) {
    // paramList[it->first] = it->second;
    
    for(int ijp=0;ijp<int(particleNames.size());ijp++) {
      makeBranch(particleNames[ijp],it->first,it->second, cnt);
    }

    for(int ijp=0;ijp<int(particleResoNames.size());ijp++) {
      makeBranch(particleResoNames[ijp],it->first,it->second, cnt);
    }
    
    for(int ijp=0;ijp<int(particleResoFromPullNames.size());ijp++) {
      makeBranch(particleResoFromPullNames[ijp],it->first,it->second, cnt);
    }
    
  }
  
  particleList = particleNames;
  // particleList.clear();
  particleMap.clear();
  for(int ij=0;ij<int(particleNames.size());ij++) {
    // particleList.push_back(particleNames[ij]);
    // cout<<" "<<particleNames[ij]<<endl;
    // particleMap.emplace(particleNames[ij],ij);
    particleMap[particleNames[ij]] = ij;
  }
  
  histoLists = histoNames;
  // histoLists.clear();
  histoMap.clear();
  for(int ij=0;ij<int(histoNames.size());ij++) {
    // histoLists.push_back(histoNames[ij]);
    histoMap[histoNames[ij]] = ij;}
  histoBn = histoBins;
  
  particleResoList = particleResoNames;
  // particleResoList.clear();
  particleResoMap.clear();
  for(int ij=0;ij<int(particleResoNames.size());ij++) {
    // particleResoList.push_back(particleResoNames[ij]);
    particleResoMap[particleResoNames[ij]] = ij;}
  
  histoResoList = histoResoNames;
  // histoResoList.clear();
  histoResoMap.clear();
  for(int ij=0;ij<int(histoResoNames.size());ij++) {
    // histoResoList.push_back(histoResoNames[ij]);
    histoResoMap[histoResoNames[ij][0]] = ij;}
  histoResoBn = histoResoBins;
  
  particleResoFromPullList = particleResoFromPullNames;
  // particleResoFromPullList.clear();
  particleResoFromPullMap.clear();
  for(int ij=0;ij<int(particleResoFromPullNames.size());ij++) {
    // particleResoFromPullList.push_back(particleResoFromPullNames[ij]);
    particleResoFromPullMap[particleResoFromPullNames[ij]] = ij;}
  
  histoResoFromPullList = histoResoFromPullNames;
  // histoResoFromPullList.clear();
  histoResoFromPullMap.clear();
  for(int ij=0;ij<int(histoResoFromPullNames.size());ij++) {
    // histoResoFromPullList.push_back(histoResoFromPullNames[ij]);
    histoResoFromPullMap[histoResoFromPullNames[ij][0]] = ij;}
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

Long64_t Utils::countTracks(TString trk,
			    TString cuts,
			    int isBC) {
  
  /*
    function counts the number of unique tracks in all events
    tr   : input tree to calculate efficiency
    MCtr : input MC tree to calculate efficiency
    trk  : which track to look for, ie. pisoft, mu, K, etc.
    fromOtherBranch : tracks are reconstructed from other decay modes
  */
  
  ROOT::RDataFrame mcRDF(*MCtr);
  
  auto nMC  = mcRDF.Count();

  auto maxexp = mcRDF.Max("__experiment__");
  auto maxrun = mcRDF.Max("__run__");
  auto maxevt = mcRDF.Max("__event__");
  
  auto expvecMC = mcRDF.Take<Int_t>("__experiment__");
  auto runvecMC = mcRDF.Take<Int_t>("__run__");
  auto evtvecMC = mcRDF.Take<Int_t>("__event__");
  
  vector<bool> isThisBranch((*maxexp +1)*(*maxrun +1)*(*maxevt +1),0);
  
  for(Long64_t ij=0;ij<Long64_t(*nMC);ij++) {
    // cout<<ij<<" "<<vector(*runvecMC)[ij]<<" "<<vector(*evtvecMC)[ij]<<" "<<endl;
    Long64_t tpos = ( vector(*evtvecMC)[ij] +
		      vector(*runvecMC)[ij]*(*maxevt +1) +
		      vector(*expvecMC)[ij]*(*maxevt +1)*(*maxrun +1));
    isThisBranch[tpos] = true;
    // cout<<"\ttpos "<<tpos<<" "<<isThisBranch[tpos]<<endl;
  }

  vector<int> expt, run, evt, index;
  expt.clear(); run.clear(); evt.clear(); index.clear();
  int source = -10;
  
  Long64_t sum = 0;
  Long64_t nentries = RCtr->GetEntries();
  for(Long64_t ij=0;ij<nentries;ij++) {
    RCtr->GetEntry(ij);
    if(ij%1000==0) {cout<<(nentries-ij)<<endl;}
    
    Long64_t tpos = ( evtData +
		      runData*(*maxevt +1) +
		      expData*(*maxevt +1)*(*maxrun +1));
    // cout<<ij<<" "<<expData<<" "<<runData<<" "<<evtData<<" "<<endl;
    // cout<<"\ttpos "<<tpos<<" "<<isThisBranch[tpos]<<endl;
    if(isThisBranch[tpos]==false) {continue;}
    
    int passORinsertORpush = -1;
    if(source<0) {source = getDataValue(trk+"_particleSource"); passORinsertORpush = 1;}
    
    Long64_t tsz = expt.size();
    if(tsz && (expt.back()!=expData || run.back()!=runData)) {
      sum += tsz; tsz = 0; passORinsertORpush = 1;
      expt.clear(); run.clear(); evt.clear(); index.clear();
      // cout<<" sum: "<<sum<<endl;
    }
    
    if(source==getDataValue(trk+"_particleSource")) {
      for(int jk=tsz-1;jk>=0;jk--) {
  	if(expt[jk]==expData && run[jk]==runData &&
  	   evt[jk]==evtData && index[jk]==getDataValue(trk+"_mdstIndex")) {
  	  passORinsertORpush = -1; break;
	} else if(expt[jk]<expData ||
  		  (expt[jk]==expData && run[jk]<runData) ||
  		  (expt[jk]==expData && run[jk]==runData &&
		   evt[jk]<evtData) ||
  		  (expt[jk]==expData && run[jk]==runData &&
		   evt[jk]==evtData && index[jk]<getDataValue(trk+"_mdstIndex"))) {
  	  passORinsertORpush = jk + 1; break;}
  	passORinsertORpush = jk;
      } // for(int jk=tsz-1;jk>=0;jk++) {
    }
    
    // cout<<"\t"<<tsz<<" "<<passORinsertORpush<<endl;
    
    if(passORinsertORpush<0) {
      continue;
    } else if(passORinsertORpush<tsz) {
      // cout<<ij<<" "<<runData<<" "<<evtData<<" "<<getDataValue(trk+"_mdstIndex")<<" "<<endl;
      // cout<<"\t"<<tsz<<" "<<passORinsertORpush<<endl;
      // cout<<"\t\tinsert"<<endl;
      expt.insert(expt.begin()+passORinsertORpush,expData);
      run.insert(run.begin()+passORinsertORpush,runData);
      evt.insert(evt.begin()+passORinsertORpush,evtData);
      index.insert(index.begin()+passORinsertORpush,int(getDataValue(trk+"_mdstIndex")));
    } else {
      // cout<<ij<<" "<<runData<<" "<<evtData<<" "<<getDataValue(trk+"_mdstIndex")<<" "<<endl;
      // cout<<"\t"<<tsz<<" "<<passORinsertORpush<<endl;
      // cout<<"\t\tpush"<<endl;
      expt.push_back(expData);
      run.push_back(runData);
      evt.push_back(evtData);
      index.push_back(int(getDataValue(trk+"_mdstIndex")));
    }
    
    if(isBC>=0) {
      for(int ijh=0;ijh<int(histoLists.size());ijh++) {
      	// cout<<" trk "<<trk<<" "<<particleMap[trk]<<" "<<histoLists[ijh]<<endl;
      	if(particleMap.find(trk)==particleMap.end()) {continue;}
      	histo_sig[particleMap[trk]][ijh][isBC]->
      	  Fill(getDataValue(trk+"_"+histoLists[ijh]));
      }
      
      for(int ijh=0;ijh<int(histoResoList.size());ijh++) {
    	if(particleResoMap.find(trk)==particleResoMap.end()) {continue;}
    	histo_reso_sig[particleResoMap[trk]][ijh][isBC]->
	  Fill(getDataValue(trk+"_"+histoResoList[ijh][0]),
	       getDataValue(trk+"_"+histoResoList[ijh][0]) -
	       getDataValue(trk+"_"+histoResoList[ijh][1]));
      }
      
      for(int ijh=0;ijh<int(histoResoFromPullList.size());ijh++) {
    	if(particleResoFromPullMap.find(trk)==particleResoFromPullMap.end()) {continue;}
    	histo_resofrompull_sig[particleResoFromPullMap[trk]][ijh][isBC]->
	  Fill(getDataValue(trk+"_"+histoResoFromPullList[ijh][0]) *
	       getDataValue(trk+"_"+histoResoFromPullList[ijh][1]));
      }
      
    }
    
  } // for(Long64_t ij=0;ij<entries;ij++) {
  
  sum += expt.size();
  return sum;
  
  // return 1;
  // return expt.size();
}


int Utils::printEffi(TString common,
		     TString rank) {
  
  /*
    tr    : input tree to calculate efficiency
    MCtr  : input MC tree to calculate efficiency
    common: common cuts
    rank  : rank cut
    signal: signal cut
    trk   : which track to look for, ie. pisoft, mu, K, etc.
  */
  
  ROOT::RDataFrame mcRDF(*MCtr);
  auto nMC  = mcRDF.Count();
  
  for(int ijp=0;ijp<int(particleList.size());ijp++) {
    for(int ijh=0;ijh<int(histoLists.size());ijh++) {
      TString name = (particleList[ijp] + "_" + histoLists[ijh] + "_" + channelName);
      if(name.Contains("_mc")) {
	TString parname = (particleList[ijp] + "_" + histoLists[ijh]);
	cout<<" mc fill "<<name<<" "<<parname<<endl;
	cout<<"\t"<<histoBn[ijp][ijh][0]<<" "<<histoBn[ijp][ijh][1]<<" "<<histoBn[ijp][ijh][2]<<endl;
	auto thisto = mcRDF.Histo1D({name.Data(),name.Data(),int(histoBn[ijp][ijh][0]),histoBn[ijp][ijh][1],histoBn[ijp][ijh][2]},
				   parname.Data());
	histo_mc[ijp][ijh] = (TH1D*)thisto->Clone();
      }}}
  
  Long64_t nt, ns, nsb, ntb;
  
  for(int ijp=0;ijp<int(particleList.size());ijp++) {
    
    TString trk = particleList[ijp];
    TString signal = trk + "_isSignal==1";
    signal += " && TMath::Abs(" + trk + "_genMotherPDG)==" + parMotherMap[trk];
    cout<<endl<<" "<<trk<<" "<<particleMap[trk]<<" efficiency and purity : signal cut:: "<<signal<<endl;
    
    vector<TString> cutlist = {common,
			       common+" && "+signal,
			       common+" && "+signal+" && "+rank,
			       common+" && "+rank};
    if(trk=="B0") {
      // auto nt1  = tr.Filter(cutlist[0].Data()).Count();
      // auto ns1  = tr.Filter(cutlist[1].Data()).Count();
      // auto nsb1 = tr.Filter(cutlist[2].Data()).Count();
      // auto ntb1 = tr.Filter(cutlist[3].Data()).Count();
      
      // for(int ijh=0;ijh<int(histoLists.size());ijh++) {
      // 	for(int ijb=0;ijb<int(histoTypes.size());ijb++) {
      // 	  TString name = ( trk + "_" + histoLists[ijh] + "_" + channelName
      // 			   + "_" + histoTypes[ijb]);
      // 	  cout<<" reco fill "<<name<<endl;
      // 	  auto thisto = tr.Filter(cutlist[ijb].Data())
      // 	    .Histo1D({name.Data(),name.Data(),int(histoBn[ijp][ijh][0]),histoBn[ijp][ijh][1],histoBn[ijp][ijh][2]},
      // 		     (trk + "_" + histoLists[ijh]).Data());
      // 	  histo_sig[ijp][ijh][ijb] = (TH1D*)thisto->Clone();
      // 	}}
      
      // nt = *nt1; ns = *ns1; nsb = *nsb1; ntb = *ntb1;
    } else {
      nt  = countTracks(trk,cutlist[0],0);
      ns  = countTracks(trk,cutlist[1],1);
      nsb = countTracks(trk,cutlist[2],2);
      ntb = countTracks(trk,cutlist[3],3);
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
    for(int ijh=0;ijh<int(histoLists.size());ijh++) {
      for(int ijb=0;ijb<int(histoTypes.size());ijb++) {
	TString name = (particleList[ijp] + "_" + histoLists[ijh] + "_" + channelName
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

void Utils::makeBranch(TString partname,
		       TString parname,
		       TString type,
		       int *cnt) {
  TString name = (partname + "_" + parname);
  if(VariableDataType.find(type) == VariableDataType.end()) {
    cout<<" data type not found for "<<name<<endl;}
  
  int datatype = VariableDataType[type];
  if(paramMap.find(name) == paramMap.end()) {
    cout<<" name "<<name<<" type "<<type<<" "<<datatype<<" cnt "<<cnt[datatype]<<endl;
    if(name.Contains("_mc")) {
      if(type=="c_double") {
	branchDouble.push_back(0);
	MCtr->SetBranchAddress(name,&branchDouble[cnt[datatype]]);
      } else if(type=="c_int") {
      	branchInt.push_back(0);
      	MCtr->SetBranchAddress(name,&branchInt[cnt[datatype]]);
      } else if(type=="c_bool") {
      	branchBool.push_back(0);
      	MCtr->SetBranchAddress(name,&branchBool[cnt[datatype]]);
      }
    } else {
      if(type=="c_double") {
	branchDouble.push_back(0);
	RCtr->SetBranchAddress(name,&branchDouble[cnt[datatype]]);
      } else if(type=="c_int") {
	branchInt.push_back(0);
	RCtr->SetBranchAddress(name,&branchInt[cnt[datatype]]);
      } else if(type=="c_bool") {
      	branchBool.push_back(0);
      	RCtr->SetBranchAddress(name,&branchBool[cnt[datatype]]);
      }
    }
    paramMap[name] = std::pair<TString,Int_t>(type,cnt[datatype]);
    cnt[datatype]++;
  }
}

double Utils::getDataValue(TString brdetails1)
{
  // cout<<" getDataValue "<<brdetails1<<" "<<paramMap[brdetails1].first<<" "<<paramMap[brdetails1].second<<endl;
  if(paramMap[brdetails1].first == "c_double") {
    return branchDouble[paramMap[brdetails1].second];
  } else if(paramMap[brdetails1].first == "c_int") {
    return branchInt[paramMap[brdetails1].second];
  } else if(paramMap[brdetails1].first == "c_bool") {
    return branchBool[paramMap[brdetails1].second];
  } else {
    return std::numeric_limits<double>::quiet_NaN();
  }
}
