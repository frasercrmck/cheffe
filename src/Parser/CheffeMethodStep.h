#ifndef CHEFFE_METHOD_STEP
#define CHEFFE_METHOD_STEP

#include <vector>
#include <ostream>
#include <memory>
#include <cassert>

class CheffeIngredient;

enum class MethodStepKind
{
  Take,
  Put,
  Fold,
  Add,
  Remove,
  Combine,
  Divide,
  AddDry,
  LiquifyBowl,
  LiquifyIngredient,
  StirBowl,
  StirIngredient,
  Mix,
  Clean,
  Pour,
  Verb,
  UntilVerbed,
  SetAside,
  Serve,
  Refrigerate,
  Invalid
};

class MethodOp
{
public:
  MethodOp()
  {
  }

  virtual ~MethodOp()
  {
  }

  virtual void dump(std::ostream &OS) const;
};

class IngredientOp : public MethodOp
{
public:
  IngredientOp() : MethodOp(), IsUndefined(true), Ingredient(nullptr)
  {
  }

  ~IngredientOp() override
  {
  }

  IngredientOp(const bool IsUndefined,
               const std::shared_ptr<CheffeIngredient> &Ingredient)
      : MethodOp(), IsUndefined(IsUndefined), Ingredient(Ingredient)
  {
  }

  void dump(std::ostream &OS) const override;

private:
  bool IsUndefined;
  std::shared_ptr<CheffeIngredient> Ingredient;
};

class MixingBowlOp : public MethodOp
{
public:
  MixingBowlOp() : MethodOp(), MixingBowlNo(1)
  {
  }

  MixingBowlOp(const unsigned MixingBowl) : MethodOp(), MixingBowlNo(MixingBowl)
  {
  }

  void dump(std::ostream &OS) const override;

private:
  unsigned MixingBowlNo;
};

class BakingDishOp : public MethodOp
{
public:
  BakingDishOp() : MethodOp(), BakingDishNo(1)
  {
  }

  BakingDishOp(const unsigned BakingDish) : MethodOp(), BakingDishNo(BakingDish)
  {
  }

  void dump(std::ostream &OS) const override;

private:
  unsigned BakingDishNo = 1;
};

class NumberOp : public MethodOp
{
public:
  NumberOp() : MethodOp(), NumberValue(0)
  {
  }

  NumberOp(const int Value) : MethodOp(), NumberValue(Value)
  {
  }

  void dump(std::ostream &OS) const override;

private:
  int NumberValue = 0;
};

class RecipeOp : public MethodOp
{
public:
  RecipeOp() : MethodOp(), RecipeName("")
  {
  }

  RecipeOp(const std::string &Recipe) : MethodOp(), RecipeName(Recipe)
  {
  }

  void dump(std::ostream &OS) const override;

private:
  std::string RecipeName = "";
};

class CheffeMethodStep
{
public:
  CheffeMethodStep(const MethodStepKind Kind) : Kind(Kind)
  {
  }

  void addIngredient(
      const std::pair<bool, std::shared_ptr<CheffeIngredient>> &IngredientInfo);

  void addIngredient(const bool IsUndefined,
                     const std::shared_ptr<CheffeIngredient> &Ingredient);

  void addMixingBowl(const unsigned MixingBowlNo);

  void addBakingDish(const unsigned BakingDishNo);

  void addNumber(const int NumberValue);

  void addRecipe(const std::string &RecipeName);

  friend std::ostream &operator<<(std::ostream &OS,
                                  const CheffeMethodStep &MethodStep);

private:
  MethodStepKind Kind;
  std::vector<std::unique_ptr<MethodOp>> MethodOps;
};

#endif // CHEFFE_METHOD_STEP
