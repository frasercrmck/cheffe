#ifndef CHEFFE_DEBUG_UTILS
#define CHEFFE_DEBUG_UTILS

#include "cheffe.h"

#include <iostream>
#include <string>

namespace cheffe
{

#ifndef NDEBUG

extern bool DebugFlag;

bool isCurrentDebugType(const char *Type);
void setCurrentDebugType(const char *Type);

#define CHEFFE_DEBUG_WITH_TYPE(TYPE, EXPR)                                     \
  do                                                                           \
  {                                                                            \
    if (::cheffe::DebugFlag && ::cheffe::isCurrentDebugType(TYPE))             \
    {                                                                          \
      std::cout << EXPR;                                                       \
    }                                                                          \
  } while (false)

#define CHEFFE_DEBUG(EXPR) CHEFFE_DEBUG_WITH_TYPE(DEBUG_TYPE, EXPR)

#else // NDEBUG

#define isCurrentDebugType(X) (false)
#define setCurrentDebugType(X)

#define CHEFFE_DEBUG(EXPR)

#define CHEFFE_DEBUG_WITH_TYPE(TYPE, EXPR)

#endif // NDEBUG

}; // end namespace cheffe

#endif // CHEFFE_DEBUG_UTILS
