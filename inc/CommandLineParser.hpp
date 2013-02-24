#ifndef _CommandLineParser
#define _CommandLineParser

#include <map>
#include <string>
#include <cstdio>

#include "SortUtility.hpp"

class CommandLineParser
{
private:
  int _argc;
  char** _argv;

public:
  CommandLineParser(const int & argc, char** argv);

  void parseInto(std::map<std::string, std::string> & options);

  std::map<std::string, std::string> parse();
};

#endif
