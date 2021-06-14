#include "ArgParser.hh" // Own include
#include "Utils.hh"
#include <iostream>
#include <cstdlib> // For exit()
using namespace std;

// TODO check for repetitions in ParseArgs when adding to the map

bool ArgParser::HasFlag(TString name)
{
  for (const TString& flag : m_flags)
    if (flag == name)
      return true;
  return false;
}

void ArgParser::PrintHelpAndExit(TString progName, bool exitWithError)
{
  cout << m_desc << endl;
  cout << "Usage: " << progName;
  for (const TString& arg : m_positionalArgs)
    cout << " " << arg;
  for (const TString& arg: m_flags)
    cout << " " << arg;
  cout << endl;
  exit(exitWithError ? EXIT_FAILURE : EXIT_SUCCESS);
}

map<TString,TString> ArgParser::ParseArgs(int argc, char** argv)
{
  map<TString,TString> res;
  TString progName = argc == 0 ? "./ana" : argv[0];
  auto currentPosArg = m_positionalArgs.cbegin();

  for (int i = 1; i < argc; i++) {
    TString arg = argv[i];
    if (arg.BeginsWith("--")) { // Flag
      if (arg == "--help") {
        PrintHelpAndExit(progName, false);
      } else if (HasFlag(arg)) {
        if (res.find(arg) != res.end()) {
          cout << "Repeated flag: " << arg << endl;
          PrintHelpAndExit(progName, true);
        } else {
          res[arg.Strip(TString::kLeading, '-')] = "";
        }
      } else {
        cout << "Invalid flag: " << arg << endl;
        PrintHelpAndExit(progName, true);
      }
    } else { // Positional argument
      if (currentPosArg == m_positionalArgs.cend()) {
        cout << "Too many positional arguments." << endl;
        PrintHelpAndExit(progName, true);
      } else {
        res[*currentPosArg] = arg;
        currentPosArg++;
      }
    }
  }

  if (currentPosArg != m_positionalArgs.cend()) {
    cout << "Not enough positional arguments." << endl;
    PrintHelpAndExit(progName, true);
  }

  return res;
}
