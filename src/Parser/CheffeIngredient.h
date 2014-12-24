#ifndef CHEFFE_INGREDIENT
#define CHEFFE_INGREDIENT

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
  bool HasInitialValue = false;
  int InitialValue = 0;
  bool IsDry = true;
  std::string MeasureType = "";
  std::string Measure = "";
  std::string Name = "";
  friend std::ostream &operator<<(std::ostream &OS,
                                  const CheffeIngredient &Ingredient);
};

inline std::ostream &operator<<(std::ostream &OS,
                                const CheffeIngredient &Ingredient)
{
  if (Ingredient.HasInitialValue)
  {
    OS << Ingredient.InitialValue;
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
