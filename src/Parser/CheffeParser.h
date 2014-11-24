#ifndef CHEFFE_PARSER
#define CHEFFE_PARSER

#include "cheffe.h"
#include "Lexer/CheffeLexer.h"
#include "Utils/CheffeDiagnosticHandler.h"

#include <vector>
#include <unordered_set>

namespace cheffe
{

const std::unordered_set<std::string> ValidSingularTimeUnits = {"hour",
                                                                "minutes"};
const std::unordered_set<std::string> ValidPluralTimeUnits = {"hours",
                                                              "minutes"};

const std::unordered_set<std::string> ValidMethodKeywords = {
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

struct IngredientInfoTy
{
  public:
    bool HasInitialValue = false;
    int InitialValue = 0;
    bool IsDry = true;
    std::string MeasureType = "";
    std::string Measure = "";
    std::string Name = "";
};

std::ostream &operator<<(std::ostream &stream, const IngredientInfoTy &Info);

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

public:
  CheffeParser(const CheffeSourceFile &SrcFile,
               std::shared_ptr<CheffeDiagnosticHandler> Diags)
      : Lexer(SrcFile), CurrentToken(), Diagnostics(Diags)
  {
  }

  CheffeErrorCode parseRecipe();

private:
  CheffeErrorCode parseRecipeTitle();
  CheffeErrorCode parseCommentBlock();
  CheffeErrorCode parseIngredientsList();
  CheffeErrorCode parseIngredient(IngredientInfoTy &IngredientInfo);
  CheffeErrorCode parseCookingTime();
  CheffeErrorCode parseOvenTemperature();
  CheffeErrorCode parseMethodList();
  CheffeErrorCode parseMethodStatement();
  CheffeErrorCode parseServesStatement();

  bool isValidTimeUnit(const std::string &TimeUnit, TimeUnitKindTy &Kind);
  bool isValidMeasure(const std::string &Measure, MeasureKindTy &Kind);

  Token getNextToken();

  // Return true if token didn't match, false otherwise.
  template <typename T> bool expectToken(const T &Tok);

  template <typename T> bool consumeAndExpectToken(const T &Tok);
};

} // end namespace cheffe

#endif
