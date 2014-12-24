#ifndef CHEFFE_JIT
#define CHEFFE_JIT

#include "cheffe.h"
#include "CheffeCommon.h"

namespace cheffe
{

class CheffeJIT
{
public:
  CheffeJIT(std::unique_ptr<RecipeMap> RecipeMap,
            const std::string &MainRecipeTitle)
      : RecipeMap(std::move(RecipeMap)), MainRecipeTitle(MainRecipeTitle)
  {
  }

  CheffeErrorCode executeRecipe();

private:
  std::unique_ptr<RecipeMap> RecipeMap;
  std::string MainRecipeTitle;
};

} // end namespace cheffe

#endif // CHEFFE_JIT
