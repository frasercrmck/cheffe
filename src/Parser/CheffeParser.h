#ifndef CHEFFE_PARSER
#define CHEFFE_PARSER

#include "cheffe.h"
#include "Lexer/CheffeLexer.h"
#include "Parser/CheffeRecipeInfo.h"
#include "Utils/CheffeDiagnosticHandler.h"

#include <map>
#include <utility>
#include <vector>
#include <set>
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

typedef std::pair<std::string, std::string> StringPair;
const std::set<StringPair> ValidVerbKeywords = {{"Sift", "Sifted"},
                                                {"Rub", "Rubbed"},
                                                {"Melt", "Melted"},
                                                {"Caramelise", "Caramelised"},
                                                {"Cook", "Cooked"},
                                                {"Heat", "Heated"}};

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

enum class ArithmeticMethodStep
{
  Add,
  Remove,
  Combine,
  Divide
};

const std::map<ArithmeticMethodStep, std::string> MethodStepPrepositions = {
    {ArithmeticMethodStep::Add, "to"},
    {ArithmeticMethodStep::Remove, "from"},
    {ArithmeticMethodStep::Combine, "into"},
    {ArithmeticMethodStep::Divide, "into"},
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

  static CheffeErrorCode checkOrdinalIdentifier(const unsigned Number,
                                                const std::string &Sequence);

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

  CheffeErrorCode parsePossibleOrdinalIdentifier(unsigned &MixingBowlNo);

  CheffeErrorCode parseTakeMethodStep();
  CheffeErrorCode parsePutOrFoldMethodStep();
  CheffeErrorCode parseArithmeticMethodStep(const ArithmeticMethodStep Step);
  CheffeErrorCode parseLiquifyMethodStep();
  CheffeErrorCode parseStirMethodStep();
  CheffeErrorCode parseMixMethodStep();
  CheffeErrorCode parseCleanMethodStep();
  CheffeErrorCode parsePourMethodStep();
  CheffeErrorCode parseVerbMethodStep();
  CheffeErrorCode parseSetAsideMethodStep();
  CheffeErrorCode parseServeMethodStep();
  CheffeErrorCode parseRefrigerateMethodStep();

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
