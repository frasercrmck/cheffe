#include "Utils/CheffeUtils.h"
#include "IR/CheffeProgramInfo.h"

namespace cheffe
{

std::shared_ptr<CheffeRecipeInfo>
CheffeProgramInfo::getRecipe(const std::string &RecipeTitle) const
{
  auto RecipeIter =
      std::find_if(std::begin(RecipeInfo), std::end(RecipeInfo),
                   [&RecipeTitle](const decltype(RecipeInfo)::value_type Recipe)
                   {
        return AreLowerCasedStringsEqual(RecipeTitle, Recipe.first);
      });
  return RecipeIter == std::end(RecipeInfo) ? nullptr : RecipeIter->second;
}

std::shared_ptr<CheffeRecipeInfo> CheffeProgramInfo::getEntryPointRecipe()
{
  return getRecipe(EntryPointRecipeTitle);
}

void CheffeProgramInfo::addRecipe(const std::string &RecipeTitle,
                                  std::shared_ptr<CheffeRecipeInfo> Recipe)
{
  RecipeInfo.insert(std::make_pair(RecipeTitle, Recipe));
}

void CheffeProgramInfo::setEntryPointRecipeTitleIfNone(
    const std::string &RecipeTitle)
{
  if (EntryPointRecipeTitle.empty())
  {
    EntryPointRecipeTitle = RecipeTitle;
  }
}

} // end namespace cheffe
