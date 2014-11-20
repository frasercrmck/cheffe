#ifndef CHEFFE_DRIVER
#define CHEFFE_DRIVER

#include "cheffe.h"
#include "Utils/CheffeDiagnosticHandler.h"

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

  void setDiagnosticHandler(std::shared_ptr<CheffeDiagnosticHandler> Diags);

private:
  CheffeSourceFile File;
  std::shared_ptr<CheffeDiagnosticHandler> Diagnostics;
};
}; // end namespace cheffe

#endif // CHEFFE_DRIVER
