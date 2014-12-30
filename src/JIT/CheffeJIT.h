#ifndef CHEFFE_JIT
#define CHEFFE_JIT

#include "cheffe.h"
#include "CheffeCommon.h"
#include "IR/CheffeIngredient.h"
#include "IR/CheffeMethodStep.h"
#include "IR/CheffeProgramInfo.h"
#include "Utils/CheffeDiagnosticHandler.h"

#include <deque>

namespace cheffe
{

class CheffeJIT
{
private:
  typedef std::pair<bool, long long> StackItemTy;
  typedef std::deque<StackItemTy> StackTy;

public:
  CheffeJIT(std::unique_ptr<CheffeProgramInfo> ProgramInfo,
            std::shared_ptr<CheffeDiagnosticHandler> Diags)
      : ProgramInfo(std::move(ProgramInfo)), Diagnostics(Diags)
  {
  }

  CheffeErrorCode executeProgram();
  CheffeErrorCode executeRecipe(std::shared_ptr<CheffeRecipeInfo> RecipeInfo,
                                std::vector<StackTy> &CallerMixingBowls,
                                std::vector<StackTy> &CallerBakingDishes);

private:
  std::unique_ptr<CheffeProgramInfo> ProgramInfo;
  std::shared_ptr<CheffeDiagnosticHandler> Diagnostics;

  void pushStackItem(std::vector<StackTy> &Stack, const StackItemTy StackItem,
                     const unsigned StackIdx);
  StackItemTy popStackItem(std::vector<StackTy> &Stack,
                           const unsigned StackItemIdx);

  bool
  checkIngredientHasValue(const std::shared_ptr<CheffeIngredient> &Ingredient,
                          const SourceLocation IngredientLoc);
  CheffeErrorCode
  getIngredientInfo(const std::shared_ptr<MethodOp> &MOp,
                    std::shared_ptr<CheffeIngredient> &IngredientInfo,
                    SourceLocation &IngredientLoc);
};

} // end namespace cheffe

#endif // CHEFFE_JIT
