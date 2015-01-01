#include "IR/CheffeMethodStep.h"
#include "IR/CheffeIngredient.h"

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
  case MethodStepKind::LiquefyBowl:
    return "LIQUEFY_BOWL";
  case MethodStepKind::LiquefyIngredient:
    return "LIQUEFY_INGREDIENT";
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

std::shared_ptr<CheffeIngredient> IngredientOp::getIngredient() const
{
  return Ingredient;
}

SourceLocation IngredientOp::getSourceLoc() const
{
  return SourceLoc;
}

void IngredientOp::dump(std::ostream &OS) const
{
  OS << "(Ingredient ";
  if (Ingredient)
  {
    OS << *Ingredient;
  }
  else
  {
    OS << "<undef>";
  }
  OS << ")";
}

unsigned MixingBowlOp::getMixingBowlNo() const
{
  return MixingBowlNo;
}

void MixingBowlOp::dump(std::ostream &OS) const
{
  OS << "(MixingBowl " << MixingBowlNo << ")";
}

unsigned BakingDishOp::getBakingDishNo() const
{
  return BakingDishNo;
}

void BakingDishOp::dump(std::ostream &OS) const
{
  OS << "(BakingDish " << BakingDishNo << ")";
}

long long NumberOp::getNumberValue() const
{
  return NumberValue;
}

void NumberOp::dump(std::ostream &OS) const
{
  OS << "(Number " << NumberValue << ")";
}

SourceLocation RecipeOp::getSourceLoc() const
{
  return SourceLoc;
}

std::string RecipeOp::getRecipeName() const
{
  return RecipeName;
}

void RecipeOp::dump(std::ostream &OS) const
{
  OS << "(Recipe '" << RecipeName << "')";
}

std::shared_ptr<MethodOp> CheffeMethodStep::getOperand(const unsigned Idx) const
{
  assert(Idx < MethodOps.size() && "Invalid operand access!");
  return MethodOps[Idx];
}

MethodStepKind CheffeMethodStep::getMethodStepKind() const
{
  return Kind;
}

SourceLocation CheffeMethodStep::getSourceLoc() const
{
  return SourceLoc;
}

void CheffeMethodStep::setSourceLoc(const SourceLocation Loc)
{
  SourceLoc = Loc;
}

void CheffeMethodStep::addIngredient(
    const std::shared_ptr<CheffeIngredient> &Ingredient,
    const SourceLocation SourceLoc)
{
  MethodOps.push_back(std::make_shared<IngredientOp>(Ingredient, SourceLoc));
}

void CheffeMethodStep::addIngredient(std::shared_ptr<IngredientOp> IngredientOp)
{
  assert(IngredientOp != nullptr && "Invalid ingredient information");
  addIngredient(IngredientOp->getIngredient(), IngredientOp->getSourceLoc());
}

void CheffeMethodStep::addMixingBowl(const unsigned MixingBowlNo)
{
  MethodOps.push_back(std::make_shared<MixingBowlOp>(MixingBowlNo));
}

void CheffeMethodStep::addBakingDish(const unsigned BakingDishNo)
{
  MethodOps.push_back(std::make_shared<BakingDishOp>(BakingDishNo));
}

void CheffeMethodStep::addNumber(const long long NumberValue)
{
  MethodOps.push_back(std::make_shared<NumberOp>(NumberValue));
}

void CheffeMethodStep::addRecipe(const std::string &RecipeName,
                                 const SourceLocation SourceLoc)
{
  MethodOps.push_back(std::make_shared<RecipeOp>(RecipeName, SourceLoc));
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
