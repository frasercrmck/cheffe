#include "gtest/gtest.h"

#include "cheffe.h"
#include "Driver/CheffeDriver.h"
#include "Parser/CheffeParser.h"
#include "Utils/CheffeFileHandler.h"

#include <string>
#include <fstream>

using namespace cheffe;

class ParserTest : public ::testing::Test
{
public:
  ParserTest()
  {
  }

  void DoTest(const char *name, CheffeErrorCode &Error)
  {
    std::string DirPath = std::string(TEST_ROOT_PATH);
    CheffeSourceFile InFile = {DirPath.append(name), ""};

    const CheffeErrorCode Ret = CheffeFileHandler::readFile(InFile);

    ASSERT_EQ(Ret, CheffeErrorCode::CHEFFE_SUCCESS);

    ASSERT_FALSE(InFile.Source.empty());

    CheffeDriver Driver;
    Driver.setSourceFile(InFile);

    auto Diagnostics = std::make_shared<CheffeDiagnosticHandler>();

    Driver.setDiagnosticHandler(Diagnostics);

    Error = Driver.compileRecipe();
  }
};

class GoodParserTest : public ParserTest
{
public:
  void TestParse(const char *Name)
  {
    CheffeErrorCode Error = CheffeErrorCode::CHEFFE_SUCCESS;
    DoTest(Name, Error);
    EXPECT_EQ(Error, CheffeErrorCode::CHEFFE_SUCCESS);
  }
};

class BadParserTest : public ParserTest
{
public:
  void TestParse(const char *Name)
  {
    CheffeErrorCode Error = CheffeErrorCode::CHEFFE_SUCCESS;
    DoTest(Name, Error);
    EXPECT_EQ(Error, CheffeErrorCode::CHEFFE_ERROR);
  }
};

TEST_F(GoodParserTest, Hello)
{
  TestParse("/Parser/hello.ch");
}

TEST_F(GoodParserTest, HelloFull)
{
  TestParse("/Parser/hello-full.ch");
}

TEST_F(GoodParserTest, Fibonacci)
{
  TestParse("/Parser/fibonacci.ch");
}

TEST_F(GoodParserTest, MethodLineBreak)
{
  TestParse("/Parser/method-line-break.ch");
}

TEST_F(GoodParserTest, NoComments)
{
  TestParse("/Parser/no-comments.ch");
}

TEST_F(GoodParserTest, IngredientNoInitialValue)
{
  TestParse("/Parser/ingredient-no-initial-value.ch");
}

TEST_F(GoodParserTest, MismatchedCookingTime)
{
  TestParse("/Parser/mismatched-cooking-time.ch");
}

TEST_F(GoodParserTest, MismatchedCookingTime2)
{
  TestParse("/Parser/mismatched-cooking-time-2.ch");
}

TEST_F(GoodParserTest, IngredientNoMeasure)
{
  TestParse("/Parser/ingredient-no-measure.ch");
}

TEST_F(GoodParserTest, MethodStepPut)
{
  TestParse("/Parser/method-step-put.ch");
}

TEST_F(GoodParserTest, MethodStepFold)
{
  TestParse("/Parser/method-step-fold.ch");
}

TEST_F(GoodParserTest, MethodStepAdd)
{
  TestParse("/Parser/method-step-add.ch");
}

TEST_F(GoodParserTest, MethodStepRemove)
{
  TestParse("/Parser/method-step-remove.ch");
}

TEST_F(GoodParserTest, MethodStepCombine)
{
  TestParse("/Parser/method-step-combine.ch");
}

TEST_F(GoodParserTest, MethodStepDivide)
{
  TestParse("/Parser/method-step-divide.ch");
}

TEST_F(GoodParserTest, MethodStepAddDry)
{
  TestParse("/Parser/method-step-add-dry.ch");
}

TEST_F(GoodParserTest, MethodStepLiquify)
{
  TestParse("/Parser/method-step-liquify.ch");
}

TEST_F(GoodParserTest, MethodStepStir)
{
  TestParse("/Parser/method-step-stir.ch");
}

TEST_F(GoodParserTest, MethodStepMix)
{
  TestParse("/Parser/method-step-mix.ch");
}

TEST_F(GoodParserTest, MethodStepClean)
{
  TestParse("/Parser/method-step-clean.ch");
}

TEST_F(GoodParserTest, MethodStepPour)
{
  TestParse("/Parser/method-step-pour.ch");
}

TEST_F(GoodParserTest, MethodStepRefrigerate)
{
  TestParse("/Parser/method-step-refrigerate.ch");
}

TEST_F(BadParserTest, BadComments)
{
  TestParse("/Parser/bad-comments.ch");
}

TEST_F(BadParserTest, BadTitle)
{
  TestParse("/Parser/bad-title.ch");
}

TEST_F(BadParserTest, IllegalIngredient)
{
  TestParse("/Parser/illegal-ingredient.ch");
}

TEST_F(BadParserTest, IllegalIngredientNoName)
{
  TestParse("/Parser/illegal-ingredient-no-name.ch");
}

TEST_F(BadParserTest, IllegalIngredientNoName2)
{
  TestParse("/Parser/illegal-ingredient-no-name-2.ch");
}

TEST_F(BadParserTest, IllegalMethodKeyword)
{
  TestParse("/Parser/illegal-method-keyword.ch");
}

TEST_F(BadParserTest, IllegalCookingTime)
{
  TestParse("/Parser/illegal-cooking-time.ch");
}

TEST_F(BadParserTest, RecipeDefinedTwice)
{
  TestParse("/Parser/recipe-defined-twice.ch");
}

TEST_F(ParserTest, TestOrdinalSuffixes)
{
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(1, "st"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(2, "nd"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(3, "rd"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(4, "th"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(11, "th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(12, "th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(13, "th"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::checkOrdinalIdentifier(11, "st"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::checkOrdinalIdentifier(12, "nd"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::checkOrdinalIdentifier(13, "rd"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(21, "st"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(22, "nd"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(23, "rd"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(100, "th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(101, "st"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(102, "nd"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(100, "th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(101, "st"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(102, "nd"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(103, "rd"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::checkOrdinalIdentifier(101, "th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::checkOrdinalIdentifier(102, "th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::checkOrdinalIdentifier(103, "th"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(111, "th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(112, "th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::checkOrdinalIdentifier(113, "th"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::checkOrdinalIdentifier(1, "xx"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::checkOrdinalIdentifier(2, "yz"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::checkOrdinalIdentifier(3, "ii"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::checkOrdinalIdentifier(100, "test"));

  std::string expected_suffix;
  for (unsigned i = 0, lsd = 0; i < 1000; ++i, lsd = (lsd == 9) ? 0 : ++lsd)
  {
    switch (lsd)
    {
    default:
      cheffe_unreachable("Impossible digit");
      break;
    case 1:
    case 2:
    case 3:
    {
      if ((i % 100) > 10 && (i % 100) < 14)
      {
        expected_suffix = "th";
        break;
      }
      const char *suffixes[] = {"st", "nd", "rd"};
      expected_suffix = suffixes[lsd - 1];
      break;
    }
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 0:
      expected_suffix = "th";
      break;
    }
    ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
              CheffeParser::checkOrdinalIdentifier(i, expected_suffix));
    expected_suffix.clear();
  }
}
