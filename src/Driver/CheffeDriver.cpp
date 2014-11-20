#include "Driver/CheffeDriver.h"
#include "Parser/CheffeParser.h"

namespace cheffe
{
void CheffeDriver::setSourceFile(const CheffeSourceFile &SrcFile)
{
  File = SrcFile;
}

CheffeErrorCode CheffeDriver::compileRecipe()
{
  if (File.Source.empty())
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  auto Diagnostics = std::make_shared<CheffeDiagnosticHandler>(File);
  CheffeParser Parser(File, Diagnostics);

  const CheffeErrorCode Success = Parser.parseRecipe();

  return Success;
}

} // end namespace cheffe
