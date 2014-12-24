#include "CheffeCommon.h"
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

  CheffeErrorCode Success = Parser.parseRecipe();

  if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    return Success;
  }

  std::unique_ptr<RecipeMap> RecipeInfo = Parser.takeRecipeInfo();
  const std::string MainRecipeTitle = Parser.getMainRecipeTitle();

  if (!RecipeInfo)
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  CheffeJIT JIT(std::move(RecipeInfo), MainRecipeTitle);

  Success = JIT.executeRecipe();

  if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    return Success;
  }

  return Success;
}

} // end namespace cheffe
