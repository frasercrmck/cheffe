#ifndef CHEFFE_PROGRAM_INFO
#define CHEFFE_PROGRAM_INFO

#include "IR/CheffeRecipeInfo.h"

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
  getRecipe(const std::string &RecipeTitle) const;

  std::shared_ptr<CheffeRecipeInfo> getEntryPointRecipe();

  void addRecipe(const std::string &RecipeTitle,
                 std::shared_ptr<CheffeRecipeInfo> Recipe);

  void setEntryPointRecipeTitleIfNone(const std::string &RecipeTitle);

private:
  RecipeMapTy RecipeInfo;
  std::string EntryPointRecipeTitle;
};

} // end namespace cheffe

#endif // CHEFFE_PROGRAM_INFO
