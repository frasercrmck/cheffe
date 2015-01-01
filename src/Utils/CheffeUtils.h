#ifndef CHEFFE_UTILS
#define CHEFFE_UTILS

#include <string>

namespace cheffe
{

static bool AreLowerCasedStringsEqual(const std::string &LHS,
                                      const std::string &RHS)
{
  if (LHS.size() != RHS.size())
  {
    return false;
  }
  auto LHSIt = std::begin(LHS);
  auto RHSIt = std::begin(RHS);
  while (LHSIt++ != std::end(LHS) && RHSIt++ != std::end(RHS))
  {
    if (std::tolower(*LHSIt) != std::tolower(*RHSIt))
    {
      return false;
    }
  }
  return true;
}

}; // end namespace cheffe

#endif // CHEFFE_UTILS
