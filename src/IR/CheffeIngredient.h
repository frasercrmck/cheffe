#ifndef CHEFFE_INGREDIENT
#define CHEFFE_INGREDIENT

#include "Lexer/CheffeToken.h"

#include <string>
#include <ostream>

namespace cheffe
{

class CheffeIngredient
{
public:
  CheffeIngredient()
  {
  }
  bool HasValue = false;
  long long Value = 0;
  bool IsDry = true;
  std::string MeasureType = "";
  std::string Measure = "";
  std::string Name = "";
  SourceLocation DefLoc;
  friend std::ostream &operator<<(std::ostream &OS,
                                  const CheffeIngredient &Ingredient);
};

inline std::ostream &operator<<(std::ostream &OS,
                                const CheffeIngredient &Ingredient)
{
  if (Ingredient.HasValue)
  {
    OS << Ingredient.Value;
  }
  else
  {
    OS << "<none>";
  }
  OS << ":";
  OS << "'" << Ingredient.Name << "'";
  OS << ":";
  OS << (Ingredient.IsDry ? "d" : "w");
  return OS;
}

} // end namespace cheffe

#endif // CHEFFE_INGREDIENT
