#include "PDFCanvas.hh" // Own include
#include "Utils.hh"
#include <TCanvas.h> // Forward-declared

PDFCanvas::PDFCanvas(TString pdfFileName, const char *name, const char *title,
                     int width, int height)
: m_pdfName(pdfFileName)
{
  m_c = new TCanvas(name, title, width, height);
  CHECK(m_c);
  m_c->Print(m_pdfName + "[");
}

PDFCanvas::~PDFCanvas()
{
  m_c->Print(m_pdfName + "]");
  delete m_c;
}

void PDFCanvas::PrintPage() { m_c->Print(m_pdfName); }

void PDFCanvas::PrintPage(TString title) { m_c->Print(m_pdfName, "Title:" + title); }
