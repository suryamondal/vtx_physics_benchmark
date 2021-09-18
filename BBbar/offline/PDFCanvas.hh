#pragma once
#include <TString.h>
// Forward declarations
class TCanvas;

/** A TCanvas wrapper that prints multiple pages to the same PDF. */
class PDFCanvas {
 public:
  PDFCanvas() = delete;
  PDFCanvas(const PDFCanvas&) = delete;
  PDFCanvas(PDFCanvas&&) = delete;
  PDFCanvas& operator=(const PDFCanvas&) = delete;
  PDFCanvas& operator=(PDFCanvas&&) = delete;

  /** Opens pdfFileName and creates a canvas with the other arguments. */
  PDFCanvas(TString pdfFileName, const char* name = "c", const char* title = "c",
            int width = 640, int height = 480);

  /** Closes the PDF file and deletes the canvas. */
  ~PDFCanvas();

  TCanvas& operator*() { return *m_c; }
  TCanvas* operator->() { return m_c; }

  /** Prints a page to the PDF (no need to remeber the file's name). */
  void PrintPage();

  /** Prints a page to the PDF (no need to remeber the file's name). */
  void PrintPage(TString title);

  /** Gets the name/path of the current PDF file. */
  TString GetPDFFileName() { return m_pdfName; }

  /** Sets the name/path of the PDF file. The previous one is closed. */
  void SetPDFFileName(TString newFileName);

 private:
  TString m_pdfName;
  TCanvas* m_c;
};
