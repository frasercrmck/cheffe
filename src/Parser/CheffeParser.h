#ifndef CHEFFE_PARSER
#define CHEFFE_PARSER

#include "cheffe.h"
#include "Lexer/CheffeLexer.h"
#include "Utils/CheffeDiagnosticHandler.h"

#include <vector>
#include <unordered_set>

namespace cheffe
{

const std::unordered_set<std::string> ValidTimeUnits = {
  "hour", "hours", "minute", "minutes"
};

const std::unordered_set<std::string> ValidMethodKeywords = {
  "Take",    "Put",  "Fold", "Add",   "Remove", "Combine", "Divide", "Liquefy",
  "Liquify", "Stir", "Mix",  "Clean", "Pour",   "Set",     "Serve",  "Refrigerate"
};

const std::unordered_set<std::string> ValidVerbKeywords = {
  "Sift", "Rub", "Melt", "Caramelise", "Cook", "Heat"
};

class CheffeParser
{
private:
  CheffeLexer Lexer;
  Token CurrentToken;

  CheffeDiagnosticHandler Diagnostic;

public:
  CheffeParser(const std::string& File) : Lexer(File), CurrentToken()
  {
  }

  CheffeErrorCode parseRecipe();

  CheffeErrorCode parseRecipeTitle();
  CheffeErrorCode parseCommentBlock();
  CheffeErrorCode parseIngredientsList();
  CheffeErrorCode parseCookingTime();
  CheffeErrorCode parseOvenTemperature();
  CheffeErrorCode parseMethod();
  CheffeErrorCode parseMethodStatement();
  CheffeErrorCode parseServesStatement();

  Token getNextToken();

  // Return true if token didn't match, false otherwise.
  bool expectToken(const Token& Tok);
  bool consumeAndExpectToken(const Token& Tok);
};

} // end namespace cheffe

#endif
