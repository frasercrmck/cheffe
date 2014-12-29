#ifndef CHEFFE_RECIPE_INFO
#define CHEFFE_RECIPE_INFO

#include "Parser/CheffeMethodStep.h"

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

  void setServesNo(const int Serves);

  unsigned getServesNo() const;

  std::string getRecipeTitle() const;

  void addIngredientDefinition(const CheffeIngredient &Ingredient);

  std::shared_ptr<CheffeIngredient>
  getIngredient(const std::string &IngredientName) const;

  std::vector<std::shared_ptr<CheffeIngredient>> getDryIngredients() const;

  std::shared_ptr<CheffeMethodStep> addNewMethodStep(const MethodStepKind Kind);

  const std::vector<std::shared_ptr<CheffeMethodStep>> &getMethodStepList();

private:
  unsigned ServesNo;
  std::string RecipeTitle;
  std::map<std::string, std::shared_ptr<CheffeIngredient>> Ingredients;
  std::vector<std::shared_ptr<CheffeMethodStep>> MethodSteps;
};

} // end namespace cheffe

#endif // CHEFFE_RECIPE_INFO
