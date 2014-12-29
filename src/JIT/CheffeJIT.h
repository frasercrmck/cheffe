#ifndef CHEFFE_JIT
#define CHEFFE_JIT

#include "cheffe.h"
#include "CheffeCommon.h"
#include "IR/CheffeIngredient.h"
#include "IR/CheffeMethodStep.h"
#include "IR/CheffeProgramInfo.h"
#include "Utils/CheffeDiagnosticHandler.h"

namespace cheffe
{

class CheffeJIT
{
public:
  CheffeJIT(std::unique_ptr<CheffeProgramInfo> ProgramInfo,
            std::shared_ptr<CheffeDiagnosticHandler> Diags)
      : ProgramInfo(std::move(ProgramInfo)), Diagnostics(Diags)
  {
  }

  CheffeErrorCode executeRecipe();

private:
  std::unique_ptr<CheffeProgramInfo> ProgramInfo;
  std::shared_ptr<CheffeDiagnosticHandler> Diagnostics;

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
