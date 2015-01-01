#include "Parser/CheffeParser.h"
#include "IR/CheffeMethodStep.h"
#include "IR/CheffeIngredient.h"
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

// Emits a diagnostic and returns true if the ingredient is undefined
bool CheffeParser::getIngredientInfo(
    const std::string &IngredientName, const SourceLocation IngredientLoc,
    std::shared_ptr<CheffeIngredient> &Ingredient)
{
  Ingredient = CurrentRecipe->getIngredient(IngredientName);
  if (Ingredient != nullptr)
  {
    return false;
  }
  Diagnostics->report(IngredientLoc, DiagnosticKind::Error,
                      LineContext::WithContext)
      << "Ingredient '" << IngredientName
      << "' was not defined in the Ingredients paragraph";
  return true;
}

std::unique_ptr<CheffeProgramInfo> CheffeParser::takeProgramInfo()
{
  return std::move(ProgramInfo);
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
  auto NumberSourceLoc = CurrentToken.getSourceLoc();
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

  if (Number == 0)
  {
    Diagnostics->report(NumberSourceLoc, DiagnosticKind::Error,
                        LineContext::WithContext)
        << "Cannot use 0 as an ordinal identifier";
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  getNextToken();
  MixingBowlNo = Number;
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

  // Grab the first token
  getNextToken();

  do
  {
    std::string RecipeTitle;
    SourceLocation RecipeTitleLoc;
    Success = parseRecipeTitle(RecipeTitle, RecipeTitleLoc);
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }

    if (!ProgramInfo)
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    if (ProgramInfo->getRecipe(RecipeTitle))
    {
      Diagnostics->report(RecipeTitleLoc, DiagnosticKind::Error,
                          LineContext::WithContext)
          << "Recipe '" << RecipeTitle << "' defined more than once!";
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    ProgramInfo->setEntryPointRecipeTitleIfNone(RecipeTitle);

    auto Recipe = std::make_shared<CheffeRecipeInfo>(RecipeTitle);

    CurrentRecipe = Recipe;
    ProgramInfo->addRecipe(RecipeTitle, Recipe);

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

    while (CurrentToken.isBlankLine())
    {
      getNextToken();
    }

    CHEFFE_DEBUG(dbgs() << std::endl; RecipeScopeInfo.dumpInfo(dbgs()));

    if (!RecipeScopeInfo.empty())
    {
      Diagnostics->report(SourceLocation(), DiagnosticKind::Error,
                          LineContext::WithoutContext)
          << "Mismatched scopes on function exit";
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    Success = RecipeScopeInfo.fixupScopeMethodSteps(
        CurrentRecipe->getMethodStepList());

    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      Diagnostics->report(SourceLocation(), DiagnosticKind::Error,
                          LineContext::WithoutContext)
          << "Could not fixup nest instructions";
      return Success;
    }

    RecipeScopeInfo.clearInfo();

    // clang-format off
    CHEFFE_DEBUG(
      dbgs() << std::endl << "METHOD LIST:" << std::endl;
      for (auto &MethodStep : CurrentRecipe->getMethodStepList())
      {
        dbgs() << "\t" << *MethodStep;
      }
      dbgs() << std::endl;
    );
    // clang-format on
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
  while (CurrentToken.isBlankLine())
  {
    getNextToken();
  }

  const SourceLocation BeginTitleLoc = CurrentToken.getSourceLoc();
  SourceLocation EndTitleLoc = BeginTitleLoc;

  if (expectToken(TokenKind::Identifier))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

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

  // clang-format off
  CHEFFE_DEBUG(
    dbgs() << "RECIPE TITLE:" << std::endl << "\"" << RecipeTitle.c_str()
           << "\"" << std::endl << std::endl
  );
  // clang-format on

  if (consumeAndExpectToken(TokenKind::EndOfParagraph))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseCommentBlock()
{
  while (CurrentToken.isBlankLine())
  {
    getNextToken();
  }

  // Check to see whether we have a comment block or not
  if (CurrentToken.is("Ingredients"))
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

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
  CHEFFE_DEBUG(dbgs() << "COMMENTS:" << std::endl << "\"" << Comments.c_str()
                      << "\"" << std::endl << std::endl);

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
  while (CurrentToken.isBlankLine())
  {
    getNextToken();
  }

  assert(CurrentRecipe != nullptr &&
         "We should already have set a current recipe by now!");

  // Eat the 'Ingredients' token
  if (expectToken("Ingredients"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.is(TokenKind::EndOfParagraph))
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  if (expectToken(TokenKind::NewLine))
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
    CHEFFE_DEBUG(dbgs() << "INGREDIENT: " << Ingredient << std::endl);

    CurrentRecipe->addIngredientDefinition(Ingredient);
  }
  CHEFFE_DEBUG(dbgs() << std::endl);

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseIngredient(CheffeIngredient &Ingredient)
{
  getNextToken();
  const SourceLocation BeginIngredientDefLoc = CurrentToken.getSourceLoc();

  if (CurrentToken.is(TokenKind::Number))
  {
    // Value
    Ingredient.HasValue = true;
    Ingredient.Value = CurrentToken.getNumVal();
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

  if (CurrentToken.is("the"))
  {
    Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Error,
                        LineContext::WithContext)
        << "Defining an ingredient beginning with 'the' will be ambiguous";
    return CheffeErrorCode::CHEFFE_ERROR;
  }
  const std::size_t BeginIngredientNamePos =
      CurrentToken.getSourceLoc().getBegin();
  SourceLocation EndIngredientDefLoc = CurrentToken.getSourceLoc();
  while (CurrentToken.isNotAnyOf(TokenKind::NewLine, TokenKind::EndOfParagraph,
                                 TokenKind::EndOfFile))
  {
    EndIngredientDefLoc = CurrentToken.getSourceLoc();
    getNextToken();
  }
  const std::size_t EndIngredientNamePos =
      CurrentToken.getSourceLoc().getBegin();
  Ingredient.Name =
      Lexer.getTextSpan(BeginIngredientNamePos, EndIngredientNamePos);

  Ingredient.DefLoc =
      SourceLocation(BeginIngredientDefLoc, EndIngredientDefLoc);

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
  while (CurrentToken.isBlankLine())
  {
    getNextToken();
  }

  // Check to see if we in fact have no cooking time block in this recipe.
  if (CurrentToken.isNot("Cooking"))
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
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
  const long long Time = CurrentToken.getNumVal();

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

  // clang-format off
  CHEFFE_DEBUG(
    dbgs() << "COOKING TIME: " << Time << " " << TimeUnit.c_str() << std::endl
           << std::endl;
  );
  // clang-format on

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseOvenTemperature()
{
  while (CurrentToken.isBlankLine())
  {
    getNextToken();
  }

  // Check to see if we in fact have no oven temperature block in this recipe.
  if (CurrentToken.isNot("Pre"))
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
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
  const long long Temperature = CurrentToken.getNumVal();

  if (consumeAndExpectToken("degrees"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("Celsius"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  getNextToken();

  long long GasMark = 0;
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

    getNextToken();
  }

  if (expectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::EndOfParagraph))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  // clang-format off
  CHEFFE_DEBUG(
    dbgs() << "OVEN TEMPERATURE: " << Temperature;
    if (HasGasMark)
    {
      dbgs() << " (gas mark " << GasMark << ")";
    }
    dbgs() << std::endl;
  );
  // clang-format on

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseMethod()
{
  // Eat any new lines or end-of-paragraphs
  while (CurrentToken.isBlankLine())
  {
    getNextToken();
  }

  if (expectToken("Method"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.is(TokenKind::EndOfParagraph))
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  if (expectToken(TokenKind::NewLine))
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
  while (
      CurrentToken.isNotAnyOf(TokenKind::EndOfParagraph, TokenKind::EndOfFile))
  {
    CheffeErrorCode Success = parseMethodStep();
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }
  }

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
    auto VerbFindResult =
        std::find_if(std::begin(ValidVerbKeywords), std::end(ValidVerbKeywords),
                     [&MethodStepKeyword](const StringPair &Pair)
                     {
          return Pair.first == MethodStepKeyword;
        });

    IsKnownVerb = VerbFindResult != std::end(ValidVerbKeywords);
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

  SourceLocation BeginMethodStepLoc = CurrentToken.getSourceLoc();

  CheffeErrorCode Success = CheffeErrorCode::CHEFFE_SUCCESS;
  if (MethodStepKeyword == "Take")
  {
    Success = parseTakeMethodStep();
  }
  else if (MethodStepKeyword == "Put")
  {
    Success = parsePutOrFoldMethodStep(MethodStepKind::Put);
  }
  else if (MethodStepKeyword == "Fold")
  {
    Success = parsePutOrFoldMethodStep(MethodStepKind::Fold);
  }
  else if (MethodStepKeyword == "Add")
  {
    Success = parseArithmeticMethodStep(MethodStepKind::Add);
  }
  else if (MethodStepKeyword == "Remove")
  {
    Success = parseArithmeticMethodStep(MethodStepKind::Remove);
  }
  else if (MethodStepKeyword == "Combine")
  {
    Success = parseArithmeticMethodStep(MethodStepKind::Combine);
  }
  else if (MethodStepKeyword == "Divide")
  {
    Success = parseArithmeticMethodStep(MethodStepKind::Divide);
  }
  else if (MethodStepKeyword == "Liquefy" || MethodStepKeyword == "Liquify")
  {
    Success = parseLiquefyMethodStep();
  }
  else if (MethodStepKeyword == "Stir")
  {
    Success = parseStirMethodStep();
  }
  else if (MethodStepKeyword == "Mix")
  {
    Success = parseMixMethodStep();
  }
  else if (MethodStepKeyword == "Clean")
  {
    Success = parseCleanMethodStep();
  }
  else if (MethodStepKeyword == "Pour")
  {
    Success = parsePourMethodStep();
  }
  else if (IsKnownVerb)
  {
    Success = parseVerbMethodStep();
  }
  else if (MethodStepKeyword == "Set")
  {
    Success = parseSetAsideMethodStep();
  }
  else if (MethodStepKeyword == "Serve")
  {
    Success = parseServeMethodStep();
  }
  else if (MethodStepKeyword == "Refrigerate")
  {
    Success = parseRefrigerateMethodStep();
  }
  else
  {
    cheffe_unreachable("Invalid method step keyword!");
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    return Success;
  }

  // Tack on the source location information for this method step onto the
  // last-added method step that the recipe contains. Bit of a hack but it
  // keeps the parsing interfaces cleaner.
  if (CurrentRecipe->getLastMethodStep())
  {
    const SourceLocation EndMethodStepLoc = CurrentToken.getSourceLoc();
    CurrentRecipe->getLastMethodStep()->setSourceLoc(
        SourceLocation(BeginMethodStepLoc, EndMethodStepLoc));
  }

  if (expectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  getNextToken();
  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode
CheffeParser::parsePutOrFoldMethodStep(const MethodStepKind Step)
{
  getNextToken();

  if (Step != MethodStepKind::Put && Step != MethodStepKind::Fold)
  {
    cheffe_unreachable("invalid method step kind");
  }

  // Not in the spec, but in some examples:
  //   "Put the potatoes into mixing bowl."
  if (CurrentToken.is("the"))
  {
    getNextToken();
  }

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

  const std::string IngredientName = Lexer.getTextSpan(
      BeginIngredientLoc.getBegin(), EndIngredientLoc.getEnd());

  const SourceLocation IngredientLoc(BeginIngredientLoc, EndIngredientLoc);

  std::shared_ptr<CheffeIngredient> Ingredient = nullptr;
  if (getIngredientInfo(IngredientName, IngredientLoc, Ingredient))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
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

  auto MethodStep = CurrentRecipe->addNewMethodStep(Step);
  MethodStep->addIngredient(Ingredient, IngredientLoc);
  MethodStep->addMixingBowl(MixingBowlNo);

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

// Parses either an 'Add', 'Remove', 'Combine', or 'Divide' method step:
//   Add ingredient [to [nth] mixing bowl].
//   Remove ingredient [from [nth] mixing bowl].
//   Combine ingredient [into [nth] mixing bowl].
//   Divide ingredient [into [nth] mixing bowl].
//   Add dry ingredients [to [nth] mixing bowl].
// Note: the different prepositions: 'to', 'from', 'into'
CheffeErrorCode
CheffeParser::parseArithmeticMethodStep(const MethodStepKind Step)
{
  getNextToken();

  if (MethodStepPrepositions.find(Step) == std::end(MethodStepPrepositions))
  {
    cheffe_unreachable("Invalid method step kind");
  }
  const std::string Preposition = MethodStepPrepositions.find(Step)->second;

  std::shared_ptr<CheffeMethodStep> MethodStep = nullptr;

  // Not in the spec, but in some examples:
  //   "Add the potatoes [to [nth] mixing bowl]."
  //   "Add the dry ingredients [to [nth] mixing bowl]."
  if (CurrentToken.is("the"))
  {
    getNextToken();
  }

  if (Step == MethodStepKind::Add && CurrentToken.is("dry"))
  {
    if (consumeAndExpectToken("ingredients"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }
    MethodStep = CurrentRecipe->addNewMethodStep(MethodStepKind::AddDry);
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

    const std::string IngredientName = Lexer.getTextSpan(
        BeginIngredientLoc.getBegin(), EndIngredientLoc.getEnd());

    const SourceLocation IngredientLoc(BeginIngredientLoc, EndIngredientLoc);

    std::shared_ptr<CheffeIngredient> Ingredient = nullptr;
    if (getIngredientInfo(IngredientName, IngredientLoc, Ingredient))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    MethodStep = CurrentRecipe->addNewMethodStep(Step);
    MethodStep->addIngredient(Ingredient, IngredientLoc);
  }

  unsigned MixingBowlNo = 1;
  if (CurrentToken.isNot(Preposition.c_str()))
  {
    MethodStep->addMixingBowl(MixingBowlNo);
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  getNextToken();

  assert(MethodStep != nullptr && "New method step is still null");

  // Not in the spec, but in the "official" examples:
  //   "Put potatoes into the mixing bowl."
  if (CurrentToken.is("the"))
  {
    getNextToken();
  }

  CheffeErrorCode IsValidOrdinal = parsePossibleOrdinalIdentifier(MixingBowlNo);
  if (IsValidOrdinal != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    return IsValidOrdinal;
  }

  MethodStep->addMixingBowl(MixingBowlNo);

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

  // Not in the spec, but in the "official" examples:
  //   "Take the potatoes from refrigerator."
  if (CurrentToken.is("the"))
  {
    getNextToken();
  }

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

  const std::string IngredientName = Lexer.getTextSpan(
      BeginIngredientLoc.getBegin(), EndIngredientLoc.getEnd());

  const SourceLocation IngredientLoc(BeginIngredientLoc, EndIngredientLoc);

  std::shared_ptr<CheffeIngredient> Ingredient = nullptr;
  if (getIngredientInfo(IngredientName, IngredientLoc, Ingredient))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  getNextToken();
  // Not in the spec, but in the "official" examples:
  //   "Take potatoes from the refrigerator."
  if (CurrentToken.is("the"))
  {
    getNextToken();
  }

  if (expectToken("refrigerator"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  auto MethodStep = CurrentRecipe->addNewMethodStep(MethodStepKind::Take);
  MethodStep->addIngredient(Ingredient, IngredientLoc);

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseLiquefyMethodStep()
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

    auto MethodStep =
        CurrentRecipe->addNewMethodStep(MethodStepKind::LiquefyBowl);
    MethodStep->addMixingBowl(MixingBowlNo);

    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  // Not in the spec, but in the "official" examples:
  //   "Liquefy the potatoes."
  if (CurrentToken.is("the"))
  {
    getNextToken();
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

  const std::string IngredientName = Lexer.getTextSpan(
      BeginIngredientLoc.getBegin(), EndIngredientLoc.getEnd());

  const SourceLocation IngredientLoc(BeginIngredientLoc, EndIngredientLoc);

  std::shared_ptr<CheffeIngredient> Ingredient = nullptr;
  if (getIngredientInfo(IngredientName, IngredientLoc, Ingredient))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  auto MethodStep =
      CurrentRecipe->addNewMethodStep(MethodStepKind::LiquefyIngredient);
  MethodStep->addIngredient(Ingredient, IngredientLoc);

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
    unsigned MixingBowlNo = 1;
    if (CurrentToken.is("the"))
    {
      getNextToken();

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

    const long long NumberOfMinutes = CurrentToken.getNumVal();

    getNextToken();
    if (CurrentToken.isNotAnyOf("minute", "minutes"))
    {
      Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Error,
                          LineContext::WithContext)
          << "Expected 'minute' or 'minutes', got " << CurrentToken;
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    if (CurrentToken.is("minute") && NumberOfMinutes != 1)
    {
      Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Warning,
                          LineContext::WithContext)
          << "Plural time period used with 'minute'";
    }
    else if (CurrentToken.is("minutes") && NumberOfMinutes == 1)
    {
      Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Warning,
                          LineContext::WithContext)
          << "Singular time period used with 'minutes'";
    }

    if (consumeAndExpectToken(TokenKind::FullStop))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    auto MethodStep = CurrentRecipe->addNewMethodStep(MethodStepKind::StirBowl);
    MethodStep->addMixingBowl(MixingBowlNo);
    MethodStep->addNumber(NumberOfMinutes);

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

  const std::string IngredientName = Lexer.getTextSpan(
      BeginIngredientLoc.getBegin(), EndIngredientLoc.getEnd());

  const SourceLocation IngredientLoc(BeginIngredientLoc, EndIngredientLoc);

  std::shared_ptr<CheffeIngredient> Ingredient = nullptr;
  if (getIngredientInfo(IngredientName, IngredientLoc, Ingredient))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

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

  auto MethodStep =
      CurrentRecipe->addNewMethodStep(MethodStepKind::StirIngredient);
  MethodStep->addIngredient(Ingredient, IngredientLoc);
  MethodStep->addMixingBowl(MixingBowlNo);

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

// Parses the "Mix" method step:
//   Mix [the [nth] mixing bowl] well.
CheffeErrorCode CheffeParser::parseMixMethodStep()
{
  getNextToken();

  unsigned MixingBowlNo = 1;
  if (CurrentToken.isNot("well"))
  {
    if (expectToken("the"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    getNextToken();
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

  auto MethodStep = CurrentRecipe->addNewMethodStep(MethodStepKind::Mix);
  MethodStep->addMixingBowl(MixingBowlNo);

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

// Parses the "Clean" method step:
//   Clean [nth] mixing bowl.
CheffeErrorCode CheffeParser::parseCleanMethodStep()
{
  getNextToken();

  // Not in the spec, but in some examples:
  //   "Clean the [nth] mixing bowl.
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
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  if (consumeAndExpectToken("bowl"))
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  auto MethodStep = CurrentRecipe->addNewMethodStep(MethodStepKind::Clean);
  MethodStep->addMixingBowl(MixingBowlNo);

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

// Parses the "Pour" method step:
// Pour contents of the [nth] mixing bowl into the [pth] baking dish.
CheffeErrorCode CheffeParser::parsePourMethodStep()
{
  getNextToken();

  // Not in the spec, but in some examples:
  //   "Pour the contents of the [nth] mixing bowl into the [pth] baking dish.
  if (CurrentToken.is("the"))
  {
    getNextToken();
  }

  if (expectToken("contents"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

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
  const CheffeErrorCode IsValidOrdinalMixingBowl =
      parsePossibleOrdinalIdentifier(MixingBowlNo);
  if (IsValidOrdinalMixingBowl != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    return IsValidOrdinalMixingBowl;
  }

  if (expectToken("mixing"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("bowl"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("into"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("the"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  getNextToken();

  unsigned BakingDishNo = 1;
  const CheffeErrorCode IsValidOrdinalBakingDish =
      parsePossibleOrdinalIdentifier(BakingDishNo);
  if (IsValidOrdinalBakingDish != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    return IsValidOrdinalBakingDish;
  }

  if (expectToken("baking"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken("dish"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  auto MethodStep = CurrentRecipe->addNewMethodStep(MethodStepKind::Pour);
  MethodStep->addMixingBowl(MixingBowlNo);
  MethodStep->addBakingDish(BakingDishNo);

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

static bool AreLowerCasedStringsEqual(const std::string &LHS,
                                      const std::string &RHS)
{
  if (LHS.size() != RHS.size())
  {
    return false;
  }
  auto LHSIt = std::begin(LHS);
  auto RHSIt = std::begin(RHS);
  while (LHSIt++ != std::end(LHS) && RHSIt++ != std::end(RHS))
  {
    if (std::tolower(*LHSIt) != std::tolower(*RHSIt))
    {
      return false;
    }
  }
  return true;
}

// Parsers the "Verb" method steps. One of the following two:
//   Verb the ingredient.
//   Verb [the ingredient] until verbed.
CheffeErrorCode CheffeParser::parseVerbMethodStep()
{
  const std::string Verb = CurrentToken.getIdentifierString();
  getNextToken();

  SourceLocation IngredientLoc;
  std::shared_ptr<CheffeIngredient> Ingredient = nullptr;

  if (CurrentToken.isNot("until"))
  {
    // Not in the spec, but in the "official" examples:
    //   "Cook potatoes."
    //   "Cook potatoes until cooked."
    if (CurrentToken.is("the"))
    {
      getNextToken();
    }
    const SourceLocation BeginIngredientLoc = CurrentToken.getSourceLoc();
    SourceLocation EndIngredientLoc = BeginIngredientLoc;
    while (CurrentToken.isNotAnyOf("until", TokenKind::FullStop,
                                   TokenKind::EndOfFile))
    {
      EndIngredientLoc = CurrentToken.getSourceLoc();
      getNextToken();
    }

    const std::string IngredientName = Lexer.getTextSpan(
        BeginIngredientLoc.getBegin(), EndIngredientLoc.getEnd());

    IngredientLoc = SourceLocation(BeginIngredientLoc, EndIngredientLoc);

    if (getIngredientInfo(IngredientName, IngredientLoc, Ingredient))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }
  }

  if (CurrentToken.isNot("until"))
  {
    auto MethodStep = CurrentRecipe->addNewMethodStep(MethodStepKind::Verb);
    if (!Ingredient)
    {
      Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Error,
                          LineContext::WithContext)
          << "Verb method steps must specify an ingredient";
      return CheffeErrorCode::CHEFFE_ERROR;
    }
    MethodStep->addIngredient(Ingredient, IngredientLoc);
    RecipeScopeInfo.addScope(Verb, MethodStep);
  }
  else
  {
    if (consumeAndExpectToken(TokenKind::Identifier))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    const std::string UntilVerb = CurrentToken.getIdentifierString();

    std::shared_ptr<CheffeScope> Scope = nullptr;
    if (RecipeScopeInfo.popScope(Scope))
    {
      Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Error,
                          LineContext::WithContext)
          << "Mismatched scope: empty scope stack";
      return CheffeErrorCode::CHEFFE_ERROR;
    }
    assert(Scope && "Scope information shall not be nullptr");

    const std::string FromVerb = Scope->BeginVerb;

    // clang-format off
    CHEFFE_DEBUG(
      dbgs() << "Searching for a matched pair between " << FromVerb
             << " & " << UntilVerb << std::endl
    );
    // clang-format on

    auto VerbFindResult =
        std::find_if(std::begin(ValidVerbKeywords), std::end(ValidVerbKeywords),
                     [&FromVerb](const StringPair &Pair)
                     {
          return Pair.first == FromVerb;
        });
    assert(VerbFindResult != std::end(ValidVerbKeywords));

    if (!AreLowerCasedStringsEqual(VerbFindResult->second, UntilVerb))
    {
      Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Error,
                          LineContext::WithContext)
          << "Mismatched verbs: trying to match '" << FromVerb << "' with '"
          << UntilVerb << "'";
      return CheffeErrorCode::CHEFFE_ERROR;
    }
    getNextToken();
    auto MethodStep =
        CurrentRecipe->addNewMethodStep(MethodStepKind::UntilVerbed);
    MethodStep->addIngredient(Ingredient, IngredientLoc);

    // Add the Ingredient that the matching Verb method step uses to this
    // UntilVerbed's method step.
    assert(Scope->BeginScope &&
           "MethodStep at beginning of loop shall not be nullptr");
    MethodStep->addIngredient(std::static_pointer_cast<IngredientOp>(
        Scope->BeginScope->getOperand(0)));

    // Register this UntilVerbed method step as the end of the current nest.
    Scope->EndScope = MethodStep;
  }

  if (expectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

// Parses the "Set Aside" method step:
// Set aside.
CheffeErrorCode CheffeParser::parseSetAsideMethodStep()
{
  if (consumeAndExpectToken("aside"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  auto MethodStep = CurrentRecipe->addNewMethodStep(MethodStepKind::SetAside);

  if (RecipeScopeInfo.addBreak(MethodStep))
  {
    Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Error,
                        LineContext::WithContext)
        << "'Set Aside' found outwith any loop nest";
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

// Parses the "Serve" method step:
// Serve with auxiliary-recipe.
CheffeErrorCode CheffeParser::parseServeMethodStep()
{
  if (consumeAndExpectToken("with"))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  if (consumeAndExpectToken(TokenKind::Identifier))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  const SourceLocation BeginRecipeLoc = CurrentToken.getSourceLoc();
  SourceLocation EndRecipeLoc = BeginRecipeLoc;
  while (CurrentToken.isNotAnyOf(TokenKind::FullStop, TokenKind::EndOfFile))
  {
    EndRecipeLoc = CurrentToken.getSourceLoc();
    getNextToken();
  }

  const std::string Recipe =
      Lexer.getTextSpan(BeginRecipeLoc.getBegin(), EndRecipeLoc.getEnd());

  if (expectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  auto MethodStep = CurrentRecipe->addNewMethodStep(MethodStepKind::Serve);
  MethodStep->addRecipe(Recipe);

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

// Parses the "Refrigerate" method step:
// Refrigerate [for number hour[s]].
CheffeErrorCode CheffeParser::parseRefrigerateMethodStep()
{
  getNextToken();

  unsigned NumberOfHours = 0u;
  if (CurrentToken.isNot(TokenKind::FullStop))
  {
    if (expectToken("for"))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    if (consumeAndExpectToken(TokenKind::Number))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    NumberOfHours = CurrentToken.getNumVal();

    getNextToken();
    if (expectToken(TokenKind::Identifier))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }
    if (CurrentToken.isNotAnyOf("hour", "hours"))
    {
      Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Error,
                          LineContext::WithContext)
          << "Expected 'hour' or 'hours', got " << CurrentToken;
      return CheffeErrorCode::CHEFFE_ERROR;
    }
    if (CurrentToken.is("hour") && NumberOfHours != 1)
    {
      Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Warning,
                          LineContext::WithContext)
          << "Plural time period used with 'hour'";
    }
    else if (CurrentToken.is("hours") && NumberOfHours == 1)
    {
      Diagnostics->report(CurrentToken.getSourceLoc(), DiagnosticKind::Warning,
                          LineContext::WithContext)
          << "Singular time period used with 'hours'";
    }

    getNextToken();
  }

  if (expectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  auto MethodStep =
      CurrentRecipe->addNewMethodStep(MethodStepKind::Refrigerate);
  MethodStep->addNumber(NumberOfHours);

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseServesStatement()
{
  while (CurrentToken.isBlankLine())
  {
    getNextToken();
  }

  if (CurrentToken.is(TokenKind::EndOfFile))
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  // Check to see if we in fact have no serves statement in this recipe.
  if (CurrentToken.isNot("Serves"))
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  if (consumeAndExpectToken(TokenKind::Number))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }
  const long long ServesNo = CurrentToken.getNumVal();
  const SourceLocation NumberLoc = CurrentToken.getSourceLoc();

  if (consumeAndExpectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  CHEFFE_DEBUG(dbgs() << "SERVES: " << ServesNo << std::endl << std::endl);

  if (ServesNo < std::numeric_limits<unsigned>::min() ||
      ServesNo > std::numeric_limits<unsigned>::max())
  {
    Diagnostics->report(NumberLoc, DiagnosticKind::Error,
                        LineContext::WithContext)
        << "Serves No is outwith bounds of unsigned integer";
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  CurrentRecipe->setServesNo((unsigned)ServesNo);

  // Consume the full stop
  getNextToken();

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

} // end namespace cheffe
