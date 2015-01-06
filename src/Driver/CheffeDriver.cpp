#include "Driver/CheffeDriver.h"
#include "Parser/CheffeParser.h"
#include "JIT/CheffeJIT.h"

namespace cheffe
{
void CheffeDriver::setSourceFile(const CheffeSourceFile &SrcFile)
{
  File = SrcFile;
}

void CheffeDriver::setDiagnosticHandler(
    std::shared_ptr<CheffeDiagnosticHandler> Diags)
{
  Diagnostics = Diags;
}

CheffeErrorCode
CheffeDriver::compileProgram(std::unique_ptr<CheffeProgramInfo> &ProgramInfo)
{
  if (File.Source.empty())
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (!Diagnostics)
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  Diagnostics->setSourceFile(File);
  CheffeParser Parser(File, Diagnostics);

  CheffeErrorCode Success = Parser.parseProgram();

  if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    return Success;
  }

  ProgramInfo = Parser.takeProgramInfo();

  if (!ProgramInfo)
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  return Success;
}

CheffeErrorCode
CheffeDriver::executeProgram(std::unique_ptr<CheffeProgramInfo> &ProgramInfo)
{
  CheffeJIT JIT(std::move(ProgramInfo), Diagnostics);

  return JIT.executeProgram();
}

} // end namespace cheffe
