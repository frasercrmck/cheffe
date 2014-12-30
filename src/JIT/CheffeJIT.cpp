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

typedef std::pair<bool, long long> StackItemTy;
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
  if (StackItemIdx >= Stack.size())
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
  // Clear up from any old execution
  MixingBowls.clear();
  BakingDishes.clear();

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
    dbgs() << std::endl << "Executing '" << MainRecipeInfo->getRecipeTitle()
           << "'..." << std::endl << std::endl;
  );
  // clang-format on

  auto MethodSteps = MainRecipeInfo->getMethodStepList();

  // clang-format off
  CHEFFE_DEBUG(
    dbgs() << "=== All Method Steps ===" << std::endl;
    for (auto &I : MethodSteps)
    {
      dbgs() << *I;
    }
    dbgs() << "========================" << std::endl << std::endl;
  );
  // clang-format on

  for (auto MSI = std::begin(MethodSteps), MSE = std::end(MethodSteps);
       MSI != MSE; ++MSI)
  {
    auto MS = *MSI;
    CHEFFE_DEBUG(dbgs() << *MS);

    switch (MS->getMethodStepKind())
    {
    default:
      return CheffeErrorCode::CHEFFE_ERROR;
    case MethodStepKind::Take:
    {
      SourceLocation IngredientLoc;
      std::shared_ptr<CheffeIngredient> Ingredient = nullptr;

      const CheffeErrorCode Success =
          getIngredientInfo(MS->getOperand(0), Ingredient, IngredientLoc);
      if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
      {
        return CheffeErrorCode::CHEFFE_ERROR;
      }

      long long NewValue = 0;
      while (!(std::cin >> NewValue))
      {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input.  Try again: ";
      }

      Ingredient->HasValue = true;
      Ingredient->Value = NewValue;
      break;
    }
    case MethodStepKind::Put:
    {
      SourceLocation IngredientLoc;
      std::shared_ptr<CheffeIngredient> Ingredient = nullptr;

      const CheffeErrorCode Success =
          getIngredientInfo(MS->getOperand(0), Ingredient, IngredientLoc);
      if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
      {
        return CheffeErrorCode::CHEFFE_ERROR;
      }

      if (!checkIngredientHasValue(Ingredient, IngredientLoc))
      {
        return CheffeErrorCode::CHEFFE_ERROR;
      }

      const bool IsDry = Ingredient->IsDry;
      const long long Value = Ingredient->Value;

      auto MixingBowl =
          std::static_pointer_cast<MixingBowlOp>(MS->getOperand(1));
      const unsigned MixingBowlNo = MixingBowl->getMixingBowlNo();

      pushMixingBowlItem(std::make_pair(IsDry, Value), MixingBowlNo - 1);
      break;
    }
    case MethodStepKind::Fold:
    {
      SourceLocation IngredientLoc;
      std::shared_ptr<CheffeIngredient> Ingredient = nullptr;

      const CheffeErrorCode Success =
          getIngredientInfo(MS->getOperand(0), Ingredient, IngredientLoc);
      if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
      {
        return CheffeErrorCode::CHEFFE_ERROR;
      }

      auto MixingBowl =
          std::static_pointer_cast<MixingBowlOp>(MS->getOperand(1));
      const unsigned MixingBowlNo = MixingBowl->getMixingBowlNo();

      auto TopOfStack = popMixingBowlItem(MixingBowlNo - 1);

      Ingredient->HasValue = true;
      Ingredient->Value = TopOfStack.second;
      break;
    }
    case MethodStepKind::AddDry:
    {
      auto MixingBowl =
          std::static_pointer_cast<MixingBowlOp>(MS->getOperand(0));
      const unsigned MixingBowlNo = MixingBowl->getMixingBowlNo();
      if (MixingBowlNo > MixingBowls.size())
      {
        MixingBowls.resize(MixingBowlNo);
      }

      long long DrySum = 0;
      for (auto &Item : MainRecipeInfo->getDryIngredients())
      {
        if (!checkIngredientHasValue(Item, Item->DefLoc))
        {
          return CheffeErrorCode::CHEFFE_ERROR;
        }
        DrySum += Item->Value;
      }

      pushMixingBowlItem(std::make_pair(true, DrySum), MixingBowlNo - 1);
      break;
    }
    case MethodStepKind::Add:
    case MethodStepKind::Remove:
    case MethodStepKind::Combine:
    case MethodStepKind::Divide:
    {
      SourceLocation IngredientLoc;
      std::shared_ptr<CheffeIngredient> Ingredient = nullptr;

      const CheffeErrorCode Success =
          getIngredientInfo(MS->getOperand(0), Ingredient, IngredientLoc);
      if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
      {
        return Success;
      }

      if (!checkIngredientHasValue(Ingredient, IngredientLoc))
      {
        return CheffeErrorCode::CHEFFE_ERROR;
      }

      auto MixingBowl =
          std::static_pointer_cast<MixingBowlOp>(MS->getOperand(1));
      const unsigned MixingBowlNo = MixingBowl->getMixingBowlNo();

      const long long Value = Ingredient->Value;
      auto NewValue = popMixingBowlItem(MixingBowlNo - 1);

      switch (MS->getMethodStepKind())
      {
      default:
        cheffe_unreachable("Impossible operand code");
        break;
      case MethodStepKind::Add:
        NewValue.second += Value;
        break;
      case MethodStepKind::Remove:
        NewValue.second -= Value;
        break;
      case MethodStepKind::Combine:
        NewValue.second *= Value;
        break;
      case MethodStepKind::Divide:
        NewValue.second /= Value;
        break;
      }

      pushMixingBowlItem(NewValue, MixingBowlNo - 1);
      break;
    }
    case MethodStepKind::Pour:
    {
      // FIXME: No safety here if the indices are wrong!
      auto MixingBowl =
          std::static_pointer_cast<MixingBowlOp>(MS->getOperand(0));
      auto BakingDish =
          std::static_pointer_cast<BakingDishOp>(MS->getOperand(1));

      const unsigned MixingBowlNo = MixingBowl->getMixingBowlNo();
      const unsigned BakingDishNo = BakingDish->getBakingDishNo();

      // No point in trying to copy if the mixing bowl is empty
      if (MixingBowlNo > MixingBowls.size())
      {
        break;
      }

      if (BakingDishNo >= BakingDishes.size())
      {
        BakingDishes.resize(BakingDishNo);
      }

      for (auto &StackItem : MixingBowls[MixingBowlNo - 1])
      {
        pushBakingDishItem(StackItem, BakingDishNo - 1);
      }
      break;
    }
    case MethodStepKind::LiquifyBowl:
    {
      auto MixingBowl =
          std::static_pointer_cast<MixingBowlOp>(MS->getOperand(0));
      const unsigned MixingBowlNo = MixingBowl->getMixingBowlNo();
      // If we haven't put anything into this mixing bowl, don't bother trying
      // to loop
      if (MixingBowlNo > MixingBowls.size())
      {
        break;
      }

      for (auto &Item : MixingBowls[MixingBowlNo - 1])
      {
        Item.first = false;
      }
      break;
    }
    }
  }

  const unsigned ServesNo = MainRecipeInfo->getServesNo();

  bool HaveOutputAnything = false;
  for (unsigned i = 0; i < ServesNo && i < BakingDishes.size(); ++i)
  {
    while (!BakingDishes[i].empty())
    {
      HaveOutputAnything = true;
      auto Item = popBakingDishItem(i);
      if (Item.first)
      {
        std::cout << Item.second;
      }
      else
      {
        std::cout << (char)Item.second;
      }
    }
  }

  // The spec is a bit weird in that it says to output the contents of the
  // baking dish, even if those are numbers. Is the user supposed to add a
  // newline theirselves?
  if (HaveOutputAnything)
  {
    std::cout << std::endl;
  }

  CHEFFE_DEBUG(dbgs() << "Ending execution of '"
                      << MainRecipeInfo->getRecipeTitle()
                      << "'" << std::endl << std::endl);

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

} // end namespace cheffe
