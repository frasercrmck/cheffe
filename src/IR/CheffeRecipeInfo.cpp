#include "IR/CheffeRecipeInfo.h"
#include "IR/CheffeIngredient.h"

namespace cheffe
{

void CheffeRecipeInfo::setServesNo(const unsigned Serves)
{
  ServesNo = Serves;
}

unsigned CheffeRecipeInfo::getServesNo() const
{
  return ServesNo;
}

std::string CheffeRecipeInfo::getRecipeTitle() const
{
  return RecipeTitle;
}

void CheffeRecipeInfo::addIngredientDefinition(
    const CheffeIngredient &Ingredient)
{
  // It's alright to overwrite an existing ingredient; it's in the spec
  auto IngredientInfo =
      std::shared_ptr<CheffeIngredient>(new CheffeIngredient(Ingredient));
  Ingredients[IngredientInfo->Name] = std::move(IngredientInfo);
}

std::shared_ptr<CheffeIngredient>
CheffeRecipeInfo::getIngredient(const std::string &IngredientName) const
{
  auto Ingredient = Ingredients.find(IngredientName);
  if (Ingredient == std::end(Ingredients))
  {
    return nullptr;
  }
  return Ingredient->second;
}

std::vector<std::shared_ptr<CheffeIngredient>>
CheffeRecipeInfo::getDryIngredients() const
{
  std::vector<std::shared_ptr<CheffeIngredient>> DryIngredients;
  for (auto &Ingredient : Ingredients)
  {
    if (Ingredient.second->RuntimeValueData.IsDry)
    {
      DryIngredients.push_back(Ingredient.second);
    }
  }

  return DryIngredients;
}

std::shared_ptr<CheffeMethodStep>
CheffeRecipeInfo::addNewMethodStep(const MethodStepKind Kind)
{
  auto MethodStep =
      std::shared_ptr<CheffeMethodStep>(new CheffeMethodStep(Kind));
  MethodSteps.push_back(MethodStep);
  return MethodStep;
}

std::shared_ptr<CheffeMethodStep> CheffeRecipeInfo::getLastMethodStep() const
{
  return MethodSteps.empty() ? nullptr : MethodSteps.back();
}

void CheffeRecipeInfo::resetIngredientsToInitialValues()
{
  for (auto Ingredient : Ingredients)
  {
    Ingredient.second->resetToInitialValue();
  }
}

const std::vector<std::shared_ptr<CheffeMethodStep>> &
CheffeRecipeInfo::getMethodStepList()
{
  return MethodSteps;
}

} // end namespace cheffe
