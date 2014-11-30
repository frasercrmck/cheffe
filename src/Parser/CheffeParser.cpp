#include "Parser/CheffeParser.h"
#include "Utils/CheffeDebugUtils.h"

#include <cassert>
#include <algorithm>

#define DEBUG_TYPE "parser"

namespace cheffe
{

Token CheffeParser::getNextToken()
{
  return CurrentToken = Lexer.getToken();
}

CheffeErrorCode CheffeParser::parseRecipe()
{
  CheffeErrorCode Success = CheffeErrorCode::CHEFFE_SUCCESS;

  do
  {
    std::string RecipeTitle;
    SourceLocation RecipeTitleLoc;
    Success = parseRecipeTitle(RecipeTitle, RecipeTitleLoc);
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }

    if (RecipeInfo.find(RecipeTitle) != std::end(RecipeInfo))
    {
      Diagnostics->report(RecipeTitleLoc, DiagnosticKind::Error,
                          LineContext::WithContext)
          << "Recipe '" << RecipeTitle << "' defined more than once!";
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    auto Recipe =
        std::unique_ptr<CheffeRecipeInfo>(new CheffeRecipeInfo(RecipeTitle));

    CurrentRecipe = Recipe.get();
    RecipeInfo[RecipeTitle] = std::move(Recipe);

    Success = parseCommentBlock();
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }

    Success = parseIngredientsList();
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }

    Success = parseCookingTime();
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }

    Success = parseOvenTemperature();
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }

    Success = parseMethod();
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }

    Success = parseServesStatement();
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }
  } while (CurrentToken.isNot(TokenKind::EndOfFile));

  return Success;
}

// Return true if token didn't match, false otherwise.
template <typename T> bool CheffeParser::consumeAndExpectToken(const T &Kind)
{
  getNextToken();
  return expectToken(Kind);
}

// Return true if token didn't match, false otherwise.
template <typename T> bool CheffeParser::expectToken(const T &Kind)
{
  if (CurrentToken.isNot(Kind))
  {
    Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Error,
                        LineContext::WithContext)
        << "Expected " << Kind << ", got " << CurrentToken;
    return true;
  }

  return false;
}

