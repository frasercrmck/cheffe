#ifndef CHEFFE_METHOD_STEP
#define CHEFFE_METHOD_STEP

#include "Parser/CheffeIngredient.h"

#include <vector>
#include <ostream>
#include <memory>
#include <cassert>

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

static std::string getMethodStepKindAsString(const MethodStepKind Kind)
{
  switch (Kind)
  {
  case MethodStepKind::Take:
    return "TAKE";
  case MethodStepKind::Put:
    return "PUT";
  case MethodStepKind::Fold:
    return "FOLD";
  case MethodStepKind::Add:
    return "ADD";
  case MethodStepKind::Remove:
    return "REMOVE";
  case MethodStepKind::Combine:
    return "COMBINE";
  case MethodStepKind::Divide:
    return "DIVIDE";
  case MethodStepKind::AddDry:
    return "ADD_DRY";
  case MethodStepKind::LiquifyBowl:
    return "LIQUIFY_BOWL";
  case MethodStepKind::LiquifyIngredient:
    return "LIQUIFY_INGREDIENT";
  case MethodStepKind::StirBowl:
    return "STIR_BOWL";
  case MethodStepKind::StirIngredient:
    return "STIR_INGREDIENT";
  case MethodStepKind::Mix:
    return "MIX";
  case MethodStepKind::Clean:
    return "CLEAN";
  case MethodStepKind::Pour:
    return "POUR";
  case MethodStepKind::Verb:
    return "VERB";
  case MethodStepKind::UntilVerbed:
    return "UNTIL_VERBED";
  case MethodStepKind::SetAside:
    return "SET_ASIDE";
  case MethodStepKind::Serve:
    return "SERVE";
  case MethodStepKind::Refrigerate:
    return "REFRIGERATE";
  case MethodStepKind::Invalid:
    return "INVALID";
  }
}

class MethodOp
{
public:
  MethodOp()
  {
  }

  virtual void dump(std::ostream &OS) const
  {
    (void)OS;
  }
};

class IngredientOp : public MethodOp
{
public:
  IngredientOp() : MethodOp(), IsUndefined(true), Ingredient(nullptr)
  {
  }

  IngredientOp(const bool IsUndefined,
               const std::shared_ptr<CheffeIngredient> &Ingredient)
      : MethodOp(), IsUndefined(IsUndefined), Ingredient(Ingredient)
  {
  }

  void dump(std::ostream &OS) const override
  {
    OS << "(Ingredient ";
    if (IsUndefined)
    {
      OS << "<undefined>";
    }
    else
    {
      OS << *Ingredient;
    }
    OS << ")";
  }

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

  void dump(std::ostream &OS) const override
  {
    OS << "(MixingBowl " << MixingBowlNo << ")";
  }

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

  void dump(std::ostream &OS) const override
  {
    OS << "(BakingDish " << BakingDishNo << ")";
  }

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

  void dump(std::ostream &OS) const override
  {
    OS << "(Number " << NumberValue << ")";
  }

private:
  int NumberValue = 0;
};

class CheffeMethodStep
{
public:
  CheffeMethodStep(const MethodStepKind Kind) : Kind(Kind)
  {
  }

  void addIngredient(
      const std::pair<bool, std::shared_ptr<CheffeIngredient>> &IngredientInfo)
  {
    addIngredient(IngredientInfo.first, IngredientInfo.second);
  }

  void addIngredient(const bool IsUndefined,
                     const std::shared_ptr<CheffeIngredient> &Ingredient)
  {
    assert(((IsUndefined && Ingredient == nullptr) ||
            (!IsUndefined && Ingredient != nullptr)) &&
           "Invalid ingredient information");
    MethodOps.push_back(std::unique_ptr<IngredientOp>(
        new IngredientOp(IsUndefined, Ingredient)));
  }

  void addMixingBowl(const unsigned MixingBowlNo)
  {
    MethodOps.push_back(
        std::unique_ptr<MixingBowlOp>(new MixingBowlOp(MixingBowlNo)));
  }

  void addBakingDish(const unsigned BakingDishNo)
  {
    MethodOps.push_back(
        std::unique_ptr<BakingDishOp>(new BakingDishOp(BakingDishNo)));
  }

  void addNumber(const int NumberValue)
  {
    MethodOps.push_back(std::unique_ptr<NumberOp>(new NumberOp(NumberValue)));
  }

  friend std::ostream &operator<<(std::ostream &OS,
                                  const CheffeMethodStep &MethodStep)
  {
    OS << getMethodStepKindAsString(MethodStep.Kind);
    for (auto &Op : MethodStep.MethodOps)
    {
      OS << ", ";
      Op->dump(OS);
    }
    OS << std::endl;
    return OS;
  }

private:
  MethodStepKind Kind;
  std::vector<std::unique_ptr<MethodOp>> MethodOps;
};

#endif // CHEFFE_METHOD_STEP
