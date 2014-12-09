#ifndef CHEFFE_METHOD_STEP
#define CHEFFE_METHOD_STEP

#include <ostream>

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
    return "Take";
  case MethodStepKind::Put:
    return "Put";
  case MethodStepKind::Fold:
    return "Fold";
  case MethodStepKind::Add:
    return "Add";
  case MethodStepKind::Remove:
    return "Remove";
  case MethodStepKind::Combine:
    return "Combine";
  case MethodStepKind::Divide:
    return "Divide";
  case MethodStepKind::AddDry:
    return "AddDry";
  case MethodStepKind::LiquifyBowl:
    return "LiquifyBowl";
  case MethodStepKind::LiquifyIngredient:
    return "LiquifyIngredient";
  case MethodStepKind::StirBowl:
    return "StirBowl";
  case MethodStepKind::StirIngredient:
    return "StirIngredient";
  case MethodStepKind::Mix:
    return "Mix";
  case MethodStepKind::Clean:
    return "Clean";
  case MethodStepKind::Pour:
    return "Pour";
  case MethodStepKind::Verb:
    return "Verb";
  case MethodStepKind::UntilVerbed:
    return "UntilVerbed";
  case MethodStepKind::SetAside:
    return "SetAside";
  case MethodStepKind::Serve:
    return "Serve";
  case MethodStepKind::Refrigerate:
    return "Refrigerate";
  case MethodStepKind::Invalid:
    return "Invalid";
  }
}

class CheffeMethodStep
{
public:
  CheffeMethodStep(const MethodStepKind Kind) : Kind(Kind)
  {
  }

  friend std::ostream &operator<<(std::ostream &stream,
                                  const CheffeMethodStep &MethodStep)
  {
    stream << getMethodStepKindAsString(MethodStep.Kind) << std::endl;
    return stream;
  }

private:
  MethodStepKind Kind;
};

#endif // CHEFFE_METHOD_STEP
