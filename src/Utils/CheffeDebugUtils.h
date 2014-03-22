#ifndef CHEFFE_DEBUG_UTILS
#define CHEFFE_DEBUG_UTILS

#include <iostream>

#ifndef NDEBUG
#define CHEFFE_DEBUG(expr)                                                                         \
  do                                                                                               \
  {                                                                                                \
    std::cout << expr;                                                                             \
  } while (false)
#else
#define CHEFFE_DEBUG(expr)
#endif

#endif
