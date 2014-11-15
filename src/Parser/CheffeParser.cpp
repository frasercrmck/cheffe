#include "Parser/CheffeParser.h"
#include "Utils/CheffeDebugUtils.h"

#include <algorithm>

#define DEBUG_TYPE "parser"

namespace cheffe
{

std::ostream &operator<<(std::ostream &stream, const IngredientInfoTy &Info)
{
  if (Info.HasInitialValue)
  {
    stream << Info.InitialValue;
  }
  else
  {
    stream << "<none>";
  }
  stream << " - ";
  stream << (Info.IsDry ? "dry" : "wet");
  stream << " - ";
  stream << "\"" << Info.Name << "\"";
  return stream;
}

Token CheffeParser::getNextToken()
{
  return CurrentToken = Lexer.getToken();
}

CheffeErrorCode CheffeParser::parseRecipe()
{
  CheffeErrorCode Success = CheffeErrorCode::CHEFFE_SUCCESS;

  do
  {
    Success = parseRecipeTitle();
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }

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

    Success = parseMethodList();
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
    Diagnostic.report(CurrentToken.getLineNumber(),
                      CurrentToken.getColumnNumber())
        << "Expected " << Kind << ", got " << CurrentToken << std::endl;

    Diagnostic.printLine(CurrentToken.getLineNumber(), CurrentToken.getBegin(),
                         CurrentToken.getEnd());
    return true;
  }

  return false;
}

