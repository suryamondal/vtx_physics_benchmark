#ifndef GLOBALLIBS_H
#define GLOBALLIBS_H


#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <ctime>
#include <bitset>
#include <map>
#include <algorithm>
#include <string>
#include <functional>

#include "TTimeStamp.h"
#include "TH1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TTree.h"
#include "TFile.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TObject.h"
#include "TRandom.h"
#include "TVector2.h"
#include "TVector3.h"
#include "TGraph.h"
#include "TF1.h"
#include "TMinuit.h"
#include "TString.h"
#include <ROOT/RDataFrame.hxx>


/** data type of variables */
static std::map<TString, int> VariableDataType =
  {{"c_double", 0},
   {"c_int",    1},
   {"c_bool",   2}
  };


#endif
