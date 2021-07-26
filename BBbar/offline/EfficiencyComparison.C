/** ROOT macro for plotting efficiency plots together.
 *
 */
#include <TString.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TLegend.h>
#include <TColor.h>
#include <TStyle.h>
#include <vector>
#include <stdexcept>
using namespace std;

#define STRNG(x) #x
#define STRNG2(x) STRNG(x)
#define CHECK(assertion) if(!(assertion)) throw runtime_error("At " STRNG2(__FILE__) ":" STRNG2(__LINE__))

const Color_t Palette[] = {kBlack, kRed};
const Int_t NPalette = sizeof(Palette) / sizeof(Color_t);
const auto MyBlue = TColor::GetColor("#348ABD");
const Int_t Font = 63;
const Double_t TSize = 19; // In pixels

vector<TString> Ls(TDirectory* d)
{
  vector<TString> res;
  for (const auto& obj : *(d->GetListOfKeys()))
    res.push_back(obj->GetName());
  return res;
}

void setStyle()
{
  gStyle->SetTextFont(Font);
  gStyle->SetTextSize(TSize);
  gStyle->SetLabelFont(Font, "xyz");
  gStyle->SetTitleFont(Font, "xyz");
  gStyle->SetLabelSize(TSize, "xyz");
  gStyle->SetTitleSize(TSize, "xyz");
  gStyle->SetTitleX(0.5); gStyle->SetTitleY(0.97);
  gStyle->SetTitleW(0.7); gStyle->SetTitleH(0.1);
}

void setFont(TH1* h)
{
  h->SetTitleFont(Font, "XYZ");
  h->SetTitleSize(TSize, "XYZ");
  h->SetLabelFont(Font, "XYZ");
  h->SetLabelSize(TSize, "XYZ");
  h->SetTitleOffset(3, "X");
  h->SetTitleOffset(1.1, "Y");
}

void EfficiencyComparison(TString outPDF, const vector<TString>& filesNames,
                          const vector<TString>& titles)
{
  setStyle();
  TCanvas c("c", "c", 640, 480);
  c.Print(outPDF + "[");
  // Subpads
  TPad topPad("topPad", "topPad", 0, 0.33, 1, 1, kWhite);
  TPad btmPad("btmPad", "btmPad", 0, 0, 1, 0.33, kWhite);
  topPad.SetMargin(0.1, 0.1, 0, 0.149);
  btmPad.SetMargin(0.1, 0.1, 0.303, 0);
  topPad.SetNumber(1);
  btmPad.SetNumber(2);
  topPad.Draw();
  btmPad.Draw();

  // Open files
  vector<TFile *> files;
  for (const auto& fileName : filesNames) {
    TFile* f = TFile::Open(fileName);
    CHECK(f);
    files.push_back(f);
  }

  // Loop over (known) directories
  for (const TString& dirName : {"KpiCuts", "K3piCuts"}) {
    vector<TDirectory*> dirs;
    for (const auto& f : files) {
      TDirectory* dir = f->GetDirectory(dirName);
      CHECK(dir);
      dirs.push_back(dir);
    }

    // Loop over hist names
    for (const auto& hName : Ls(dirs.front())) {
      if (!hName.Contains("_eff_")) continue;
      vector<TH1*> hists;
      for (const auto& d : dirs) {
        TH1* h = d->Get<TH1>(hName);
        CHECK(h);
        hists.push_back(h);
      }

      TString hNameMC = hName;
      TH1 *hMC = dirs.front()->Get<TH1>(hNameMC.ReplaceAll("_eff_", "_MC_"));
      CHECK(hMC);

      // Print
      topPad.cd();

      hMC->SetLineWidth(0);
      hMC->SetFillColorAlpha(MyBlue, 0.4);
      hMC->SetFillStyle(1001);
      hMC->Scale(0.9 / hMC->GetBinContent(hMC->GetMaximumBin()));
      hMC->SetMinimum(0.0001); hMC->SetMaximum(1);
      hMC->Draw("hist");
      setFont(hMC);

      TLegend leg(0.86, 0.91 - 0.06 * (hists.size() + 1), 0.98, 0.91);
      leg.AddEntry(hMC, "MC", "F");

      for (int i = 0; i < hists.size(); i++) {
        hists[i]->SetLineColor(Palette[i%NPalette]);
        // hists[i]->SetLineWidth(i == 0 ? 2 : 1);
        hists[i]->SetMarkerColor(Palette[i%NPalette]);
        leg.AddEntry(hists[i], titles.at(i), "LE");
        hists[i]->Draw("same");
      }
      leg.Draw();
      topPad.SetGrid();

      btmPad.cd();
      TH1* hRatio = (TH1*)hists.at(0)->Clone(hists.at(0)->GetName() + TString("_ratio"));
      hRatio->Divide(hists.at(1));
      hRatio->GetYaxis()->SetTitle(titles.at(0) + " / " + titles.at(1));
      hRatio->SetTitle("");
      hRatio->SetMinimum(0); hRatio->SetMaximum(9.5);
      hRatio->Draw();
      setFont(hRatio);
      hRatio->GetYaxis()->SetNdivisions(505);
      btmPad.SetGrid(1, 2);

      c.Print(outPDF, TString("Title:") + hists[0]->GetTitle());
    }
  }

  c.Print(outPDF + "]");
}
