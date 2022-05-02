#pragma once
#include "PDFCanvas.hh"
#include <TString.h>
#include <ROOT/RDataFrame.hxx>
#include <tuple>
#include <vector>

/** Utility class to make plots where signal and background events are
 * overlayed.
 */
class SigBkgPlotter {
 public:
  typedef ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter,void> FilterDF;
  typedef ROOT::RDF::RInterface<ROOT::Detail::RDF::RLoopManager,void> DefineDF;
  typedef ROOT::RDF::RResultPtr<TH1D> RRes1D;
  typedef std::tuple<RRes1D,RRes1D> TRRes1D;
  typedef ROOT::RDF::RResultPtr<TH2D> RRes2D;
  typedef std::tuple<RRes2D,RRes2D> TRRes2D;
  typedef std::tuple<int, int, int, double> filterElement;

  SigBkgPlotter() = delete;
  SigBkgPlotter(const SigBkgPlotter&) = delete;
  SigBkgPlotter(SigBkgPlotter&&) = delete;
  SigBkgPlotter& operator=(const SigBkgPlotter&) = delete;
  SigBkgPlotter& operator=(SigBkgPlotter&&) = delete;

  /** Constructor for a SigBkgPlotter that takes data from df, uses sigCond to
   * tell signal from background and prints plots to c.
   */
  SigBkgPlotter(DefineDF& df, DefineDF& mcdf, TString sigCond, PDFCanvas& c,
                TString namePrefix = "undefined", TString titlePrefix = "Undefined",
                bool normalizeHistos = false, bool logScale = false)
  : m_all(df.Filter("true", "AllCandidates")),
    m_sig(df.Filter((const char*)sigCond, "Signal")),
    m_bkg(df.Filter((const char*)("!(" + sigCond + ")"), "Background")),
    m_mc(mcdf), m_c(c), m_namePrefix(namePrefix), m_titlePrefix(titlePrefix),
    m_normalizeHistos(normalizeHistos), m_logScale(logScale) {}

  /** Constructor for a SigBkgPlotter that takes data from df, uses sigCond to
   * tell signal from background and prints plots to c.
   */
  SigBkgPlotter(FilterDF& df, DefineDF& mcdf, TString sigCond, PDFCanvas& c,
                TString namePrefix = "undefined", TString titlePrefix = "Undefined",
                bool normalizeHistos = false, bool logScale = false)
  : m_all(df.Filter("true", "AllCandidates")),
    m_sig(df.Filter((const char*)sigCond, "Signal")),
    m_bkg(df.Filter((const char*)("!(" + sigCond + ")"), "Background")),
    m_mc(mcdf), m_c(c), m_namePrefix(namePrefix), m_titlePrefix(titlePrefix),
    m_normalizeHistos(normalizeHistos), m_logScale(logScale) {}

  /** Makes a tuple {sig,bkg} of histograms of the given variable.
   * The tuple is returned and saved to the interal list of plots.
   * @param scale Multiplies variable by this number before filling
   * @param sigOnly Only make signal histogram (PrintAll will skip it)
   */
  TRRes1D Histo1D(const char* variable, TString title,
                  int nBins, double xLow, double xUp, double scale = 1.0,
		  bool sigOnly = false);

  /** Makes a tuple {sig,bkg} of histograms of the given variable for
   * each of the given particles. In title, $p is replaced with the
   * "title" of the particle (see ParticlesTitles in Constants.hh).
   */
  void Histo1D(std::initializer_list<TString> particles, const char *variable,
               TString title, int nBins, double xLow, double xUp, double scale = 1.0,
	       bool sigOnly = false);

  /** Makes a tuple {sig,bkg} of 2D histograms of the given variables.
   * The tuple is returned and saved to the interal list of plots.
   */
  TRRes2D Histo2D(const char* vx, const char* vy, TString title,
                  int xBins, double xLow, double xUp,
                  int yBins, double yLow, double yUp);

  /** Makes a tuple {sig,bkg} of 2D histograms of the given variables
   * for each of the given particles. In title, $p is replaced with the
   * "title" of the particle (see ParticlesTitles in Constants.hh).
   */
  void Histo2D(std::initializer_list<TString> particles,
               const char* vx, const char* vy, TString title,
               int xBins, double xLow, double xUp,
               int yBins, double yLow, double yUp);

  /** Makes a tuple {sig,mc} of histograms of the given variable.
   * The tuple is returned and saved in the internal list of plots.
   * PrintAll() will plot the ratio sig/mc (the efficiency).
   * @param scale Multiplies variable by this number before filling
   */
  TRRes1D EffH1D(const char* variable, TString title,
                 int nBins, double xLow, double xUp, double scale = 1.0);

  /** Same as EffHisto1D(const char*, ...), but repeated for each of the
   * given particles. In title, $p is replaced with the
   * "title" of the particle (see ParticlesTitles in Constants.hh).
   */
  void EffH1D(std::initializer_list<TString> particles, const char* variable,
              TString title, int nBins, double xLow, double xUp, double scale = 1.0);

  /** Makes a tuple {sig,mc} of histograms of the given variable.
   * The tuple is returned and saved in the internal list of plots.
   * PrintAll() will plot the ratio trk/total (the purity).
   * @param scale Multiplies variable by this number before filling
   */
  TRRes1D PurityH1D(const char* variable, TString title,
                 int nBins, double xLow, double xUp, double scale = 1.0);

