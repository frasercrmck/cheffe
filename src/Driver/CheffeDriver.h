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

  CheffeErrorCode compileRecipe();

  void setSourceFile(const CheffeSourceFile &File);

private:
  CheffeSourceFile File;
};
}; // end namespace cheffe

#endif // CHEFFE_DRIVER
