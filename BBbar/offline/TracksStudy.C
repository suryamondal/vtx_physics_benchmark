/** ROOT macro for studying tracks.
 * Usage: root -l -b -q "TracksStudy.C(\"path/to/file.root\")"
 */
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TColor.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TH1.h>
#include <THStack.h>
#include <TLegend.h>
#include <TLatex.h>
#include <TLine.h>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <map>
using namespace std;

#define STRNG(x) #x
#define STRNG2(x) STRNG(x)
#define CHECK(assertion) if(!(assertion)) throw runtime_error("At " STRNG2(__FILE__) ":" STRNG2(__LINE__))

const auto MyRed = TColor::GetColor("#E24A33");
const auto MyBlue = TColor::GetColor("#348ABD");

typedef tuple<Int_t,Int_t,Int_t> EvtID; /**< EvtID = {exp, run, evt} */

const initializer_list<int> PDGIDs {11, 13, 211, 321, 2212, 0};
const map<int,TString> PDGTitles {
  {0, "Others"},  {11, "Electrons"}, {13, "Muons"}, {211, "Pions"},
  {321, "Kaons"}, {2212, "Protons"}};

template <class T> class PerParticleStat {
public:
  T electrons, muons, pions, kaons, protons, others; // Direct access to particle variables
  PerParticleStat() : electrons(), muons(), pions(), kaons(), protons(), others() {}

  // Access particle variables by pdgID (absolute value is used)
  T& operator[](int pdgID) {
    switch (abs(pdgID)) {
      case 11: return electrons; break;
      case 13: return muons; break;
      case 211: return pions; break;
      case 321: return kaons; break;
      case 2212: return protons; break;
      default: return others; break;
    }
  }
  const T& operator[](int pdgID) const {
    switch (abs(pdgID)) {
      case 11: return electrons; break;
      case 13: return muons; break;
      case 211: return pions; break;
      case 321: return kaons; break;
      case 2212: return protons; break;
      default: return others; break;
    }
  }

  T total() const { return electrons + muons + pions + kaons + protons + others; }
};

template<> class PerParticleStat<TH1F> {
public:
  TH1F electrons, muons, pions, kaons, protons, others; // Direct access to particle variables
  PerParticleStat() = delete;
  PerParticleStat(TString name, TString title, int nBins, double xLow, double xUp)
  : electrons(name + "_e", title, nBins, xLow, xUp),
    muons(name + "_mu", title, nBins, xLow, xUp),
    pions(name + "_pi", title, nBins, xLow, xUp),
    kaons(name + "_K", title, nBins, xLow, xUp),
    protons(name + "_p", title, nBins, xLow, xUp),
    others(name + "_other", title, nBins, xLow, xUp) {}

  // Access particle variables by pdgID (absolute value is used)
  TH1F& operator[](int pdgID) {
    switch (abs(pdgID)) {
      case 11: return electrons; break;
      case 13: return muons; break;
      case 211: return pions; break;
      case 321: return kaons; break;
      case 2212: return protons; break;
      default: return others; break;
    }
  }
  const TH1F& operator[](int pdgID) const {
    switch (abs(pdgID)) {
      case 11: return electrons; break;
      case 13: return muons; break;
      case 211: return pions; break;
      case 321: return kaons; break;
      case 2212: return protons; break;
      default: return others; break;
    }
  }

  TH1F* total() const {
    TString newName = electrons.GetName();
    newName = newName(0, newName.Length() - 1) + "total";
    TH1F* tot = (TH1F*)electrons.Clone(newName);
    tot->Add(&muons);
    tot->Add(&pions);
    tot->Add(&kaons);
    tot->Add(&protons);
    tot->Add(&others);
    return tot;
  }
};

typedef struct EvtTracksData {
  PerParticleStat<int> nTracks, nCloneTracks, nBadTracks;
} EvtTracksData;

typedef struct CandidateData {
  double B0_M, B0_chiprob, Dst_M, Dst_pCM, D0_M;
  double Dst_massDiff() const { return Dst_M - D0_M; }
  double B0_dM() const { return TMath::Abs(B0_M - 5.27965); }
  double Dst_dM() const { return TMath::Abs(Dst_M - 2.01026); }
  double D0_dM() const { return TMath::Abs(D0_M - 1.86484); }
  double Dst_dMassDiff() const { return TMath::Abs(Dst_dM() - 0.1454258); }
} CandidateData;

typedef struct EvtCandidatesData {
  int nSignalCandidates;
  CandidateData signalData;
  vector<CandidateData> misrecosData;
  EvtCandidatesData() : nSignalCandidates(0) {}
} EvtCandidatesData;

