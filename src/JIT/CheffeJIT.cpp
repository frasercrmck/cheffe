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
  if (!ProgramInfo)
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  std::shared_ptr<CheffeRecipeInfo> MainRecipeInfo =
      ProgramInfo->getEntryPointRecipe();

  if (!MainRecipeInfo)
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

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
