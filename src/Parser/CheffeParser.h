#ifndef CHEFFE_PARSER
#define CHEFFE_PARSER

#include "cheffe.h"
#include "Lexer/CheffeLexer.h"

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
  "Sift", "Rub"
};

class CheffeParser
{
private:
  CheffeLexer Lexer;
  Token CurrentToken;

public:
  CheffeParser(const std::string& File) : Lexer(File), CurrentToken()
  {
  }

  int parseRecipe();

  int parseRecipeTitle();
  int parseCommentBlock();
  int parseIngredientsList();
  int parseCookingTime();
  int parseOvenTemperature();
  int parseMethod();
  int parseMethodStatement();
  int parseServesStatement();

  Token getNextToken();
};

} // end namespace cheffe

#endif
