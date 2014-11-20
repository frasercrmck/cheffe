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
