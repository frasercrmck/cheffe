#include "Parser/CheffeParser.h"
#include "Utils/CheffeDebugUtils.h"

#include <cassert>
#include <array>
#include <algorithm>

#define DEBUG_TYPE "parser"

namespace cheffe
{

Token CheffeParser::getNextToken()
{
  return CurrentToken = Lexer.getToken();
}

void CheffeParser::emitDiagnosticIfIngredientUndefined(
    const std::string &Ingredient, const SourceLocation IngredientLoc)
{
  if (CurrentRecipe->wasIngredientDefined(Ingredient))
  {
    return;
  }
  Diagnostics->report(IngredientLoc, DiagnosticKind::Warning,
                      LineContext::WithContext)
      << "Ingredient '" << Ingredient
      << "' was not defined in the Ingredients paragraph";
}

// Parse an ordinal identifier at the current token, if it's a number. Ordinal
// identifiers are always optional. Note that this can update the current
// token.
CheffeErrorCode
CheffeParser::parsePossibleOrdinalIdentifier(unsigned &MixingBowlNo)
{
  MixingBowlNo = 1;
  if (CurrentToken.isNot(TokenKind::Number))
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  const unsigned Number = CurrentToken.getNumVal();
  if (consumeAndExpectToken(TokenKind::Identifier))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }
  CheffeErrorCode IsValidOrdinal =
      checkOrdinalIdentifier(Number, CurrentToken.getIdentifierString());
  if (IsValidOrdinal != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Warning,
                        LineContext::WithContext)
        << "Incorrect use of ordinal identifier: mismatch between number and "
           "suffix";
  }

  getNextToken();
  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::checkOrdinalIdentifier(const unsigned Number,
                                                     const std::string &Suffix)
{
  if (Suffix.empty())
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  std::string ExpectedSuffix;
  switch (const unsigned Digit = Number % 10)
  {
  default:
    cheffe_unreachable("Invalid digit found in sequence!");
    break;
  case 1:
  case 2:
  case 3:
    if (Number % 100 > 10 && Number % 100 < 14)
    {
      ExpectedSuffix = "th";
      break;
    }

    ExpectedSuffix = std::array<std::string, 3>
    {
      {
        "st", "nd", "rd"
      }
    }
    [Digit - 1];
    break;
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 0:
    ExpectedSuffix = "th";
    break;
  }

  if (Suffix != ExpectedSuffix)
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  return CheffeErrorCode::CHEFFE_SUCCESS;
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
  if (CurrentToken.is(Kind))
  {
    return false;
  }
  Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Error,
                      LineContext::WithContext)
      << "Expected " << Kind << ", got " << CurrentToken;
  return true;
}

