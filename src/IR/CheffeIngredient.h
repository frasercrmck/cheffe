#ifndef CHEFFE_INGREDIENT
#define CHEFFE_INGREDIENT

#include "Lexer/CheffeToken.h"

#include <string>
#include <ostream>

namespace cheffe
{

struct ValueData
{
  bool HasValue = false;
  long long Value = 0;
  bool IsDry = true;
};

class CheffeIngredient
{
  friend class CheffeParser;

public:
  CheffeIngredient()
  {
  }

private:
  ValueData InitialValueData;

public:
  ValueData RuntimeValueData;
  std::string MeasureType = "";
  std::string Measure = "";
  std::string Name = "";
  SourceLocation DefLoc;
  friend std::ostream &operator<<(std::ostream &OS,
                                  const CheffeIngredient &Ingredient);
  void resetToInitialValue()
  {
    RuntimeValueData.IsDry = InitialValueData.IsDry;
    RuntimeValueData.Value = InitialValueData.Value;
    RuntimeValueData.HasValue = InitialValueData.HasValue;
  }
};

inline std::ostream &operator<<(std::ostream &OS,
                                const CheffeIngredient &Ingredient)
{
  if (Ingredient.RuntimeValueData.HasValue)
  {
    OS << Ingredient.RuntimeValueData.Value;
  }
  else
  {
    OS << "<none>";
  }
  OS << ":";
  OS << "'" << Ingredient.Name << "'";
  OS << ":";
  OS << (Ingredient.RuntimeValueData.IsDry ? "d" : "w");
  return OS;
}

} // end namespace cheffe

#endif // CHEFFE_INGREDIENT
