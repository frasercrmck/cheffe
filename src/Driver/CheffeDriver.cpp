#include "Driver/CheffeDriver.h"
#include "Parser/CheffeParser.h"

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

CheffeErrorCode CheffeDriver::compileRecipe()
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

  const CheffeErrorCode Success = Parser.parseRecipe();

  return Success;
}

} // end namespace cheffe