CheffeErrorCode CheffeParser::parseRecipeTitle(std::string &RecipeTitle,
                                               SourceLocation &RecipeTitleLoc)
{
  getNextToken();
  const SourceLocation BeginTitleLoc = CurrentToken.getSourceLoc();
  SourceLocation EndTitleLoc = BeginTitleLoc;

  // Parse the Recipe Title.
  while (CurrentToken.isNotAnyOf(TokenKind::FullStop, TokenKind::EndOfParagraph,
                                 TokenKind::EndOfFile))
  {
    EndTitleLoc = CurrentToken.getSourceLoc();
    getNextToken();
  }

  if (expectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (EndTitleLoc.getEnd() == BeginTitleLoc.getBegin())
  {
    Diagnostics->report(BeginTitleLoc, DiagnosticKind::Error,
                        LineContext::WithContext)
        << "Could not find a title";
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  RecipeTitle =
      Lexer.getTextSpan(BeginTitleLoc.getBegin(), EndTitleLoc.getEnd());
  RecipeTitleLoc = SourceLocation(BeginTitleLoc, EndTitleLoc);

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

  struct IgnoreNewLineHelper
  {
    IgnoreNewLineHelper(CheffeParser *Parser) : Parser(Parser)
    {
      Parser->Lexer.setIgnoreNewLines(true);
    }
    ~IgnoreNewLineHelper()
    {
      Parser->Lexer.setIgnoreNewLines(false);
    }

    CheffeParser *Parser;
  } Helper(this);

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

  CheffeErrorCode Success = CheffeErrorCode::CHEFFE_SUCCESS;
  if (MethodStepKeyword == "Take")
  {
    Success = parseTakeMethodStep();
  }
  else if (MethodStepKeyword == "Put")
  {
    Success = parsePutOrFoldMethodStep();
  }
  else if (MethodStepKeyword == "Fold")
  {
    Success = parsePutOrFoldMethodStep();
  }
  else if (MethodStepKeyword == "Add")
  {
    Success = parseArithmeticMethodStep(ArithmeticMethodStep::Add);
  }
  else if (MethodStepKeyword == "Remove")
  {
    Success = parseArithmeticMethodStep(ArithmeticMethodStep::Remove);
  }
  else if (MethodStepKeyword == "Combine")
  {
    Success = parseArithmeticMethodStep(ArithmeticMethodStep::Combine);
  }
  else if (MethodStepKeyword == "Divide")
  {
    Success = parseArithmeticMethodStep(ArithmeticMethodStep::Divide);
  }
  else if (MethodStepKeyword == "Liquefy" || MethodStepKeyword == "Liquify")
  {
    Success = parseLiquifyMethodStep();
  }
  else if (MethodStepKeyword == "Stir")
  {
    Success = parseStirMethodStep();
  }
  else if (MethodStepKeyword == "Mix")
  {
    Success = parseMixMethodStep();
  }
  else
  {
    // Haven't defined a parse function for this method step yet. Consume until
    // the full stop.
    while (CurrentToken.isNotAnyOf(TokenKind::FullStop, TokenKind::EndOfFile))
    {
      getNextToken();
    }
  }

  if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    return Success;
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
  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parsePutOrFoldMethodStep()
{
  getNextToken();

  const SourceLocation BeginIngredientLoc = CurrentToken.getSourceLoc();
  SourceLocation EndIngredientLoc = BeginIngredientLoc;
  while (CurrentToken.isNotAnyOf("into", TokenKind::FullStop,
                                 TokenKind::EndOfFile))
  {
    EndIngredientLoc = CurrentToken.getSourceLoc();
    getNextToken();
  }

  if (expectToken("into"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  const std::string Ingredient = Lexer.getTextSpan(
      BeginIngredientLoc.getBegin(), EndIngredientLoc.getEnd());

  const SourceLocation IngredientLoc(BeginIngredientLoc, EndIngredientLoc);

  emitDiagnosticIfIngredientUndefined(Ingredient, IngredientLoc);

  getNextToken();

  // Not in the spec, but in the "official" examples:
  //   "Put potatoes into the mixing bowl."
  if (CurrentToken.is("the"))
  {
    getNextToken();
  }

  unsigned MixingBowlNo = 1;
  CheffeErrorCode IsValidOrdinal = parsePossibleOrdinalIdentifier(MixingBowlNo);
  if (IsValidOrdinal != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    return IsValidOrdinal;
  }

  if (expectToken("mixing"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("bowl"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

// Parses either an 'Add', 'Remove', 'Combine', or 'Divide' method step:
//   Add ingredient [to [nth] mixing bowl].
//   Remove ingredient [from [nth] mixing bowl].
//   Combine ingredient [into [nth] mixing bowl].
//   Divide ingredient [into [nth] mixing bowl].
// Note: the different prepositions: 'to', 'from', 'into'
CheffeErrorCode
CheffeParser::parseArithmeticMethodStep(const ArithmeticMethodStep Step)
{
  getNextToken();

  if (MethodStepPrepositions.find(Step) == std::end(MethodStepPrepositions))
  {
    cheffe_unreachable("Invalid method step kind");
  }
  const std::string Preposition = MethodStepPrepositions.find(Step)->second;

  if (Step == ArithmeticMethodStep::Add && CurrentToken.is("dry"))
  {
    if (consumeAndExpectToken("ingredients"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }
    getNextToken();
  }
  else
  {
    const SourceLocation BeginIngredientLoc = CurrentToken.getSourceLoc();
    SourceLocation EndIngredientLoc = BeginIngredientLoc;
    while (CurrentToken.isNotAnyOf(Preposition.c_str(), TokenKind::FullStop,
                                   TokenKind::EndOfFile))
    {
      EndIngredientLoc = CurrentToken.getSourceLoc();
      getNextToken();
    }

    const std::string Ingredient = Lexer.getTextSpan(
        BeginIngredientLoc.getBegin(), EndIngredientLoc.getEnd());

    const SourceLocation IngredientLoc(BeginIngredientLoc, EndIngredientLoc);

    emitDiagnosticIfIngredientUndefined(Ingredient, IngredientLoc);
  }

  if (CurrentToken.isNot(Preposition.c_str()))
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  getNextToken();

  // Not in the spec, but in the "official" examples:
  //   "Put potatoes into the mixing bowl."
  if (CurrentToken.is("the"))
  {
    getNextToken();
  }

  unsigned MixingBowlNo = 1;
  CheffeErrorCode IsValidOrdinal = parsePossibleOrdinalIdentifier(MixingBowlNo);
  if (IsValidOrdinal != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    return IsValidOrdinal;
  }

  if (expectToken("mixing"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("bowl"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseTakeMethodStep()
{
  getNextToken();

  const SourceLocation BeginIngredientLoc = CurrentToken.getSourceLoc();
  SourceLocation EndIngredientLoc = BeginIngredientLoc;
  while (CurrentToken.isNotAnyOf("from", TokenKind::FullStop,
                                 TokenKind::EndOfFile))
  {
    EndIngredientLoc = CurrentToken.getSourceLoc();
    getNextToken();
  }

  if (CurrentToken.isNot("from"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  const std::string Ingredient = Lexer.getTextSpan(
      BeginIngredientLoc.getBegin(), EndIngredientLoc.getEnd());

  const SourceLocation IngredientLoc(BeginIngredientLoc, EndIngredientLoc);

  emitDiagnosticIfIngredientUndefined(Ingredient, IngredientLoc);

  if (consumeAndExpectToken("refrigerator"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseLiquifyMethodStep()
{
  getNextToken();

  if (CurrentToken.is("contents"))
  {
    if (consumeAndExpectToken("of"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    if (consumeAndExpectToken("the"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    getNextToken();
    unsigned MixingBowlNo = 1;
    CheffeErrorCode IsValidOrdinal =
        parsePossibleOrdinalIdentifier(MixingBowlNo);
    if (IsValidOrdinal != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return IsValidOrdinal;
    }

    if (expectToken("mixing"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    if (consumeAndExpectToken("bowl"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    if (consumeAndExpectToken(TokenKind::FullStop))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  const SourceLocation BeginIngredientLoc = CurrentToken.getSourceLoc();
  SourceLocation EndIngredientLoc = BeginIngredientLoc;
  while (CurrentToken.isNotAnyOf(TokenKind::FullStop, TokenKind::EndOfFile))
  {
    EndIngredientLoc = CurrentToken.getSourceLoc();
    getNextToken();
  }

  if (expectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  const std::string Ingredient = Lexer.getTextSpan(
      BeginIngredientLoc.getBegin(), EndIngredientLoc.getEnd());

  const SourceLocation IngredientLoc(BeginIngredientLoc, EndIngredientLoc);

  emitDiagnosticIfIngredientUndefined(Ingredient, IngredientLoc);

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

// Parses the "Stir" method step:
//   Stir [the [nth] mixing bowl] for number minutes.
//   Stir ingredient into the [nth] mixing bowl.
CheffeErrorCode CheffeParser::parseStirMethodStep()
{
  getNextToken();

  if (CurrentToken.is("the") || CurrentToken.is("for"))
  {
    if (CurrentToken.is("the"))
    {
      getNextToken();

      unsigned MixingBowlNo = 1;
      CheffeErrorCode IsValidOrdinal =
          parsePossibleOrdinalIdentifier(MixingBowlNo);
      if (IsValidOrdinal != CheffeErrorCode::CHEFFE_SUCCESS)
      {
        return IsValidOrdinal;
      }

      if (expectToken("mixing"))
      {
        return CheffeErrorCode::CHEFFE_ERROR;
      }

      if (consumeAndExpectToken("bowl"))
      {
        return CheffeErrorCode::CHEFFE_ERROR;
      }

      if (consumeAndExpectToken("for"))
      {
        return CheffeErrorCode::CHEFFE_ERROR;
      }
    }

    if (consumeAndExpectToken(TokenKind::Number))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    if (consumeAndExpectToken("minutes"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    if (consumeAndExpectToken(TokenKind::FullStop))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  const SourceLocation BeginIngredientLoc = CurrentToken.getSourceLoc();
  SourceLocation EndIngredientLoc = BeginIngredientLoc;
  while (CurrentToken.isNotAnyOf("into", TokenKind::FullStop,
                                 TokenKind::EndOfFile))
  {
    EndIngredientLoc = CurrentToken.getSourceLoc();
    getNextToken();
  }

  const std::string Ingredient = Lexer.getTextSpan(
      BeginIngredientLoc.getBegin(), EndIngredientLoc.getEnd());

  const SourceLocation IngredientLoc(BeginIngredientLoc, EndIngredientLoc);

  emitDiagnosticIfIngredientUndefined(Ingredient, IngredientLoc);

  if (expectToken("into"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("the"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  getNextToken();

  unsigned MixingBowlNo = 1;
  CheffeErrorCode IsValidOrdinal = parsePossibleOrdinalIdentifier(MixingBowlNo);
  if (IsValidOrdinal != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    return IsValidOrdinal;
  }

  if (expectToken("mixing"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("bowl"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

// Parses the "Mix" method step:
//   Mix [the [nth] mixing bowl] well.
CheffeErrorCode CheffeParser::parseMixMethodStep()
{
  getNextToken();

  if (CurrentToken.isNot("well"))
  {
    if (expectToken("the"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    getNextToken();
    unsigned MixingBowlNo = 1;
    CheffeErrorCode IsValidOrdinal =
        parsePossibleOrdinalIdentifier(MixingBowlNo);
    if (IsValidOrdinal != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return IsValidOrdinal;
    }

    if (expectToken("mixing"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    if (consumeAndExpectToken("bowl"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }
    getNextToken();
  }

  if (expectToken("well"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
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
