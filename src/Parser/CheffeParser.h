#ifndef CHEFFE_PARSER
#define CHEFFE_PARSER

#include "cheffe.h"
#include "Lexer/CheffeLexer.h"
#include "Parser/CheffeRecipeInfo.h"
#include "Utils/CheffeDiagnosticHandler.h"

#include <map>
#include <vector>
#include <unordered_set>

namespace cheffe
{

const std::unordered_set<std::string> ValidSingularTimeUnits = {"hour",
                                                                "minute"};
const std::unordered_set<std::string> ValidPluralTimeUnits = {"hours",
                                                              "minutes"};

const std::unordered_set<std::string> ValidMethodSteps = {
    "Take",   "Put",     "Fold",    "Add",        "Remove", "Combine",
    "Divide", "Liquefy", "Liquify", "Stir",       "Mix",    "Clean",
    "Pour",   "Set",     "Serve",   "Refrigerate"};

const std::unordered_set<std::string> ValidVerbKeywords = {
    "Sift", "Rub", "Melt", "Caramelise", "Cook", "Heat"};

const std::unordered_set<std::string> ValidMeasureTypes = {"heaped", "level"};

const std::unordered_set<std::string> ValidDryMeasures = {"g", "kg", "pinch",
                                                          "pinches"};

const std::unordered_set<std::string> ValidWetMeasures = {"ml", "l", "dash",
                                                          "dashes"};

const std::unordered_set<std::string> ValidUnspecifiedMeasures = {
    "cup", "cups", "teaspoon", "teaspoons", "tablespoon", "tablespoons"};

enum class TimeUnitKindTy
{
  Singular,
  Plural,
  Invalid
};

enum class MeasureKindTy
{
  Dry,
  Wet,
  Unspecified,
  Invalid
};

class CheffeParser
{
private:
  CheffeLexer Lexer;
  Token CurrentToken;

  std::shared_ptr<CheffeDiagnosticHandler> Diagnostics;

  std::map<std::string, std::unique_ptr<CheffeRecipeInfo>> RecipeInfo;

public:
  CheffeParser(const CheffeSourceFile &SrcFile,
               std::shared_ptr<CheffeDiagnosticHandler> Diags)
      : Lexer(SrcFile), CurrentToken(), Diagnostics(Diags),
        CurrentRecipe(nullptr)
  {
  }

  CheffeErrorCode parseRecipe();

private:
  CheffeRecipeInfo *CurrentRecipe;

  CheffeErrorCode parseRecipeTitle(std::string &RecipeTitle,
                                   SourceLocation &RecipeTitleLoc);
  CheffeErrorCode parseCommentBlock();
  CheffeErrorCode parseIngredientsList();
  CheffeErrorCode parseIngredient(CheffeIngredient &Ingredient);
  CheffeErrorCode parseCookingTime();
  CheffeErrorCode parseOvenTemperature();
  CheffeErrorCode parseMethod();
  CheffeErrorCode parseMethodStep();
  CheffeErrorCode parseServesStatement();

  void emitDiagnosticIfIngredientUndefined(const std::string &Ingredient,
                                           const SourceLocation IngredientLoc);

  bool isValidTimeUnit(const std::string &TimeUnit, TimeUnitKindTy &Kind);
  bool isValidMeasure(const std::string &Measure, MeasureKindTy &Kind);

  Token getNextToken();

  // Return true if token didn't match, false otherwise.
  template <typename T> bool expectToken(const T &Tok);

  template <typename T> bool consumeAndExpectToken(const T &Tok);
};

} // end namespace cheffe

#endif
