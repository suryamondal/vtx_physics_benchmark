#pragma once
#include <TH1.h>
#include <TString.h>
#include <stdexcept>

#define STRNG(x) #x
#define STRNG2(x) STRNG(x)
#define CHECK(assertion) if(!(assertion)) throw std::runtime_error("At " STRNG2(__FILE__) ":" STRNG2(__LINE__))

/** Allows TString literals with "string"TS (raises a warning, ignore it) */
inline TString operator""TS(const char *s, size_t sz) { return TString(s, sz); }

/** Set line, fill and marker colors in one call. */
inline void SetColor(TH1* h, Color_t cLineMarker, Color_t cFill) {
  h->SetLineColor(cLineMarker);
  h->SetMarkerColor(cLineMarker);
  h->SetFillColor(cFill);
}

/** Set line, fill and marker colors in one call. */
inline void SetColor(TH1* h, Color_t c) { SetColor(h, c, c); }

/** Normalize an histogram. */
inline void Normalize(TH1* h) { h->Scale(1.0 / h->GetEntries()); }
