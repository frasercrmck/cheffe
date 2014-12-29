#ifndef CHEFFE_JIT
#define CHEFFE_JIT

#include "cheffe.h"
#include "CheffeCommon.h"
#include "IR/CheffeProgramInfo.h"
#include "Utils/CheffeDiagnosticHandler.h"

namespace cheffe
{

class CheffeJIT
{
public:
  CheffeJIT(std::unique_ptr<CheffeProgramInfo> ProgramInfo,
            std::shared_ptr<CheffeDiagnosticHandler> Diags)
      : ProgramInfo(std::move(ProgramInfo)), Diagnostics(Diags)
  {
  }

  CheffeErrorCode executeRecipe();

private:
  std::unique_ptr<CheffeProgramInfo> ProgramInfo;
  std::shared_ptr<CheffeDiagnosticHandler> Diagnostics;
};

} // end namespace cheffe

#endif // CHEFFE_JIT
