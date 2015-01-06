#ifndef CHEFFE_DRIVER
#define CHEFFE_DRIVER

#include "cheffe.h"
#include "IR/CheffeProgramInfo.h"
#include "Utils/CheffeDiagnosticHandler.h"

#include <vector>
#include <memory>

namespace cheffe
{
class CheffeDriver
{
public:
  CheffeDriver()
  {
  }

  CheffeErrorCode compileProgram(std::unique_ptr<CheffeProgramInfo> &ProgramInfo);
  CheffeErrorCode executeProgram(std::unique_ptr<CheffeProgramInfo> &ProgramInfo);

  void setSourceFile(const CheffeSourceFile &File);

  void setDiagnosticHandler(std::shared_ptr<CheffeDiagnosticHandler> Diags);

private:
  CheffeSourceFile File;
  std::shared_ptr<CheffeDiagnosticHandler> Diagnostics;
};
}; // end namespace cheffe

#endif // CHEFFE_DRIVER
