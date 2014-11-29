#ifndef CHEFFE_RECIPE_INFO
#define CHEFFE_RECIPE_INFO

#include "Parser/CheffeIngredient.h"

#include <map>

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

private:
  std::string RecipeTitle;
  std::map<std::string, CheffeIngredient> Ingredients;
};

#endif // CHEFFE_RECIPE_INFO
