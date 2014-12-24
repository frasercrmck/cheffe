#include "cheffe.h"
#include "JIT/CheffeJIT.h"
#include "Utils/CheffeDebugUtils.h"
#include "Parser/CheffeRecipeInfo.h"

#include <iostream>

#define DEBUG_TYPE "jit"

namespace cheffe
{

CheffeErrorCode CheffeJIT::executeRecipe()
{
  if (!RecipeMap)
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }
  if (RecipeMap->find(MainRecipeTitle) == std::end(*RecipeMap))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }
  if (!RecipeMap->find(MainRecipeTitle)->second)
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  const std::unique_ptr<CheffeRecipeInfo> &MainRecipeInfo =
      RecipeMap->find(MainRecipeTitle)->second;

  // clang-format off
  CHEFFE_DEBUG(
    dbgs() << "Executing '" << MainRecipeInfo->getRecipeTitle() << "'..."
           << std::endl << std::endl;
  );
  // clang-format on

  auto MethodSteps = MainRecipeInfo->getMethodStepList();

  // clang-format off
  CHEFFE_DEBUG(
    for (auto &I : MethodSteps)
    {
      dbgs() << *I;
    }
  );
  // clang-format on

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

} // end namespace cheffe
