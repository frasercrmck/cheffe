#include "Parser/CheffeParser.h"
#include "Utils/CheffeDebugUtils.h"

#include <iostream>
#include <algorithm>

namespace cheffe
{
Token CheffeParser::getNextToken()
{
  return CurrentToken = Lexer.getToken();
}

int CheffeParser::parseRecipe()
{
  int Success = CHEFFE_SUCCESS;

  do
  {
    Success = parseRecipeTitle();
    if (Success != CHEFFE_SUCCESS)
    {
      return Success;
    }

    Success = parseCommentBlock();
    if (Success != CHEFFE_SUCCESS)
    {
      return Success;
    }

    Success = parseIngredientsList();
    if (Success != CHEFFE_SUCCESS)
    {
      return Success;
    }

    Success = parseCookingTime();
    if (Success != CHEFFE_SUCCESS)
    {
      return Success;
    }

    Success = parseOvenTemperature();
    if (Success != CHEFFE_SUCCESS)
    {
      return Success;
    }

    Success = parseMethod();
    if (Success != CHEFFE_SUCCESS)
    {
      return Success;
    }

    Success = parseServesStatement();
    if (Success != CHEFFE_SUCCESS)
    {
      return Success;
    }
  } while (CurrentToken.isNot(TokenKindEOF));

  return Success;
}

int CheffeParser::parseRecipeTitle()
{
  int Success = CHEFFE_SUCCESS;

  getNextToken();
  const std::size_t BeginTitlePos = CurrentToken.getBegin();

  // Parse the Recipe Title.
  while (CurrentToken.isNot(TokenKindFullStop) && CurrentToken.isNot(TokenKindEndOfParagraph) &&
         CurrentToken.isNot(TokenKindEOF))
  {
    getNextToken();
  }

  if (CurrentToken.isNot(TokenKindFullStop))
  {
    std::cerr << "Error while processing Recipe Title: expected '.'\n";
    return CHEFFE_ERROR;
  }
  const std::size_t EndTitlePos = CurrentToken.getBegin();

  const std::string RecipeTitle = Lexer.getTextSpan(BeginTitlePos, EndTitlePos);
  CHEFFE_DEBUG("RECIPE TITLE:\n\"" << RecipeTitle.c_str() << "\"\n\n");

  getNextToken();
  if (CurrentToken.isNot(TokenKindEndOfParagraph))
  {
    std::cerr << "Error while processing Recipe Title: expected EndOfParagraph\n";
    return CHEFFE_ERROR;
  }

  return Success;
}

int CheffeParser::parseCommentBlock()
{
  int Success = CHEFFE_SUCCESS;
  const std::string Ingredients = "Ingredients.\n";

  // Look ahead to see if we in fact have no comment block in this recipe.
  std::string LookAhead = Lexer.lookAhead(Ingredients.size());
  if (!LookAhead.compare(Ingredients))
  {
    return Success;
  }

  getNextToken();
  const std::size_t CommentsBeginPos = CurrentToken.getBegin();
  while (CurrentToken.isNot(TokenKindEndOfParagraph) && CurrentToken.isNot(TokenKindEOF))
  {
    getNextToken();
  }

  if (CurrentToken.isNot(TokenKindEndOfParagraph))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }

  std::string Comments = Lexer.getTextSpan(CommentsBeginPos, CurrentToken.getBegin());
  CHEFFE_DEBUG("COMMENTS:\n\"" << Comments.c_str() << "\"\n\n");

  return Success;
}

int CheffeParser::parseIngredientsList()
{
  int Success = CHEFFE_SUCCESS;

  // Eat the 'Ingredients' token
  getNextToken();
  if (!CurrentToken.equals("Ingredients"))
  {
    std::cerr << "Invalid Ingredient list\n";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.isNot(TokenKindFullStop))
  {
    std::cerr << "Invalid Ingredient list\n";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.isNot(TokenKindNewLine))
  {
    std::cerr << "Invalid Ingredient list\n";
    return CHEFFE_ERROR;
  }

  while (CurrentToken.isNot(TokenKindEndOfParagraph) && CurrentToken.isNot(TokenKindEOF))
  {
    getNextToken();
    const std::size_t BeginIngredientPos = CurrentToken.getBegin();
    while (CurrentToken.isNot(TokenKindNewLine) && CurrentToken.isNot(TokenKindEndOfParagraph) &&
           CurrentToken.isNot(TokenKindEOF))
    {
      getNextToken();
    }
    const std::size_t EndIngredientPos = CurrentToken.getBegin();
    std::string Ingredient = Lexer.getTextSpan(BeginIngredientPos, EndIngredientPos);
    CHEFFE_DEBUG("INGREDIENT: \"" << Ingredient.c_str() << "\"\n");
  }
  CHEFFE_DEBUG("\n");

  return Success;
}

