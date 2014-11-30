#include "CheffeDebugUtils.h"

#ifndef NDEBUG

bool cheffe::DebugFlag;

static std::string DebugType;

void cheffe::setDebugFlag(const bool Flag)
{
  DebugFlag = Flag;
}

void cheffe::setCurrentDebugType(const char *Type)
{
  DebugType = Type;
  DebugFlag |= !DebugType.empty();
}

bool cheffe::isCurrentDebugType(const char *Type)
{
  return DebugType.empty() || DebugType == Type;
}

std::ostream &cheffe::dbgs()
{
  return std::cout;
}

#endif