  /** Same as EffHisto1D(const char*, ...), but repeated for each of the
   * given particles. In title, $p is replaced with the
   * "title" of the particle (see ParticlesTitles in Constants.hh).
   */
  void PurityH1D(std::initializer_list<TString> particles, const char* variable,
              TString title, int nBins, double xLow, double xUp, double scale = 1.0);

  /** Finds the *signal* histogram called name and fits it with
   * func, then prints it to PDF.
   *
   * @param p0 is a list of {param_name, initial_value} pairs
   */
  void FitAndPrint(TString name, const char* func,
                   std::initializer_list<std::pair<TString,double>> p0 = {});

  /** Finds the *signal* histogram called name and finds the sigmaN of
   * the distribution (the half-width that contains N% of the samples,
   * with half of the remainder on each side).
   * @param N The percentage of samples in the half-width (default 68%)
   * @param rebin Rebins the histogram by this amount before drawing it
   * @param showLowHigh Show the % of samples to the left & right
   */
  void SigmaAndPrint(TString name, double N = 68.0, int rebin = 1, bool showLowHigh = false);

  
  /** Finds the *signal* histogram called name and finds the sigmaN of
   * the distribution (the half-width that contains N% of the samples,
   * with half of the remainder on each side).
   * @param N The percentage of samples in the half-width (default 68%)
   */
  void SigmaAndWrite(TString name, TString yunit, double N = 68.0, double scale = 1., bool isDiv = false);

  /** Finds the signal and backgound histograms called name and produces
   * the ROC curve plot for a cut var < threshold (if keepLow is true)
   * or var > threshodl (if keepLow is false).
   *
   * @param excludeUOF Excludes overflow and underflow (default false)
   */
  void PrintROC(TString name, bool keepLow, bool excludeOUF = false);

  /** Like the other PrintROC, but repeats for each particle. */
  void PrintROC(std::initializer_list<TString> particles,
                TString name, bool keepLow, bool excludeOUF = false);

  /** Prints all the plots made up to now to the PDF (via the PDFCanvas).
   * @param saveEff Saves efficiency histograms to current directory.
   */
  void PrintAll(bool saveEff = false);

  TString GetNamePrefix() const { return m_namePrefix; }
  void SetNamePrefix(TString namePrefix) { m_namePrefix = namePrefix; }

  TString GetTitlePrefix() const { return m_titlePrefix; }
  void SetTitlePrefix(TString titlePrefix) { m_titlePrefix = titlePrefix; }

  bool GetNormalizeHistos() const { return m_normalizeHistos; }
  void SetNormalizeHistos(bool value) { m_normalizeHistos = value; }

  bool GetLogScale() const { return m_logScale; }
  void SetLogScale(bool value) { m_logScale = value; }

  bool GetBkgDownScaleFactor() const { return m_bkgDownScale; }
  /** 0 For no scaling, 1 (default) for auto, any number for fixed. */
  void SetBkgDownScaleFactor(int value) { m_bkgDownScale = value; }

  bool HasVTX() { return m_sig.HasColumn("nVTXHits"); }

 private:
  /** Prints a signal and a background histograms to PDF. */
  void DrawSigBkg(TH1* sig, TH1* bkg);
  inline void DrawSigBkg(RRes1D sig, RRes1D bkg) { DrawSigBkg(sig.GetPtr(), bkg.GetPtr()); }
  inline void DrawSigBkg(TRRes1D tuple) { DrawSigBkg(std::get<0>(tuple), std::get<1>(tuple)); }
  inline void DrawSigBkg(RRes2D sig, RRes2D bkg) { DrawSigBkg(sig.GetPtr(), bkg.GetPtr()); }
  inline void DrawSigBkg(TRRes2D tuple) { DrawSigBkg(std::get<0>(tuple), std::get<1>(tuple)); }

  /** Prints an efficiency histogram to PDF. */
  void DrawEff(TH1* sig, TH1* mc, bool save = false);
  inline void DrawEff(RRes1D sig, RRes1D mc, bool save = false) { DrawEff(sig.GetPtr(), mc.GetPtr(), save); }
  inline void DrawEff(TRRes1D tuple, bool save = false) { DrawEff(std::get<0>(tuple), std::get<1>(tuple), save); }

  FilterDF m_all; /**< All dataframe. */
  FilterDF m_sig; /**< Signal dataframe. */
  FilterDF m_bkg; /**< Background dataframe. */
  DefineDF m_mc; /**< MC dataframe. */
  PDFCanvas& m_c; /**< The output canvas. */
  std::vector<TRRes1D> m_h1s; /**< 1D histograms go here. */
  std::vector<TRRes2D> m_h2s; /**< 1D histograms go here. */
  std::vector<TRRes1D> m_effh1s; /**< 1D efficiency histograms go here. */
  std::vector<TRRes1D> m_purityh1s; /**< 1D purity histograms go here. */
  TString m_namePrefix; /**< Prefix for the name of the histograms. */
  TString m_titlePrefix; /**< Prefix for the title of the histograms. */
  bool m_normalizeHistos; /**< Used by DrawSigBkg to decide wether to normalize histograms. */
  bool m_logScale; /**< Histograms y (or z) axis with log scale. */
  int m_bkgDownScale = 1; /**< Down-scaling factor for bkg (for visibility of sig). */
  bool m_histsAlreadyNormalized = false;

  std::map<std::string, std::vector<filterElement>> filterVector;
  // const int maxCount = 10;
};