int CheffeParser::parseCookingTime()
{
  int Success = CHEFFE_SUCCESS;
  const std::string CookingTimeStr = "Cooking time:";

  // Look ahead to see if we in fact have no comment block in this recipe.
  std::string LookAhead = Lexer.lookAhead(CookingTimeStr.size());
  if (LookAhead.compare(CookingTimeStr))
  {
    return CHEFFE_SUCCESS;
  }

  getNextToken();
  if (!CurrentToken.equals("Cooking"))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (!CurrentToken.equals("time"))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.isNot(TokenKindColon))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.isNot(TokenKindNumber))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }
  const int Time = CurrentToken.getNumVal();

  getNextToken();
  if (CurrentToken.isNot(TokenKindIdentifier))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }
  const std::string TimeUnit = CurrentToken.getIdentifierString();

  auto FindResult = std::find(std::begin(ValidTimeUnits), std::end(ValidTimeUnits), TimeUnit);
  if (FindResult == std::end(ValidTimeUnits))
  {
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.isNot(TokenKindFullStop))
  {
    std::cerr << "Invalid Cooking Time\n";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.isNot(TokenKindEndOfParagraph))
  {
    std::cerr << "Invalid Cooking Time\n";
    return CHEFFE_ERROR;
  }

  CHEFFE_DEBUG("COOKING TIME: " << Time << " " << TimeUnit.c_str() << "\n\n");

  return Success;
}

int CheffeParser::parseOvenTemperature()
{
  int Success = CHEFFE_SUCCESS;
  const std::string OvenTemperature = "Pre-heat oven to";

  // Look ahead to see if we in fact have no comment block in this recipe.
  std::string LookAhead = Lexer.lookAhead(OvenTemperature.size());
  if (LookAhead.compare(OvenTemperature))
  {
    return CHEFFE_SUCCESS;
  }

  getNextToken();
  const std::size_t OvenTempBeginPos = CurrentToken.getBegin();

  if (!CurrentToken.equals("Pre"))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.isNot(TokenKindHyphen))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (!CurrentToken.equals("heat"))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (!CurrentToken.equals("oven"))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (!CurrentToken.equals("to"))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.isNot(TokenKindNumber))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }
  const int Temperature = CurrentToken.getNumVal();

  getNextToken();
  if (!CurrentToken.equals("degrees"))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (!CurrentToken.equals("Celcius"))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }

  getNextToken();

  int GasMark = 0;
  bool HasGasMark = false;
  if (CurrentToken.is(TokenKindOpenBrace))
  {
    getNextToken();
    if (!CurrentToken.equals("gas"))
    {
      std::cerr << "";
      return CHEFFE_ERROR;
    }

    getNextToken();
    if (!CurrentToken.equals("mark"))
    {
      std::cerr << "";
      return CHEFFE_ERROR;
    }

    getNextToken();
    if (CurrentToken.isNot(TokenKindNumber))
    {
      std::cerr << "";
      return CHEFFE_ERROR;
    }
    GasMark = CurrentToken.getNumVal();

    getNextToken();
    if (CurrentToken.isNot(TokenKindCloseBrace))
    {
      std::cerr << "";
      return CHEFFE_ERROR;
    }

    HasGasMark = true;
  }

  getNextToken();
  if (CurrentToken.isNot(TokenKindFullStop))
  {
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.isNot(TokenKindEndOfParagraph))
  {
    return CHEFFE_ERROR;
  }

  CHEFFE_DEBUG("OVEN TEMPERATURE: " << Temperature);
  if (HasGasMark)
  {
    CHEFFE_DEBUG(" (gas mark " << GasMark << ")");
  }
  CHEFFE_DEBUG("\n");

  return Success;
}

