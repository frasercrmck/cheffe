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
  auto IngredientInfo = std::make_unique<CheffeIngredient>(Ingredient);
  Ingredients[IngredientInfo->Name] = std::move(IngredientInfo);
}

CheffeIngredient *
CheffeRecipeInfo::getIngredient(const std::string &IngredientName) const
{
  auto Ingredient = Ingredients.find(IngredientName);
  if (Ingredient == std::end(Ingredients))
  {
    return nullptr;
  }
  return Ingredient->second.get();
}

std::vector<CheffeIngredient *> CheffeRecipeInfo::getDryIngredients() const
{
  std::vector<CheffeIngredient *> DryIngredients;
  for (auto &Ingredient : Ingredients)
  {
    if (Ingredient.second->RuntimeValueData.IsDry)
    {
      DryIngredients.push_back(Ingredient.second.get());
    }
  }

  return DryIngredients;
}

CheffeMethodStep *CheffeRecipeInfo::addNewMethodStep(const MethodStepKind Kind)
{
  auto MethodStep = std::make_unique<CheffeMethodStep>(Kind);
  MethodSteps.push_back(std::move(MethodStep));
  return MethodSteps.back().get();
}

CheffeMethodStep *CheffeRecipeInfo::getLastMethodStep() const
{
  return MethodSteps.empty() ? nullptr : MethodSteps.back().get();
}

void CheffeRecipeInfo::resetIngredientsToInitialValues()
{
  for (auto &Ingredient : Ingredients)
  {
    Ingredient.second->resetToInitialValue();
  }
}

std::vector<CheffeMethodStep *> CheffeRecipeInfo::getMethodStepList()
{
  std::vector<CheffeMethodStep *> MethodStepList;
  for (auto &MS : MethodSteps)
  {
    MethodStepList.push_back(MS.get());
  }
  return MethodStepList;
}

} // end namespace cheffe
