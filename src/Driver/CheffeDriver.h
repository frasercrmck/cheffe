#ifndef CHEFFE_DRIVER
#define CHEFFE_DRIVER

#include "cheffe.h"

#include <vector>

namespace cheffe
{
class CheffeDriver
{
public:
  CheffeDriver()
  {
  }

  void addSourceFile(const CheffeSourceFile &File);

private:
  std::vector<CheffeSourceFile> SourceFiles;
};
}; // end namespace cheffe

#endif // CHEFFE_DRIVER