int CheffeParser::parseMethod()
{
  int Success = CHEFFE_SUCCESS;
  // Eat the 'Method' token
  getNextToken();
  if (!CurrentToken.equals("Method"))
  {
    std::cerr << "Invalid Method list\n";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.isNot(TokenKindFullStop))
  {
    std::cerr << "Invalid Method list\n";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.isNot(TokenKindNewLine))
  {
    std::cerr << "Invalid Method list\n";
    return CHEFFE_ERROR;
  }

  getNextToken();
  CHEFFE_DEBUG("\nMETHOD LIST:\n");
  while (CurrentToken.isNot(TokenKindEndOfParagraph) && CurrentToken.isNot(TokenKindEOF))
  {
    Success = parseMethodStatement();
    if (Success != CHEFFE_SUCCESS)
    {
      return Success;
    }
  }
  CHEFFE_DEBUG("\n");

  return CHEFFE_SUCCESS;
}

int CheffeParser::parseMethodStatement()
{
  if (CurrentToken.isNot(TokenKindIdentifier))
  {
    std::cerr << "Invalid Method Statement\n";
    return CHEFFE_ERROR;
  }

  const std::string MethodKeyword = CurrentToken.getIdentifierString();
  auto FindResult =
      std::find(std::begin(ValidMethodKeywords), std::end(ValidMethodKeywords), MethodKeyword);
  bool IsValidMethodKeyword = FindResult == std::end(ValidMethodKeywords);

  if (!IsValidMethodKeyword)
  {
    FindResult =
        std::find(std::begin(ValidVerbKeywords), std::end(ValidVerbKeywords), MethodKeyword);

    IsValidMethodKeyword = FindResult == std::end(ValidVerbKeywords);
  }

  if (!IsValidMethodKeyword)
  {
    std::cerr << "Invalid Method Keyword: '" << MethodKeyword.c_str() << "'\n";
    return CHEFFE_ERROR;
  }

  const std::size_t BeginMethodPos = CurrentToken.getBegin();
  while (CurrentToken.isNot(TokenKindFullStop) && CurrentToken.isNot(TokenKindEOF))
  {
    getNextToken();
  }

  if (CurrentToken.isNot(TokenKindFullStop))
  {
    std::cerr << "Error while processing Method Statement: expected '.'\n";
    return CHEFFE_ERROR;
  }
  const std::size_t EndMethodPos = CurrentToken.getEnd();
  std::string MethodStatement = Lexer.getTextSpan(BeginMethodPos, EndMethodPos);
  // Pretty-print the method statement - strip out any new lines.
  MethodStatement.erase(std::remove(std::begin(MethodStatement), std::end(MethodStatement), '\n'),
                        std::end(MethodStatement));
  CHEFFE_DEBUG("\"" << MethodStatement.c_str() << "\"\n");

  getNextToken();
  while (CurrentToken.is(TokenKindNewLine))
  {
    getNextToken();
  }
  return CHEFFE_SUCCESS;
}

int CheffeParser::parseServesStatement()
{
  if (CurrentToken.is(TokenKindEOF))
  {
    return CHEFFE_SUCCESS;
  }

  const std::string ServesStr = "Serves";

  // Look ahead to see if we in fact have no serves statement in this recipe.
  std::string LookAhead = Lexer.lookAhead(ServesStr.size());
  if (LookAhead.compare(ServesStr))
  {
    return CHEFFE_SUCCESS;
  }

  getNextToken();
  if (!CurrentToken.equals(ServesStr))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }

  getNextToken();
  if (CurrentToken.isNot(TokenKindNumber))
  {
    std::cerr << "";
    return CHEFFE_ERROR;
  }
  const int ServesNum = CurrentToken.getNumVal();

  getNextToken();
  if (CurrentToken.isNot(TokenKindFullStop))
  {
    std::cerr << "Invalid Serves Statement\n";
    return CHEFFE_ERROR;
  }

  CHEFFE_DEBUG("SERVES: " << ServesNum << "\n\n");

  getNextToken();
  if (CurrentToken.isNot(TokenKindEndOfParagraph) && CurrentToken.isNot(TokenKindEOF))
  {
    if (CurrentToken.isNot(TokenKindNewLine))
    {
      std::cerr << "Invalid Serves Statement\n";
      return CHEFFE_ERROR;
    }

    getNextToken();
    if (CurrentToken.isNot(TokenKindEndOfParagraph) && CurrentToken.isNot(TokenKindEOF))
    {
      std::cerr << "Invalid Serves Statement\n";
      return CHEFFE_ERROR;
    }
  }

  return CHEFFE_SUCCESS;
}

} // end namespace cheffe
