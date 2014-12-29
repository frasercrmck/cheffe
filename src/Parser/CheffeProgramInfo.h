#ifndef CHEFFE_PROGRAM_INFO
#define CHEFFE_PROGRAM_INFO

#include "Parser/CheffeRecipeInfo.h"

#include <map>
#include <string>

namespace cheffe
{

class CheffeProgramInfo
{
private:
  typedef std::map<std::string, std::shared_ptr<CheffeRecipeInfo>> RecipeMapTy;

public:
  std::shared_ptr<CheffeRecipeInfo>
  getRecipe(const std::string &RecipeTitle) const
  {
    auto RecipeIter = RecipeInfo.find(RecipeTitle);
    return RecipeIter == std::end(RecipeInfo) ? nullptr : RecipeIter->second;
  }

  std::shared_ptr<CheffeRecipeInfo> getEntryPointRecipe()
  {
    return getRecipe(EntryPointRecipeTitle);
  }

  void addRecipe(const std::string &RecipeTitle,
                 std::shared_ptr<CheffeRecipeInfo> Recipe)
  {
    RecipeInfo.insert(std::make_pair(RecipeTitle, Recipe));
  }

  void setEntryPointRecipeTitleIfNone(const std::string &RecipeTitle)
  {
    if (EntryPointRecipeTitle.empty())
    {
      EntryPointRecipeTitle = RecipeTitle;
    }
  }

private:
  RecipeMapTy RecipeInfo;
  std::string EntryPointRecipeTitle;
};

} // end namespace cheffe

#endif // CHEFFE_PROGRAM_INFO
