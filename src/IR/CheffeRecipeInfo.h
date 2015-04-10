#ifndef CHEFFE_RECIPE_INFO
#define CHEFFE_RECIPE_INFO

#include "IR/CheffeMethodStep.h"

#include <map>
#include <vector>
#include <string>

namespace cheffe
{

class CheffeIngredient;

class CheffeRecipeInfo
{
public:
  CheffeRecipeInfo() = delete;

  CheffeRecipeInfo(const std::string &Title) : ServesNo(0), RecipeTitle(Title)
  {
  }

  void setServesNo(const unsigned Serves);

  unsigned getServesNo() const;

  std::string getRecipeTitle() const;

  void addIngredientDefinition(const CheffeIngredient &Ingredient);

  CheffeIngredient *getIngredient(const std::string &IngredientName) const;

  std::vector<CheffeIngredient *> getDryIngredients() const;

  CheffeMethodStep *addNewMethodStep(const MethodStepKind Kind);

  CheffeMethodStep *getLastMethodStep() const;

  std::vector<CheffeMethodStep *> getMethodStepList();

  void resetIngredientsToInitialValues();

private:
  unsigned ServesNo;
  std::string RecipeTitle;
  std::map<std::string, std::unique_ptr<CheffeIngredient>> Ingredients;
  std::vector<std::unique_ptr<CheffeMethodStep>> MethodSteps;
};

} // end namespace cheffe

#endif // CHEFFE_RECIPE_INFO
