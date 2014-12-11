#ifndef CHEFFE_RECIPE_INFO
#define CHEFFE_RECIPE_INFO

#include "Parser/CheffeIngredient.h"
#include "Parser/CheffeMethodStep.h"

#include <map>
#include <vector>

class CheffeRecipeInfo
{
public:
  CheffeRecipeInfo(const std::string &Title) : RecipeTitle(Title)
  {
  }

  std::string getRecipeTitle()
  {
    return RecipeTitle;
  }

  void addIngredient(const CheffeIngredient &Ingredient)
  {
    // It's alright to overwrite an existing ingredient; it's in the spec
    Ingredients[Ingredient.Name] = Ingredient;
  }

  bool wasIngredientDefined(const std::string &Ingredient)
  {
    return Ingredients.find(Ingredient) != std::end(Ingredients);
  }

  std::shared_ptr<CheffeMethodStep> addNewMethodStep(const MethodStepKind Kind)
  {
    auto MethodStep =
        std::shared_ptr<CheffeMethodStep>(new CheffeMethodStep(Kind));
    MethodSteps.push_back(MethodStep);
    return MethodStep;
  }

  const std::vector<std::shared_ptr<CheffeMethodStep>> &getMethodStepList()
  {
    return MethodSteps;
  }

private:
  std::string RecipeTitle;
  std::map<std::string, CheffeIngredient> Ingredients;
  std::vector<std::shared_ptr<CheffeMethodStep>> MethodSteps;
};

#endif // CHEFFE_RECIPE_INFO
