#pragma once
#include <TH1.h>
#include <TString.h>
#include <TPaveText.h>
#include <stdexcept>

#define STRNG(x) #x
#define STRNG2(x) STRNG(x)
#define CHECK(assertion) if(!(assertion)) throw std::runtime_error("At " STRNG2(__FILE__) ":" STRNG2(__LINE__))

/** Allows TString literals with "string"TS (raises a warning, ignore it) */
inline TString operator""TS(const char *s, size_t sz) { return TString(s, sz); }

/** Set line, fill and marker colors in one call. */
inline void SetColor(TH1* h, Color_t cLineMarker, Color_t cFill)
{
  h->SetLineColor(cLineMarker);
  h->SetMarkerColor(cLineMarker);
  h->SetFillColor(cFill);
}

/** Default TPave style setter. */
inline void SetPaveStyle(TPaveText& p, Color_t c = kBlack)
{
  p.SetBorderSize(1);
  p.SetFillColor(kWhite);
  p.SetFillStyle(1001);
  p.SetLineColor(c);
  p.SetTextColor(c);
}

/** Set line, fill and marker colors in one call. */
inline void SetColor(TH1* h, Color_t c) { SetColor(h, c, c); }

/** Normalize an histogram. */
inline void Normalize(TH1* h) { h->Scale(1.0 / h->GetEntries()); }

/** Restores a previously normalized histogram. */
inline void Unnormalize(TH1* h) { h->Scale(h->GetEntries()); }

/** This function shall be called with the name you want to give to sth.
 *  - The first time a name is used, the function returns the name.
 *  - The second time, it returns the name + "_2"
 *  - The third time, it returns the name + "_3"
 *  - ...
 */
TString GetUniqueName(TString baseName);