CheffeErrorCode CheffeParser::parseRecipeTitle(std::string &RecipeTitle,
                                               SourceLocation &RecipeTitleLoc)
{
  getNextToken();
  const SourceLocation BeginTitleLoc = CurrentToken.getSourceLoc();

  // Parse the Recipe Title.
  while (CurrentToken.isNotAnyOf(TokenKind::FullStop, TokenKind::EndOfParagraph,
                                 TokenKind::EndOfFile))
  {
    getNextToken();
  }

  if (expectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }
  const std::size_t EndTitlePos = CurrentToken.getSourceLoc().getBegin();

  RecipeTitle = Lexer.getTextSpan(BeginTitleLoc.getBegin(), EndTitlePos);
  RecipeTitleLoc =
      SourceLocation(BeginTitleLoc.getBegin(), EndTitlePos,
                     BeginTitleLoc.getLineNo(), BeginTitleLoc.getColumnNo());

  CHEFFE_DEBUG("RECIPE TITLE:\n\"" << RecipeTitle.c_str() << "\"\n\n");

  if (consumeAndExpectToken(TokenKind::EndOfParagraph))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseCommentBlock()
{
  const std::string Ingredients = "Ingredients.\n";

  // Look ahead to see if we in fact have no comment block in this recipe.
  std::string LookAhead = Lexer.lookAhead(Ingredients.size());
  if (LookAhead == Ingredients)
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  getNextToken();
  const std::size_t CommentsBeginPos = CurrentToken.getSourceLoc().getBegin();
  while (
      CurrentToken.isNotAnyOf(TokenKind::EndOfParagraph, TokenKind::EndOfFile))
  {
    getNextToken();
  }

  if (expectToken(TokenKind::EndOfParagraph))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  std::string Comments = Lexer.getTextSpan(
      CommentsBeginPos, CurrentToken.getSourceLoc().getBegin());
  CHEFFE_DEBUG("COMMENTS:\n\"" << Comments.c_str() << "\"\n\n");

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

bool CheffeParser::isValidMeasure(const std::string &Measure,
                                  MeasureKindTy &Kind)
{
  Kind = MeasureKindTy::Invalid;
  auto MeasureFindResult = std::find(std::begin(ValidDryMeasures),
                                     std::end(ValidDryMeasures), Measure);
  if (MeasureFindResult != std::end(ValidDryMeasures))
  {
    Kind = MeasureKindTy::Dry;
    return true;
  }

  MeasureFindResult = std::find(std::begin(ValidWetMeasures),
                                std::end(ValidWetMeasures), Measure);
  if (MeasureFindResult != std::end(ValidWetMeasures))
  {
    Kind = MeasureKindTy::Wet;
    return true;
  }

  MeasureFindResult = std::find(std::begin(ValidUnspecifiedMeasures),
                                std::end(ValidUnspecifiedMeasures), Measure);
  if (MeasureFindResult != std::end(ValidUnspecifiedMeasures))
  {
    Kind = MeasureKindTy::Unspecified;
    return true;
  }

  return false;
}

CheffeErrorCode CheffeParser::parseIngredientsList()
{
  assert(CurrentRecipe != nullptr &&
         "We should already have set a current recipe by now!");
  // Eat the 'Ingredients' token
  if (consumeAndExpectToken("Ingredients"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::NewLine))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  while (
      CurrentToken.isNotAnyOf(TokenKind::EndOfParagraph, TokenKind::EndOfFile))
  {
    CheffeIngredient Ingredient;
    CheffeErrorCode Success = parseIngredient(Ingredient);
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }
    CHEFFE_DEBUG("INGREDIENT: " << Ingredient << std::endl);

    CurrentRecipe->addIngredient(Ingredient);
  }
  CHEFFE_DEBUG("\n");

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseIngredient(CheffeIngredient &Ingredient)
{
  getNextToken();

  if (CurrentToken.is(TokenKind::Number))
  {
    // Initial value
    Ingredient.HasInitialValue = true;
    Ingredient.InitialValue = CurrentToken.getNumVal();
    getNextToken();
  }

  if (expectToken(TokenKind::Identifier))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  std::string IdentifierString = CurrentToken.getIdentifierString();
  auto MeasureTypeFindResult =
      std::find(std::begin(ValidMeasureTypes), std::end(ValidMeasureTypes),
                IdentifierString);

  const bool IsValidMeasureType =
      MeasureTypeFindResult != std::end(ValidMeasureTypes);

  bool IsIngredientDefinedDry = false;
  if (IsValidMeasureType)
  {
    IsIngredientDefinedDry = true;
    Ingredient.MeasureType = IdentifierString;

    if (consumeAndExpectToken(TokenKind::Identifier))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    IdentifierString = CurrentToken.getIdentifierString();
  }

  MeasureKindTy MeasureKind;
  const bool IsValidMeasure = isValidMeasure(IdentifierString, MeasureKind);

  if (IsValidMeasure)
  {
    if (IsIngredientDefinedDry && MeasureKind == MeasureKindTy::Wet)
    {
      Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Error,
                          LineContext::WithContext)
          << "Wet measure used when dry measure kind specified";
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    Ingredient.IsDry = MeasureKind != MeasureKindTy::Wet;

    Ingredient.Measure = IdentifierString;

    if (consumeAndExpectToken(TokenKind::Identifier))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }
  }

  const std::size_t BeginIngredientNamePos =
      CurrentToken.getSourceLoc().getBegin();
  while (CurrentToken.isNotAnyOf(TokenKind::NewLine, TokenKind::EndOfParagraph,
                                 TokenKind::EndOfFile))
  {
    getNextToken();
  }
  const std::size_t EndIngredientNamePos =
      CurrentToken.getSourceLoc().getBegin();
  Ingredient.Name =
      Lexer.getTextSpan(BeginIngredientNamePos, EndIngredientNamePos);

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

bool CheffeParser::isValidTimeUnit(const std::string &TimeUnit,
                                   TimeUnitKindTy &Kind)
{
  Kind = TimeUnitKindTy::Invalid;
  auto TimeUnitFindResult =
      std::find(std::begin(ValidSingularTimeUnits),
                std::end(ValidSingularTimeUnits), TimeUnit);
  if (TimeUnitFindResult != std::end(ValidSingularTimeUnits))
  {
    Kind = TimeUnitKindTy::Singular;
    return true;
  }

  TimeUnitFindResult = std::find(std::begin(ValidPluralTimeUnits),
                                 std::end(ValidPluralTimeUnits), TimeUnit);
  if (TimeUnitFindResult != std::end(ValidPluralTimeUnits))
  {
    Kind = TimeUnitKindTy::Plural;
    return true;
  }

  return false;
}

CheffeErrorCode CheffeParser::parseCookingTime()
{
  const std::string CookingTimeStr = "Cooking time:";

  // Look ahead to see if we in fact have no comment block in this recipe.
  std::string LookAhead = Lexer.lookAhead(CookingTimeStr.size());
  if (LookAhead != CookingTimeStr)
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  if (consumeAndExpectToken("Cooking"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("time"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::Colon))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::Number))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }
  const int Time = CurrentToken.getNumVal();

  if (consumeAndExpectToken(TokenKind::Identifier))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }
  const std::string TimeUnit = CurrentToken.getIdentifierString();

  TimeUnitKindTy TimeUnitKind;
  const bool IsValidTimeUnit = isValidTimeUnit(TimeUnit, TimeUnitKind);
  if (!IsValidTimeUnit)
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (Time == 1 && TimeUnitKind == TimeUnitKindTy::Plural)
  {
    Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Warning,
                        LineContext::WithContext)
        << "Singular cooking time specified with plural time unit";
  }
  else if (Time != 1 && TimeUnitKind == TimeUnitKindTy::Singular)
  {
    Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Warning,
                        LineContext::WithContext)
        << "Plural cooking time specified with singular time unit";
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::EndOfParagraph))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  CHEFFE_DEBUG("COOKING TIME: " << Time << " " << TimeUnit.c_str() << "\n\n");

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseOvenTemperature()
{
  const std::string OvenTemperature = "Pre-heat oven to";

  // Look ahead to see if we in fact have no comment block in this recipe.
  std::string LookAhead = Lexer.lookAhead(OvenTemperature.size());
  if (LookAhead != OvenTemperature)
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  if (consumeAndExpectToken("Pre"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::Hyphen))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("heat"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("oven"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("to"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::Number))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }
  const int Temperature = CurrentToken.getNumVal();

  if (consumeAndExpectToken("degrees"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("Celcius"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  getNextToken();

  int GasMark = 0;
  bool HasGasMark = false;
  if (CurrentToken.is(TokenKind::OpenBrace))
  {
    if (consumeAndExpectToken("gas"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    if (consumeAndExpectToken("mark"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    if (consumeAndExpectToken(TokenKind::Number))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }
    GasMark = CurrentToken.getNumVal();

    if (consumeAndExpectToken(TokenKind::CloseBrace))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    HasGasMark = true;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::EndOfParagraph))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  CHEFFE_DEBUG("OVEN TEMPERATURE: " << Temperature);
  if (HasGasMark)
  {
    CHEFFE_DEBUG(" (gas mark " << GasMark << ")");
  }
  CHEFFE_DEBUG("\n");

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseMethod()
{
  // Eat the 'Method' token
  if (consumeAndExpectToken("Method"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::NewLine))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  getNextToken();
  CHEFFE_DEBUG("\nMETHOD LIST:\n");
  while (
      CurrentToken.isNotAnyOf(TokenKind::EndOfParagraph, TokenKind::EndOfFile))
  {
    CheffeErrorCode Success = parseMethodStep();
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }
  }
  CHEFFE_DEBUG("\n");

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseMethodStep()
{
  if (expectToken(TokenKind::Identifier))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  const std::string MethodStepKeyword = CurrentToken.getIdentifierString();
  auto FindResult = std::find(std::begin(ValidMethodSteps),
                              std::end(ValidMethodSteps), MethodStepKeyword);

  bool IsKnownVerb = false;
  const bool IsValidMethodStepKeyword =
      FindResult != std::end(ValidMethodSteps);

  if (!IsValidMethodStepKeyword)
  {
    FindResult = std::find(std::begin(ValidVerbKeywords),
                           std::end(ValidVerbKeywords), MethodStepKeyword);

    IsKnownVerb = FindResult != std::end(ValidVerbKeywords);
  }

  if (!IsValidMethodStepKeyword && !IsKnownVerb)
  {
    Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Error,
                        LineContext::WithContext)
        << "Invalid Method Step Keyword: '" << MethodStepKeyword.c_str() << "'";
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (MethodStepKeyword == "Liquify")
  {
    Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Warning,
                        LineContext::WithContext)
        << "'Liquify' keyword is deprecated: use 'Liquefy' instead";
  }

  const std::size_t BeginMethodStepPos = CurrentToken.getSourceLoc().getBegin();
  while (CurrentToken.isNotAnyOf(TokenKind::FullStop, TokenKind::EndOfFile))
  {
    getNextToken();
  }

  if (expectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }
  const std::size_t EndMethodStepPos = CurrentToken.getSourceLoc().getEnd();
  std::string MethodStep =
      Lexer.getTextSpan(BeginMethodStepPos, EndMethodStepPos);
  // Pretty-print the method step - strip out any new lines.
  MethodStep.erase(
      std::remove(std::begin(MethodStep), std::end(MethodStep), '\n'),
      std::end(MethodStep));
  CHEFFE_DEBUG("\"" << MethodStep.c_str() << "\"\n");

  getNextToken();
  while (CurrentToken.is(TokenKind::NewLine))
  {
    getNextToken();
  }
  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseServesStatement()
{
  if (CurrentToken.is(TokenKind::EndOfFile))
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  const std::string ServesStr = "Serves";

  // Look ahead to see if we in fact have no serves statement in this recipe.
  std::string LookAhead = Lexer.lookAhead(ServesStr.size());
  if (LookAhead != ServesStr)
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  if (consumeAndExpectToken(ServesStr))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::Number))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }
  const int ServesNum = CurrentToken.getNumVal();

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  CHEFFE_DEBUG("SERVES: " << ServesNum << "\n\n");

  getNextToken();
  if (CurrentToken.isNotAnyOf(TokenKind::EndOfParagraph, TokenKind::EndOfFile))
  {
    if (expectToken(TokenKind::NewLine))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    getNextToken();
    if (CurrentToken.isNotAnyOf(TokenKind::EndOfParagraph,
                                TokenKind::EndOfFile))
    {
      Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Error,
                          LineContext::WithoutContext)
          << "Invalid Serves Statement";
      return CheffeErrorCode::CHEFFE_ERROR;
    }
  }

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

} // end namespace cheffe
