#include "Utils/CheffeErrorHandling.h"
#include "Utils/CheffeDebugUtils.h"

void cheffe::cheffe_unreachable(const char *Message)
{
  if (Message)
  {
    dbgs() << Message << std::endl;
  }

  dbgs() << "UNREACHABLE executed!" << std::endl;

  abort();
}