class PDFCanvas {
public:
  PDFCanvas() = delete;
  PDFCanvas(const PDFCanvas&) = delete;
  PDFCanvas(PDFCanvas&&) = delete;
  PDFCanvas& operator=(const PDFCanvas&) = delete;
  PDFCanvas& operator=(PDFCanvas&&) = delete;
  PDFCanvas(TString name, TString PDFName) : outName(PDFName), c(name, name, 640, 480) {
    c.Print(outName + "[");
  }
  ~PDFCanvas() { c.Print(outName + "]"); }

  void Print() { c.Print(outName); }
  void Print(TString title = "") { c.Print(outName, "Title:" + title); }

  TCanvas* operator->() { return &c; }
  TCanvas* operator*() { return &c; }

  void PrintHistos(TString title, initializer_list<tuple<TH1*,TString,Color_t>> histos, double max = 0.0) {
    TH1* h0 = get<0>(*histos.begin());
    THStack hs("hs", title + ";" + h0->GetXaxis()->GetTitle() + ";" + h0->GetYaxis()->GetTitle());
    TLegend leg(0.8, 0.91 - 0.05 * histos.size(), 0.98, 0.91);
    int i = 0;
    for (const auto& t : histos) {
      TH1* h = get<0>(t); const TString& label = get<1>(t); Color_t color = get<2>(t);
      h->SetLineColor(color);
      hs.Add(h, "hist");
      leg.AddEntry(h, label, "L");
    }
    if (max > 0.0)
      hs.SetMaximum(max);
    c.cd();
    hs.Draw("nostack");
    leg.Draw();
    Print(title);
  }

  void PrintHisto(TH1* h, bool line = false, double max = 0.0) {
    c.cd();
    h->SetFillColor(MyBlue);
    h->SetLineColor(kBlack);
    if (max > 0.0)
      h->SetMaximum(max);
    h->Draw("hist");
    if (line) {
      TLine ln(0, h->GetMinimum(), 0, h->GetMaximum() * (1 + gStyle->GetHistTopMargin()));
      ln.SetLineColor(MyRed);
      ln.Draw();
      TLatex latex;
      latex.SetTextSize(0.025);
      int zeroBin;
      for (zeroBin = 1; zeroBin < h->GetNcells(); zeroBin++)
        if (h->GetBinLowEdge(zeroBin) >= 0) break;
      const double nBelow = h->Integral(0, zeroBin - 1);
      const double nAbove = h->Integral(zeroBin, h->GetNcells() - 1);
      const double nTot = h->GetEntries();
      latex.DrawLatexNDC(0.25, 0.8, TString::Format("%.0lf", nBelow));
      latex.DrawLatexNDC(0.25, 0.75, TString::Format("%.0lf%%", nBelow / nTot * 100.0));
      latex.DrawLatexNDC(0.75, 0.8, TString::Format("%.0lf", nAbove));
      latex.DrawLatexNDC(0.75, 0.75, TString::Format("%.0lf%%", nAbove / nTot * 100.0));
      Print(h->GetTitle());
    } else
      Print(h->GetTitle());
  }

private:
  TString outName;
  TCanvas c;
};

