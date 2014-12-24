#include "Parser/CheffeMethodStep.h"
#include "Parser/CheffeIngredient.h"

namespace cheffe
{

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

void MethodOp::dump(std::ostream &OS) const
{
  (void)OS;
}

void IngredientOp::dump(std::ostream &OS) const
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

void MixingBowlOp::dump(std::ostream &OS) const
{
  OS << "(MixingBowl " << MixingBowlNo << ")";
}

void BakingDishOp::dump(std::ostream &OS) const
{
  OS << "(BakingDish " << BakingDishNo << ")";
}

void NumberOp::dump(std::ostream &OS) const
{
  OS << "(Number " << NumberValue << ")";
}

void RecipeOp::dump(std::ostream &OS) const
{
  OS << "(Recipe '" << RecipeName << "')";
}

void CheffeMethodStep::addIngredient(
    const std::pair<bool, std::shared_ptr<CheffeIngredient>> &IngredientInfo)
{
  addIngredient(IngredientInfo.first, IngredientInfo.second);
}

void CheffeMethodStep::addIngredient(
    const bool IsUndefined, const std::shared_ptr<CheffeIngredient> &Ingredient)
{
  assert(((IsUndefined && Ingredient == nullptr) ||
          (!IsUndefined && Ingredient != nullptr)) &&
         "Invalid ingredient information");
  MethodOps.push_back(
      std::unique_ptr<IngredientOp>(new IngredientOp(IsUndefined, Ingredient)));
}

void CheffeMethodStep::addMixingBowl(const unsigned MixingBowlNo)
{
  MethodOps.push_back(
      std::unique_ptr<MixingBowlOp>(new MixingBowlOp(MixingBowlNo)));
}

void CheffeMethodStep::addBakingDish(const unsigned BakingDishNo)
{
  MethodOps.push_back(
      std::unique_ptr<BakingDishOp>(new BakingDishOp(BakingDishNo)));
}

void CheffeMethodStep::addNumber(const int NumberValue)
{
  MethodOps.push_back(std::unique_ptr<NumberOp>(new NumberOp(NumberValue)));
}

void CheffeMethodStep::addRecipe(const std::string &RecipeName)
{
  MethodOps.push_back(std::unique_ptr<RecipeOp>(new RecipeOp(RecipeName)));
}

std::ostream &operator<<(std::ostream &OS, const CheffeMethodStep &MethodStep)
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

} // end namespace cheffe
