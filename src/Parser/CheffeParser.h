#ifndef CHEFFE_PARSER
#define CHEFFE_PARSER

#include "cheffe.h"
#include "Parser/CheffeScopeInfo.h"
#include "Lexer/CheffeLexer.h"
#include "IR/CheffeProgramInfo.h"
#include "Utils/CheffeDiagnosticHandler.h"

#include <map>
#include <utility>
#include <vector>
#include <set>
#include <unordered_set>

namespace cheffe
{

const std::set<std::pair<std::string, std::string>> ValidTimeUnits = {
    {"hour", "hours"}, {"minute", "minutes"}};

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
                                                {"Chop", "Chopped"},
                                                {"Bake", "Baked"},
                                                {"Roast", "Roasted"},
                                                {"Boil", "Boiled"},
                                                {"Chill", "Chilled"},
                                                {"Fry", "Fried"},
                                                {"Loop", "Looped"},
                                                {"Shake", "Shaked"},
                                                {"Sieve", "Sieved"},
                                                {"Squeeze", "Squeezed"},
                                                {"Drip", "Dripped"},
                                                {"Drop", "Dropped"},
                                                {"Scoop", "Scooped"},
                                                {"Coat", "Coated"},
                                                {"Randomize", "Randomized"},
                                                {"Toss", "Tossed"},
                                                {"Infuse", "Infused"},
                                                {"Watch", "Watched"},
                                                {"Smell", "Smelled"},
                                                {"Crush", "Crushed"},
                                                {"Mash", "Mashed"},
                                                {"Grind", "Watched"},
                                                {"Shuffle", "Shuffled"},
                                                {"Layer", "Layered"},
                                                {"Prepare", "Prepared"},
                                                {"Separate", "Separated"},
                                                {"Sprinkle", "Sprinkled"},
                                                {"Move", "Moved"},
                                                {"Recite", "Recited"},
                                                {"Repeat", "Repeated"},
                                                {"Siphon", "Siphoned"},
                                                {"Gulp", "Gulped"},
                                                {"Quote", "Quoted"},
                                                {"Part", "Parted"},
                                                {"Dissolve", "Dissolved"},
                                                {"Agitate", "Agitated"},
                                                {"Cool", "Cooled"},
                                                {"Leave", "Left"},
                                                {"Water", "Watered"},
                                                {"Heat", "Heated"}};

const std::unordered_set<std::string> ValidMeasureTypes = {"heaped", "level"};

const std::unordered_set<std::string> ValidDryMeasures = {"g", "kg", "pinch",
                                                          "pinches"};

const std::unordered_set<std::string> ValidWetMeasures = {"ml", "l", "dash",
                                                          "dashes"};

const std::unordered_set<std::string> ValidUnspecifiedMeasures = {
    "cup", "cups", "teaspoon", "teaspoons", "tablespoon", "tablespoons"};

enum class MeasureKindTy
{
  Dry,
  Wet,
  Unspecified,
  Invalid
};

const std::map<MethodStepKind, std::string> MethodStepPrepositions = {
    {MethodStepKind::Add, "to"},
    {MethodStepKind::Remove, "from"},
    {MethodStepKind::Combine, "into"},
    {MethodStepKind::Divide, "into"},
};

class CheffeParserOptions
{
  friend class CheffeParser;

public:
  CheffeParserOptions()
  {
    StrictChef = false;
  }

  void setStrictChef(const bool Switch)
  {
    StrictChef = Switch;
  }

private:
  unsigned StrictChef : 1;
};

class CheffeParser
{
public:
  CheffeParser()
      : Lexer(), CurrentToken(), Diagnostics(nullptr), CurrentRecipe(nullptr),
        ProgramInfo(new CheffeProgramInfo()), Options(new CheffeParserOptions())
  {
  }

  CheffeErrorCode parseProgram();

  void setSourceFile(const CheffeSourceFile &SrcFile);

  void setDiagnosticHandler(std::shared_ptr<CheffeDiagnosticHandler> Diags);

  std::unique_ptr<CheffeProgramInfo> takeProgramInfo();

  std::shared_ptr<CheffeParserOptions> getOptions() const;

  static bool isValidOrdinalIdentifier(const unsigned Number,
                                       const std::string &Sequence);

private:
  CheffeLexer Lexer;
  Token CurrentToken;

  std::shared_ptr<CheffeDiagnosticHandler> Diagnostics;

  std::shared_ptr<CheffeRecipeInfo> CurrentRecipe;
  std::unique_ptr<CheffeProgramInfo> ProgramInfo;

  std::shared_ptr<CheffeParserOptions> Options;

  CheffeScopeInfo RecipeScopeInfo;

  CheffeErrorCode parseRecipeTitle(std::string &RecipeTitle,
                                   SourceLocation &RecipeTitleLoc);
  CheffeErrorCode parseCommentBlock();
  CheffeErrorCode parseIngredientsList();
  CheffeErrorCode parseIngredient();
  CheffeErrorCode parseCookingTime();
  CheffeErrorCode parseOvenTemperature();
  CheffeErrorCode parseMethod();
  CheffeErrorCode parseMethodStep();
  CheffeErrorCode parseServesStatement();

  CheffeErrorCode parsePossibleOrdinalIdentifier(unsigned &MixingBowlNo);

  CheffeErrorCode parseTakeMethodStep();
  CheffeErrorCode parsePutOrFoldMethodStep(const MethodStepKind Step);
  CheffeErrorCode parseArithmeticMethodStep(const MethodStepKind Step);
  CheffeErrorCode parseLiquefyMethodStep();
  CheffeErrorCode parseStirMethodStep();
  CheffeErrorCode parseMixMethodStep();
  CheffeErrorCode parseCleanMethodStep();
  CheffeErrorCode parsePourMethodStep();
  CheffeErrorCode parseVerbMethodStep();
  CheffeErrorCode parseSetAsideMethodStep();
  CheffeErrorCode parseServeMethodStep();
  CheffeErrorCode parseRefrigerateMethodStep();

  bool getIngredientInfo(const std::string &IngredientName,
                         const SourceLocation IngredientLoc,
                         std::shared_ptr<CheffeIngredient> &Ingredient);

  bool isValidTimeUnit(const std::string &TimeUnit, StringPair &MatchingPair);
  bool isValidMeasure(const std::string &Measure, MeasureKindTy &Kind);

  bool checkCurrentTokenPlurality(const long long Number,
                                  const std::string &Singular,
                                  const std::string &Plural,
                                  const std::string &Name);

  bool checkNonStandardTokenAndConsume(const std::string &Str);

  Token getNextToken();

  // Return true if token didn't match, false otherwise.
  template <typename T> bool expectToken(const T &Tok);

  template <typename T> bool consumeAndExpectToken(const T &Tok);
};

} // end namespace cheffe

#endif
