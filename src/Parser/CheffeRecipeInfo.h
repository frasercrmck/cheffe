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
    auto IngredientInfo =
        std::shared_ptr<CheffeIngredient>(new CheffeIngredient(Ingredient));
    Ingredients[IngredientInfo->Name] = std::move(IngredientInfo);
  }

  std::shared_ptr<CheffeIngredient> getIngredient(const std::string &IngredientName)
  {
    auto Ingredient = Ingredients.find(IngredientName);
    if (Ingredient == std::end(Ingredients))
    {
      return nullptr;
    }
    return Ingredient->second;
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
  std::map<std::string, std::shared_ptr<CheffeIngredient>> Ingredients;
  std::vector<std::shared_ptr<CheffeMethodStep>> MethodSteps;
};

#endif // CHEFFE_RECIPE_INFO