CheffeErrorCode CheffeParser::parseRecipeTitle()
{
  getNextToken();
  const std::size_t BeginTitlePos = CurrentToken.getBegin();

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
  const std::size_t EndTitlePos = CurrentToken.getBegin();

  const std::string RecipeTitle = Lexer.getTextSpan(BeginTitlePos, EndTitlePos);
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
  if (!LookAhead.compare(Ingredients))
  {
    return CheffeErrorCode::CHEFFE_SUCCESS;
  }

  getNextToken();
  const std::size_t CommentsBeginPos = CurrentToken.getBegin();
  while (
      CurrentToken.isNotAnyOf(TokenKind::EndOfParagraph, TokenKind::EndOfFile))
  {
    getNextToken();
  }

  if (expectToken(TokenKind::EndOfParagraph))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  std::string Comments =
      Lexer.getTextSpan(CommentsBeginPos, CurrentToken.getBegin());
  CHEFFE_DEBUG("COMMENTS:\n\"" << Comments.c_str() << "\"\n\n");

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

std::tuple<bool, MeasureKindTy>
CheffeParser::isValidMeasure(const std::string &Measure)
{
  auto MeasureFindResult = std::find(std::begin(ValidDryMeasures),
                                     std::end(ValidDryMeasures), Measure);
  if (MeasureFindResult != std::end(ValidDryMeasures))
  {
    return std::make_tuple(true, MeasureKindTy::Dry);
  }

  MeasureFindResult = std::find(std::begin(ValidWetMeasures),
                                std::end(ValidWetMeasures), Measure);
  if (MeasureFindResult != std::end(ValidWetMeasures))
  {
    return std::make_tuple(true, MeasureKindTy::Wet);
  }

  MeasureFindResult = std::find(std::begin(ValidUnspecifiedMeasures),
                                std::end(ValidUnspecifiedMeasures), Measure);
  if (MeasureFindResult != std::end(ValidUnspecifiedMeasures))
  {
    return std::make_tuple(true, MeasureKindTy::Unspecified);
  }

  return std::make_tuple(false, MeasureKindTy::Invalid);
}

CheffeErrorCode CheffeParser::parseIngredientsList()
{
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
    IngredientInfoTy IngredientInfo;
    CheffeErrorCode Success = parseIngredient(IngredientInfo);
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }
    CHEFFE_DEBUG("INGREDIENT: " << IngredientInfo << std::endl);
  }
  CHEFFE_DEBUG("\n");

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseIngredient(IngredientInfoTy &IngredientInfo)
{
  getNextToken();

  if (CurrentToken.is(TokenKind::Number))
  {
    // Initial value
    IngredientInfo.HasInitialValue = true;
    IngredientInfo.InitialValue = CurrentToken.getNumVal();
    getNextToken();

    if (CurrentToken.isAnyOf(TokenKind::NewLine, TokenKind::EndOfParagraph,
                             TokenKind::EndOfFile))
    {
      Diagnostic.report(CurrentToken.getLineNumber(),
                        CurrentToken.getColumnNumber())
          << "Unexpected " << CurrentToken.getKind() << std::endl;

      Diagnostic.printLine(CurrentToken.getLineNumber(),
                           CurrentToken.getBegin(), CurrentToken.getEnd());
      return CheffeErrorCode::CHEFFE_ERROR;
    }
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
    IngredientInfo.MeasureType = IdentifierString;

    if (consumeAndExpectToken(TokenKind::Identifier))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    IdentifierString = CurrentToken.getIdentifierString();
  }

  bool IsValidMeasure;
  MeasureKindTy MeasureKind;
  std::tie(IsValidMeasure, MeasureKind) = isValidMeasure(IdentifierString);

  if (IsValidMeasure)
  {
    if (IsIngredientDefinedDry && MeasureKind == MeasureKindTy::Wet)
    {
      Diagnostic.report(CurrentToken.getLineNumber(),
                        CurrentToken.getColumnNumber())
          << "Wet measure used when dry measure kind specified" << std::endl;
      Diagnostic.printLine(CurrentToken.getLineNumber(),
                           CurrentToken.getBegin(), CurrentToken.getEnd());
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    IngredientInfo.IsDry = MeasureKind != MeasureKindTy::Wet;

    IngredientInfo.Measure = IdentifierString;

    if (consumeAndExpectToken(TokenKind::Identifier))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }
  }

  const std::size_t BeginIngredientNamePos = CurrentToken.getBegin();
  while (CurrentToken.isNotAnyOf(TokenKind::NewLine, TokenKind::EndOfParagraph,
                                 TokenKind::EndOfFile))
  {
    getNextToken();
  }
  const std::size_t EndIngredientNamePos = CurrentToken.getBegin();
  IngredientInfo.Name =
      Lexer.getTextSpan(BeginIngredientNamePos, EndIngredientNamePos);

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseCookingTime()
{
  const std::string CookingTimeStr = "Cooking time:";

  // Look ahead to see if we in fact have no comment block in this recipe.
  std::string LookAhead = Lexer.lookAhead(CookingTimeStr.size());
  if (LookAhead.compare(CookingTimeStr))
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

  auto FindResult =
      std::find(std::begin(ValidTimeUnits), std::end(ValidTimeUnits), TimeUnit);
  if (FindResult == std::end(ValidTimeUnits))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
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
  if (LookAhead.compare(OvenTemperature))
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

CheffeErrorCode CheffeParser::parseMethodList()
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
    CheffeErrorCode Success = parseMethodStatement();
    if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
    {
      return Success;
    }
  }
  CHEFFE_DEBUG("\n");

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

CheffeErrorCode CheffeParser::parseMethodStatement()
{
  if (expectToken(TokenKind::Identifier))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  const std::string MethodKeyword = CurrentToken.getIdentifierString();
  auto FindResult = std::find(std::begin(ValidMethodKeywords),
                              std::end(ValidMethodKeywords), MethodKeyword);

  bool IsKnownVerb = false;
  const bool IsValidMethodKeyword = FindResult != std::end(ValidMethodKeywords);

  if (!IsValidMethodKeyword)
  {
    FindResult = std::find(std::begin(ValidVerbKeywords),
                           std::end(ValidVerbKeywords), MethodKeyword);

    IsKnownVerb = FindResult != std::end(ValidVerbKeywords);
  }

  if (!IsValidMethodKeyword && !IsKnownVerb)
  {
    Diagnostic.report(CurrentToken.getLineNumber(),
                      CurrentToken.getColumnNumber())
        << "Invalid Method Keyword: '" << MethodKeyword.c_str() << "'"
        << std::endl;
    Diagnostic.printLine(CurrentToken.getLineNumber(), CurrentToken.getBegin(),
                         CurrentToken.getEnd());
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  const std::size_t BeginMethodPos = CurrentToken.getBegin();
  while (CurrentToken.isNotAnyOf(TokenKind::FullStop, TokenKind::EndOfFile))
  {
    getNextToken();
  }

  if (expectToken(TokenKind::FullStop))
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }
  const std::size_t EndMethodPos = CurrentToken.getEnd();
  std::string MethodStatement = Lexer.getTextSpan(BeginMethodPos, EndMethodPos);
  // Pretty-print the method statement - strip out any new lines.
  MethodStatement.erase(
      std::remove(std::begin(MethodStatement), std::end(MethodStatement), '\n'),
      std::end(MethodStatement));
  CHEFFE_DEBUG("\"" << MethodStatement.c_str() << "\"\n");

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
  if (LookAhead.compare(ServesStr))
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
      Diagnostic.report(CurrentToken.getLineNumber(),
                        CurrentToken.getColumnNumber())
          << "Invalid Serves Statement" << std::endl;
      return CheffeErrorCode::CHEFFE_ERROR;
    }
  }

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

} // end namespace cheffe
