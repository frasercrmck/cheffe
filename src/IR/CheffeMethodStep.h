#ifndef CHEFFE_METHOD_STEP
#define CHEFFE_METHOD_STEP

#include "Lexer/CheffeToken.h"
#include "IR/CheffeIngredient.h"

#include <vector>
#include <ostream>
#include <memory>
#include <cassert>

namespace cheffe
{

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
  LiquefyBowl,
  LiquefyIngredient,
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
  IngredientOp() : MethodOp(), Ingredient(nullptr)
  {
  }

  ~IngredientOp() override
  {
  }

  IngredientOp(CheffeIngredient *Ingredient, const SourceLocation SourceLoc)
      : MethodOp(), Ingredient(Ingredient), SourceLoc(SourceLoc)
  {
  }

  CheffeIngredient *getIngredient() const;

  SourceLocation getSourceLoc() const;

  void dump(std::ostream &OS) const override;

private:
  CheffeIngredient *Ingredient;
  SourceLocation SourceLoc;
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

  unsigned getMixingBowlNo() const;

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

  unsigned getBakingDishNo() const;

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

  NumberOp(const long long Value) : MethodOp(), NumberValue(Value)
  {
  }

  long long getNumberValue() const;

  void dump(std::ostream &OS) const override;

private:
  long long NumberValue = 0;
};

class RecipeOp : public MethodOp
{
public:
  RecipeOp() : MethodOp(), RecipeName("")
  {
  }

  RecipeOp(const std::string &Recipe, const SourceLocation SourceLoc)
      : MethodOp(), RecipeName(Recipe), SourceLoc(SourceLoc)
  {
  }

  std::string getRecipeName() const;

  SourceLocation getSourceLoc() const;

  void dump(std::ostream &OS) const override;

private:
  std::string RecipeName = "";
  SourceLocation SourceLoc;
};

class CheffeMethodStep
{
public:
  CheffeMethodStep(const MethodStepKind Kind) : Kind(Kind)
  {
  }

  MethodStepKind getMethodStepKind() const;

  SourceLocation getSourceLoc() const;
  void setSourceLoc(const SourceLocation Loc);

  MethodOp *getOperand(const unsigned Idx) const;

  void addIngredient(CheffeIngredient *IngredientInfo,
                     const SourceLocation SourceLoc);

  void addIngredient(IngredientOp *IngredientOp);

  void addMixingBowl(const unsigned MixingBowlNo);

  void addBakingDish(const unsigned BakingDishNo);

  void addNumber(const long long NumberValue);

  void addRecipe(const std::string &RecipeName, const SourceLocation SourceLoc);

  friend std::ostream &operator<<(std::ostream &OS,
                                  const CheffeMethodStep &MethodStep);

private:
  MethodStepKind Kind;
  SourceLocation SourceLoc;
  std::vector<std::unique_ptr<MethodOp>> MethodOps;
};

} // end namespace cheffe

#endif // CHEFFE_METHOD_STEP
