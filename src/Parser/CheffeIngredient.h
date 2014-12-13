#ifndef CHEFFE_INGREDIENT
#define CHEFFE_INGREDIENT

#include <string>
#include <ostream>

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
  friend std::ostream &operator<<(std::ostream &stream,
                                  const CheffeIngredient &Ingredient);
};

inline std::ostream &operator<<(std::ostream &stream,
                                const CheffeIngredient &Ingredient)
{
  if (Ingredient.HasInitialValue)
  {
    stream << Ingredient.InitialValue;
  }
  else
  {
    stream << "<none>";
  }
  stream << ":";
  stream << "'" << Ingredient.Name << "'";
  stream << ":";
  stream << (Ingredient.IsDry ? "d" : "w");
  return stream;
}

#endif // CHEFFE_INGREDIENT
