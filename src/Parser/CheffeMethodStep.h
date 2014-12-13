#ifndef CHEFFE_METHOD_STEP
#define CHEFFE_METHOD_STEP

#include <vector>
#include <ostream>
#include <memory>

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

  virtual void dump(std::ostream &os) const
  {
    (void)os;
  }
};

class MixingBowl : public MethodOp
{
public:
  MixingBowl() : MethodOp(), MixingBowlNo(1)
  {
  }

  MixingBowl(const unsigned MixingBowl) : MethodOp(), MixingBowlNo(MixingBowl)
  {
  }

  void dump(std::ostream &os) const override
  {
    os << "(MixingBowl " << MixingBowlNo << ")";
  }

private:
  unsigned MixingBowlNo;
};

class BakingDish : public MethodOp
{
public:
  BakingDish() : MethodOp(), BakingDishNo(1)
  {
  }

  BakingDish(const unsigned BakingDish) : MethodOp(), BakingDishNo(BakingDish)
  {
  }

  void dump(std::ostream &os) const override
  {
    os << "(BakingDish " << BakingDishNo << ")";
  }

private:
  unsigned BakingDishNo;
};

class CheffeMethodStep
{
public:
  CheffeMethodStep(const MethodStepKind Kind) : Kind(Kind)
  {
  }

  void addMixingBowl(const unsigned MixingBowlNo)
  {
    MethodOps.push_back(
        std::unique_ptr<MixingBowl>(new MixingBowl(MixingBowlNo)));
  }

  void addBakingDish(const unsigned BakingDishNo)
  {
    MethodOps.push_back(
        std::unique_ptr<BakingDish>(new BakingDish(BakingDishNo)));
  }

  friend std::ostream &operator<<(std::ostream &stream,
                                  const CheffeMethodStep &MethodStep)
  {
    stream << getMethodStepKindAsString(MethodStep.Kind);
    for (auto &Op : MethodStep.MethodOps)
    {
      stream << ", ";
      Op->dump(stream);
    }
    stream << std::endl;
    return stream;
  }

private:
  MethodStepKind Kind;
  std::vector<std::unique_ptr<MethodOp>> MethodOps;
};

#endif // CHEFFE_METHOD_STEP