void TracksStudy(TString filePath)
{
  TFile* file = TFile::Open(filePath);
  CHECK(file);

  const TString outFileName = filePath(0, filePath.Length() - 5) + "_tracks.pdf";
  gStyle->SetOptStat(0);
  gStyle->SetHistLineWidth(2);
  PDFCanvas c("c", outFileName);

  { // Tracks stuff
    PerParticleStat<TH1F> tracksVsPT("vsPT", "Tracks vs p_{T};p_{T} [GeV/c];Tracks / bin", 100, 0, 3),
                          clonesVsPT("clonesVsPT", "Clone tracks vs p_{T};p_{T} [GeV/c];Tracks / bin", 100, 0, 3),
                          badsVsPT("badsVsPT", "Bad tracks vs p_{T};p_{T} [GeV/c];Tracks / bin", 100, 0, 3),
                          nTracks("nGood", "Good tracks per event;Number of tracks;Events / bin", 25, -0.5, 24.5),
                          nClones("nClones", "Clone tracks per event;Number of tracks;Events / bin", 25, -0.5, 24.5),
                          nBad("nBad", "Bad tracks per event;Number of tracks;Events / bin", 25, -0.5, 24.5);
    TH1F totalTracks("nGood", "Good tracks per event;Number of tracks;Events / bin", 25, -0.5, 24.5),
         totalClones("nClones", "Clone tracks per event;Number of tracks;Events / bin", 25, -0.5, 24.5),
         totalBad("nBad", "Bad tracks per event;Number of tracks;Events / bin", 25, -0.5, 24.5);
    { // Loop over ntuple and gather data in histograms
      map<EvtID,EvtTracksData> perEventData;

      TTree* tree = file->Get<TTree>("Tracks");
      CHECK(tree);
      TTreeReader rdr(tree);
      TTreeReaderValue<Double_t> mcPDG(rdr, "mcPDG"),
                                isSignal(rdr, "isSignalAcceptWrongFSPs"),
                                isCloneTrack(rdr, "isCloneTrack"),
                                mcPT(rdr, "mcPT");
      TTreeReaderValue<Int_t>    exp(rdr, "__experiment__"),
                                run(rdr, "__run__"),
                                evt(rdr, "__event__");
      while (rdr.Next()) {
        EvtID evtid = make_tuple(*exp, *run, *evt);
        int pdg = (-10000 < *mcPDG && *mcPDG < 10000) ? *mcPDG : 0.0;
        perEventData[evtid].nTracks[pdg]++;
        if (*isSignal > 0.5 && *isCloneTrack < 0.5)
          tracksVsPT[pdg].Fill(*mcPT);
        if (!(*isSignal > 0.5)) { // Badly reconstructed + completely fake tracks
          perEventData[evtid].nBadTracks[pdg]++;
          badsVsPT[pdg].Fill(*mcPT);
        }
        if (*isCloneTrack > 0.5) {
          perEventData[evtid].nCloneTracks[pdg]++;
          clonesVsPT[pdg].Fill(*mcPT);
        }
      }

      for (const auto& t : perEventData) {
        const auto& data = t.second;
        for (int pdg : PDGIDs) {
          nTracks[pdg].Fill(data.nTracks[pdg] - data.nBadTracks[pdg] - data.nCloneTracks[pdg]);
          nClones[pdg].Fill(data.nCloneTracks[pdg]);
          nBad[pdg].Fill(data.nBadTracks[pdg]);
        }
        totalTracks.Fill(data.nTracks.total() - data.nBadTracks.total() - data.nCloneTracks.total());
        totalClones.Fill(data.nCloneTracks.total());
        totalBad.Fill(data.nBadTracks.total());
      }
    }

    for (int pdg: PDGIDs) {
      const TString& pTitle = PDGTitles.at(pdg);
      c.PrintHistos(pTitle + " tracks per event", {
        {&nTracks[pdg], "Good", kBlack},
        {&nClones[pdg], "Clones", MyBlue},
        {&nBad[pdg], "Bad/fake", MyRed}});
      c.PrintHistos(pTitle + " tracks vs p_{T}", {
        {&tracksVsPT[pdg], "Good", kBlack},
        {&clonesVsPT[pdg], "Clones", MyBlue},
        {&badsVsPT[pdg], "Bad/fake", MyRed}});
    }
    c.PrintHistos("Tracks per event", {
      {&totalTracks, "Good", kBlack},
      {&totalClones, "Clones", MyBlue},
      {&totalBad, "Bad/fake", MyRed}}, 8500);
    TH1F* totalTracksVsPT = tracksVsPT.total();
    TH1F* totalClonesVsPT = clonesVsPT.total();
    TH1F* totalBadVsPT = badsVsPT.total();
    c.PrintHistos("Tracks vs p_{T}", {
      {totalTracksVsPT, "Good", kBlack},
      {totalClonesVsPT, "Clones", MyBlue},
      {totalBadVsPT, "Bad/fake", MyRed}}, 5300);
    delete totalTracksVsPT; delete totalClonesVsPT; delete totalBadVsPT;
  }

  // Best candidate selection stuff
  gStyle->SetOptStat(110000);
  gStyle->SetHistLineWidth(1);
  for (const TString channel : {"Kpi", "K3pi"}) {
    const TString chn = channel + "_", cht = (channel + " - ").ReplaceAll("pi", "#pi");
    TH1F hSigCand(chn + "nSigCand", cht + "Signal candidates;Number of signal candidates;Events / bin", 10, -0.5, 9.5),
         hMisCand(chn + "nMisCand", cht + "Misreconstructed candidates;Number of bad candidates;Events / bin", 50, -0.5, 49.5),
         hB0_M(chn + "B0_M", cht + "M_{B^{0}};M - M_{sig} [GeV/c^{2}];Candidates / bin", 100, -3.5, 1.5),
         hB0_chiprob(chn + "B0_chiprob", cht + "#chi^{2}_{B^{0}};#chi^{2} - #chi^{2}_{sig};Candidates / bin", 100, -1, 1),
         hDst_M(chn + "Dst_M", cht + "M_{D*};M - M_{sig} [GeV/c^{2}];Candidates / bin", 100, -0.05, 0.05),
         hDst_massDiff(chn + "Dst_massDiff", cht + "#DeltaM = M_{D*} - M_{D^{0}};#DeltaM - #DeltaM_{sig} [GeV/c^{2}];Candidates / bin", 100, -0.02, 0.02),
         hDst_pCM(chn + "Dst_pCM", cht + "p_{CM,D*};p_{CM} - p_{CM,sig} [GeV/c];Candidates / bin", 100, -0.2, 0.2),
         hD0_M(chn + "D0_M", cht + "M_{D^{0}};M - M_{sig} [GeV/c^{2}];Candidates / bin", 100, -0.05, 0.05),
         hB0_dM(chn + "B0_dM", cht + "|#deltaM_{B^{0}}| = |M_{B^{0}} - M_{B^{0},PDG}|;|#deltaM| - |#deltaM_{sig}| [GeV/c^{2}];Candidates / bin", 100, -2, 3.5),
         hDst_dM(chn + "Dst_dM", cht + "|#deltaM_{D*}| = |M_{D*} - M_{D*,PDG}|;|#deltaM| - |#deltaM_{sig}| [GeV/c^{2}];Candidates / bin", 100, -0.05, 0.1),
         hDst_dMassDiff(chn + "Dst_dMassDiff", cht + "|#delta#DeltaM| = |#DeltaM - #DeltaM_{PDG}|;|#delta#DeltaM| - |#delta#DeltaM_{sig}| [GeV/c^{2}];Candidates / bin", 100, -0.1, 0.05),
         hD0_dM(chn + "D0_dM", cht + "|#deltaM_{D^{0}}| = |M_{D^{0}} - M_{D^{0},PDG}|;|#deltaM| - |#deltaM_{sig}| [GeV/c^{2}];Candidates / bin", 100, -0.05, 0.1);
    { // Loop over ntuple and gather data in histograms
      map<EvtID,EvtCandidatesData> perEventData;

      TTree* tree = file->Get<TTree>(channel);
      CHECK(tree);
      TTreeReader rdr(tree);
      TTreeReaderValue<Double_t> isSignal(rdr, "B0_isSignalAcceptMissingNeutrino"),
                                 B0_M(rdr, "B0_M"),
                                 B0_chiprob(rdr, "B0_chiProb"),
                                 Dst_M(rdr, "Dst_M"),
                                 Dst_pCM(rdr, "Dst_p_CMS"),
                                 D0_M(rdr, "D0_M");
      TTreeReaderValue<Int_t>   exp(rdr, "__experiment__"),
                                run(rdr, "__run__"),
                                evt(rdr, "__event__");
      while (rdr.Next()) {
        EvtID evtid = make_tuple(*exp, *run, *evt);
        CandidateData data {*B0_M, *B0_chiprob, *Dst_M, *Dst_pCM, *D0_M};
        if (*isSignal > 0.5) {
          if ((perEventData[evtid].nSignalCandidates++) == 0)
            perEventData[evtid].signalData = data;
        } else
          perEventData[evtid].misrecosData.push_back(data);
      }

      for (const auto& t : perEventData) {
        const auto& data = t.second;
        hSigCand.Fill(data.nSignalCandidates);
        hMisCand.Fill(data.misrecosData.size());
        if (!data.nSignalCandidates) continue;
        const auto& sig = data.signalData;
        for (const auto& cand : data.misrecosData) {
          hB0_M.Fill(cand.B0_M - sig.B0_M);
          hB0_chiprob.Fill(cand.B0_chiprob - sig.B0_chiprob);
          hDst_M.Fill(cand.Dst_M - sig.Dst_M);
          hDst_massDiff.Fill(cand.Dst_massDiff() - sig.Dst_massDiff());
          hDst_pCM.Fill(cand.Dst_pCM - sig.Dst_pCM);
          hD0_M.Fill(cand.D0_M - sig.D0_M);
          hB0_dM.Fill(cand.B0_dM() - sig.B0_dM());
          hDst_dM.Fill(cand.Dst_dM() - sig.Dst_dM());
          hDst_dMassDiff.Fill(cand.Dst_dMassDiff() - sig.Dst_dMassDiff());
          hD0_dM.Fill(cand.D0_dM() - sig.D0_dM());
        }
      }
    }

    for (TH1 *h : {&hSigCand, &hMisCand})
      c.PrintHisto(h);
    for (TH1 *h : {&hB0_M, &hB0_chiprob, &hDst_M, &hDst_massDiff, &hDst_pCM, &hD0_M, &hB0_dM, &hDst_dM, &hDst_dMassDiff, &hD0_dM})
      c.PrintHisto(h, true);
  }
}
