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
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::parseOrdinalIdentifier("test"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::parseOrdinalIdentifier("100test"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::parseOrdinalIdentifier("a100test"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("1st"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("01st"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("2nd"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("02nd"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("03rd"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("3rd"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("4th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("04th"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("11th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("12th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("13th"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::parseOrdinalIdentifier("11st"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::parseOrdinalIdentifier("12nd"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::parseOrdinalIdentifier("13rd"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("21st"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("22nd"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("23rd"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("100th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("101st"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("102nd"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("100th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("101st"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("102nd"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("103rd"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::parseOrdinalIdentifier("101th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::parseOrdinalIdentifier("102th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::parseOrdinalIdentifier("103th"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("111th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("112th"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
            CheffeParser::parseOrdinalIdentifier("113th"));

  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::parseOrdinalIdentifier("1xx"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::parseOrdinalIdentifier("2yz"));
  ASSERT_EQ(CheffeErrorCode::CHEFFE_ERROR,
            CheffeParser::parseOrdinalIdentifier("3ii"));

  std::stringstream ss;
  std::string expected_suffix;
  for (unsigned i = 0, lsd = 0; i < 1000; ++i, lsd = (lsd == 9) ? 0 : ++lsd)
  {
    ss << i;
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
    ss << expected_suffix;
    ASSERT_EQ(CheffeErrorCode::CHEFFE_SUCCESS,
              CheffeParser::parseOrdinalIdentifier(ss.str()));
    ss.str(std::string());
    expected_suffix.clear();
  }
}
