#include "cheffe.h"
#include "JIT/CheffeJIT.h"
#include "IR/CheffeRecipeInfo.h"
#include "IR/CheffeIngredient.h"
#include "Utils/CheffeDebugUtils.h"

#include <deque>
#include <memory>
#include <iostream>

#define DEBUG_TYPE "jit"

namespace cheffe
{

typedef std::pair<bool, uint64_t> StackItemTy;
typedef std::deque<StackItemTy> StackTy;

std::vector<StackTy> MixingBowls;
std::vector<StackTy> BakingDishes;

CheffeErrorCode
CheffeJIT::getIngredientInfo(const std::shared_ptr<MethodOp> &MOp,
                             std::shared_ptr<CheffeIngredient> &IngredientInfo,
                             SourceLocation &IngredientLoc)
{
  auto Ingredient = std::static_pointer_cast<IngredientOp>(MOp);
  if (!Ingredient->getIngredient())
  {
    Diagnostics->report(Ingredient->getSourceLoc(), DiagnosticKind::Error,
                        LineContext::WithContext)
        << "Trying to use an undefined ingredient";
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  IngredientLoc = Ingredient->getSourceLoc();
  IngredientInfo = Ingredient->getIngredient();

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

void pushStackItem(std::vector<StackTy> &Stack, const StackItemTy StackItem,
                   const unsigned StackIdx)
{
  if ((StackIdx + 1) > Stack.size())
  {
    Stack.resize(StackIdx + 1);
  }

  Stack[StackIdx].push_back(StackItem);
}

void pushMixingBowlItem(const StackItemTy StackItem, const unsigned StackIdx)
{
  return pushStackItem(MixingBowls, StackItem, StackIdx);
}

void pushBakingDishItem(const StackItemTy StackItem, const unsigned StackIdx)
{
  return pushStackItem(BakingDishes, StackItem, StackIdx);
}

StackItemTy popStackItem(std::vector<StackTy> &Stack,
                         const unsigned StackItemIdx)
{
  if (StackItemIdx > Stack.size())
  {
    return std::make_pair(true, 0);
  }

  if (Stack[StackItemIdx].empty())
  {
    return std::make_pair(true, 0);
  }
  auto StackItem = Stack[StackItemIdx].back();
  Stack[StackItemIdx].pop_back();
  return StackItem;
}

StackItemTy popMixingBowlItem(const unsigned StackItemIdx)
{
  return popStackItem(MixingBowls, StackItemIdx);
}

StackItemTy popBakingDishItem(const unsigned StackItemIdx)
{
  return popStackItem(BakingDishes, StackItemIdx);
}

bool CheffeJIT::checkIngredientHasValue(
    const std::shared_ptr<CheffeIngredient> &Ingredient,
    const SourceLocation IngredientLoc)
{
  if (Ingredient->HasValue)
  {
    return true;
  }

  Diagnostics->report(IngredientLoc, DiagnosticKind::Error,
                      LineContext::WithContext)
      << "Using ingredient '" << Ingredient->Name << "' without a value";
  return false;
}

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
