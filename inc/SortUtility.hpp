#ifndef _SortUtility
#define _SortUtility

#include <string>
#include <sstream>

#include <QString>

#include <errno.h>
#include <cstdio>

#include "SortTypes.hpp"

struct SortUtility
{
  static std::string  itos(const int & i);

  static QString SigCodeToQString(const SortTypes::SigCode & sig);

  static SortTypes::SigCode IntToSigCode(const int & intVal);
};

#endif
