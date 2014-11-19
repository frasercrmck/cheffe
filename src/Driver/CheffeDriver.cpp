#include "Driver/CheffeDriver.h"
#include "Parser/CheffeParser.h"

namespace cheffe
{
void CheffeDriver::addSourceFile(const CheffeSourceFile &File)
{
  SourceFiles.push_back(File);
}

CheffeErrorCode CheffeDriver::compileRecipe()
{
  if (SourceFiles.empty())
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  const CheffeSourceFile File = SourceFiles[0];

  auto Diagnostics = std::make_shared<CheffeDiagnosticHandler>(File);
  CheffeParser Parser(File, Diagnostics);

  const CheffeErrorCode Success = Parser.parseRecipe();

  return Success;
}

} // end namespace cheffe
