#pragma once
#include <TString.h>
#include <stdexcept>

#define STRNG(x) #x
#define STRNG2(x) STRNG(x)
#define CHECK(assertion) if(!(assertion)) throw std::runtime_error("At " STRNG2(__FILE__) ":" STRNG2(__LINE__))

inline TString operator""TS(const char *s, size_t sz) { return TString(s, sz); }
