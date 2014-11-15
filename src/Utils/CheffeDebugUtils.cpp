#include "CheffeDebugUtils.h"

#ifndef NDEBUG

bool cheffe::DebugFlag;

static std::string DebugType;

void cheffe::setCurrentDebugType(const char *Type)
{
  DebugType = Type;
  DebugFlag |= !DebugType.empty();
}

bool cheffe::isCurrentDebugType(const char *Type)
{
  return DebugType.empty() || !DebugType.compare(Type);
}

std::ostream &cheffe::dbgs()
{
  return std::cout;
}

#endif
