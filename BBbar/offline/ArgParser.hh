/** There certainly are libraries that do this in internet, but I am
 * offline, so I am writing this on my own.
 */
#pragma once
#include <TString.h>
#include <vector>
#include <map>

class ArgParser {
 public:
  ArgParser() = delete;
  ArgParser(const ArgParser&) = delete;
  ArgParser(ArgParser&&) = delete;
  ArgParser& operator=(const ArgParser&) = delete;
  ArgParser& operator=(ArgParser&&) = delete;

  explicit ArgParser(TString desc) : m_desc(desc) {}

  /** Adds a positional argument, pretty self-explainatory. */
  void AddPositionalArg(TString name) { m_positionalArgs.push_back(name); }

  /** Adds a flag, which must start with "--" on the command line.
   * Here, you can include the "--" or not, it won't make a difference.
   */
  void AddFlag(TString name) { m_flags.push_back(name.BeginsWith("--") ? name : "--" + name); }

  /** Self-explainatory. Returns a map of all the arguments like this:
   *  - Positional arguments: {ARG_NAME: GIVEN_VALUE}
   *  - Flags: {ARG_NAME: ""} if flag is given, not in map otherwise
   *
   * If the number of positional arguments is wrong, or --help is given,
   * prints an help message to stdout and exits.
   */
  std::map<TString,TString> ParseArgs(int argc, char** argv);

 private:
  bool HasFlag(TString name);

  void PrintHelpAndExit(TString progName, bool exitWithError);

  TString m_desc;
  std::vector<TString> m_positionalArgs;
  std::vector<TString> m_flags;
};
