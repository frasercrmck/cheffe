#ifndef CHEFFE_COMMON
#define CHEFFE_COMMON

#include <map>
#include <string>

namespace cheffe
{

class CheffeRecipeInfo;

typedef std::map<std::string, std::unique_ptr<CheffeRecipeInfo>> RecipeMap;

}; // end namespace cheffe

#endif // CHEFFE_COMMON
