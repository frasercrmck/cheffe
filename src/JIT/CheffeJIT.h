#ifndef CHEFFE_JIT
#define CHEFFE_JIT

#include "cheffe.h"
#include "CheffeCommon.h"
#include "Utils/CheffeDiagnosticHandler.h"

namespace cheffe
{

class CheffeJIT
{
public:
  CheffeJIT(std::unique_ptr<RecipeMap> RecipeMap,
            const std::string &MainRecipeTitle,
            std::shared_ptr<CheffeDiagnosticHandler> Diags)
      : RecipeMap(std::move(RecipeMap)), MainRecipeTitle(MainRecipeTitle),
        Diagnostics(Diags)
  {
  }

  CheffeErrorCode executeRecipe();

private:
  std::unique_ptr<RecipeMap> RecipeMap;
  std::string MainRecipeTitle;
  std::shared_ptr<CheffeDiagnosticHandler> Diagnostics;
};

} // end namespace cheffe

#endif // CHEFFE_JIT
