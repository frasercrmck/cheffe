#include "gtest/gtest.h"

#include "cheffe.h"
#include "Driver/CheffeDriver.h"
#include "Parser/CheffeParser.h"
#include "IR/CheffeProgramInfo.h"
#include "Utils/CheffeFileHandler.h"

#include <set>
#include <string>
#include <fstream>
#include <sstream>

struct StreamRedirector
{
  StreamRedirector()
      : CoutBuffer(), CerrBuffer(),
        OldCoutStream(std::cout.rdbuf(CoutBuffer.rdbuf())),
        OldCerrStream(std::cerr.rdbuf(CerrBuffer.rdbuf()))
  {
  }

  // Release the std::cout stream again
  ~StreamRedirector()
  {
    std::cout.rdbuf(OldCoutStream);
    std::cerr.rdbuf(OldCerrStream);
  }

  std::string getStandardOutString()
  {
    return CoutBuffer.str();
  }

  std::string getStandardErrorString()
  {
    return CerrBuffer.str();
  }

private:
  std::stringstream CoutBuffer;
  std::stringstream CerrBuffer;
  std::streambuf *OldCoutStream;
  std::streambuf *OldCerrStream;
};

using namespace cheffe;

class JITExecutionTest : public ::testing::Test
{
public:
  JITExecutionTest() : Redirector()
  {
  }

  void DoTest(const char *Name)
  {
    std::string DirPath = std::string(TEST_ROOT_PATH);
    CheffeSourceFile InFile = {DirPath.append(Name), ""};

    const CheffeErrorCode Ret = CheffeFileHandler::readFile(InFile);

    ASSERT_EQ(Ret, CheffeErrorCode::CHEFFE_SUCCESS);

    ASSERT_FALSE(InFile.Source.empty());

    CheffeDriver Driver;
    Driver.setSourceFile(InFile);

    auto Diagnostics = std::make_shared<CheffeDiagnosticHandler>();

    Driver.setDiagnosticHandler(Diagnostics);

    auto ProgramInfo = std::unique_ptr<CheffeProgramInfo>(nullptr);
    CheffeErrorCode Success = Driver.compileRecipe(ProgramInfo);

    ASSERT_EQ(Success, CheffeErrorCode::CHEFFE_SUCCESS);
    Diagnostics->flushDiagnostics();

    Success = Driver.executeRecipe(ProgramInfo);

    ASSERT_EQ(Success, CheffeErrorCode::CHEFFE_SUCCESS);
    Diagnostics->flushDiagnostics();
  }

  std::string getStandardOut()
  {
    return Redirector.getStandardOutString();
  }

private:
  StreamRedirector Redirector;
};

TEST_F(JITExecutionTest, Nothing1)
{
  const std::string FileName = "/JITExecution/nothing-1.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_TRUE(Output.empty());
}

TEST_F(JITExecutionTest, Nothing2)
{
  const std::string FileName = "/JITExecution/nothing-2.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_TRUE(Output.empty());
}

TEST_F(JITExecutionTest, PourNothing)
{
  const std::string FileName = "/JITExecution/pour-1.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_TRUE(Output.empty());
}

TEST_F(JITExecutionTest, Put1)
{
  const std::string FileName = "/JITExecution/put-1.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "1 1 1 1 1 1");
}

TEST_F(JITExecutionTest, Put2)
{
  const std::string FileName = "/JITExecution/put-2.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "1 1 1 1 1 1");
}

TEST_F(JITExecutionTest, Put3)
{
  const std::string FileName = "/JITExecution/put-3.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "1 1 1 1 1 1");
}

TEST_F(JITExecutionTest, Put4)
{
  const std::string FileName = "/JITExecution/put-4.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "1 1 1 1");
}

TEST_F(JITExecutionTest, Put5)
{
  const std::string FileName = "/JITExecution/put-5.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "1 1");
}

TEST_F(JITExecutionTest, Put6)
{
  const std::string FileName = "/JITExecution/put-6.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "x = 1");
}

TEST_F(JITExecutionTest, HelloWorld)
{
  const std::string FileName = "/JITExecution/hello.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "Hello world!");
}

TEST_F(JITExecutionTest, HelloWorldCake)
{
  const std::string FileName = "/JITExecution/hello-cake.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "Hello world!");
}

TEST_F(JITExecutionTest, HelloWorldFull)
{
  const std::string FileName = "/JITExecution/hello-full.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "Hello world!");
}

TEST_F(JITExecutionTest, AddDry1)
{
  const std::string FileName = "/JITExecution/adddry-1.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "10");
}

TEST_F(JITExecutionTest, Add1)
{
  const std::string FileName = "/JITExecution/add-1.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "3");
}

TEST_F(JITExecutionTest, Add2)
{
  const std::string FileName = "/JITExecution/add-2.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "42");
}

TEST_F(JITExecutionTest, Remove1)
{
  const std::string FileName = "/JITExecution/remove-1.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "-1");
}

TEST_F(JITExecutionTest, Remove2)
{
  const std::string FileName = "/JITExecution/remove-2.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "1");
}

TEST_F(JITExecutionTest, Combine1)
{
  const std::string FileName = "/JITExecution/combine-1.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "16");
}

TEST_F(JITExecutionTest, Combine2)
{
  const std::string FileName = "/JITExecution/combine-2.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "4398046511104");
}

TEST_F(JITExecutionTest, Divide1)
{
  const std::string FileName = "/JITExecution/divide-1.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "16");
}

TEST_F(JITExecutionTest, Divide2)
{
  const std::string FileName = "/JITExecution/divide-2.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "0");
}

TEST_F(JITExecutionTest, Fold1)
{
  const std::string FileName = "/JITExecution/fold-1.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "1");
}

TEST_F(JITExecutionTest, Fold2)
{
  const std::string FileName = "/JITExecution/fold-2.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "0");
}

TEST_F(JITExecutionTest, Serve1)
{
  const std::string FileName = "/JITExecution/serve-1.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "240");
}

TEST_F(JITExecutionTest, Serve2)
{
  const std::string FileName = "/JITExecution/serve-2.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "240");
}

TEST_F(JITExecutionTest, LiquefyIngr1)
{
  const std::string FileName = "/JITExecution/liquefy-ingr-1.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "*");
}

TEST_F(JITExecutionTest, LiquefyIngr2)
{
  const std::string FileName = "/JITExecution/liquefy-ingr-2.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_EQ(Output, "* 42");
}

TEST_F(JITExecutionTest, StirBowl1)
{
  const std::string FileName = "/JITExecution/stir-bowl-1.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "42 1 2 3 4 5");
}

TEST_F(JITExecutionTest, StirBowl2)
{
  const std::string FileName = "/JITExecution/stir-bowl-2.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "1 42 2 3 4 5");
}

TEST_F(JITExecutionTest, StirBowl3)
{
  const std::string FileName = "/JITExecution/stir-bowl-3.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "1 2 42 3 4 5");
}

TEST_F(JITExecutionTest, StirBowl4)
{
  const std::string FileName = "/JITExecution/stir-bowl-4.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "1 2 3 42 4 5");
}

TEST_F(JITExecutionTest, StirBowl5)
{
  const std::string FileName = "/JITExecution/stir-bowl-5.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "1 2 3 4 42 5");
}

TEST_F(JITExecutionTest, StirBowl6)
{
  const std::string FileName = "/JITExecution/stir-bowl-6.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "1 2 3 4 5 42");
}

TEST_F(JITExecutionTest, StirBowl7)
{
  const std::string FileName = "/JITExecution/stir-bowl-7.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "1 2 3 4 5 42");
}

TEST_F(JITExecutionTest, StirIngredient1)
{
  const std::string FileName = "/JITExecution/stir-ingr-1.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "1 2 3 42 4 5");
}

TEST_F(JITExecutionTest, CleanBowl1)
{
  const std::string FileName = "/JITExecution/clean-bowl-1.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "42");
}

TEST_F(JITExecutionTest, CleanBowl2)
{
  const std::string FileName = "/JITExecution/clean-bowl-2.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "42 4 3 2 1");
}

TEST_F(JITExecutionTest, CleanBowl3)
{
  const std::string FileName = "/JITExecution/clean-bowl-3.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "42 4 3 2 1");
}

TEST_F(JITExecutionTest, MixBowl1)
{
  const std::string FileName = "/JITExecution/mix-bowl-1.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();

  std::set<char> Expected = {'5', '4', '3', '2', '1'};

  for (auto Char : Output)
  {
    if (Char == ' ')
    {
      continue;
    }
    auto Iter = Expected.find(Char);
    ASSERT_NE(Iter, std::end(Expected));
    Expected.erase(Iter);
  }

  ASSERT_TRUE(Expected.empty());
}

TEST_F(JITExecutionTest, Refrigerate1)
{
  const std::string FileName = "/JITExecution/refrigerate-1.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_TRUE(Output.empty());
}

TEST_F(JITExecutionTest, Refrigerate2)
{
  const std::string FileName = "/JITExecution/refrigerate-2.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "1 2 3 4 5");
}

TEST_F(JITExecutionTest, Refrigerate3)
{
  const std::string FileName = "/JITExecution/refrigerate-3.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "1 2 3 4 5 5 4 3 2 1");
}

TEST_F(JITExecutionTest, ControlFlow1)
{
  const std::string FileName = "/JITExecution/control-flow-1.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "1 2 3 4 5 6 7 8 9 10");
}

TEST_F(JITExecutionTest, ControlFlow2)
{
  const std::string FileName = "/JITExecution/control-flow-2.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "**********");
}

TEST_F(JITExecutionTest, ControlFlow3)
{
  const std::string FileName = "/JITExecution/control-flow-3.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "1 * 2 * 3 = 6");
}

TEST_F(JITExecutionTest, ControlFlow4)
{
  const std::string FileName = "/JITExecution/control-flow-4.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "100");
}

TEST_F(JITExecutionTest, FizzBuzz)
{
  const std::string FileName = "/JITExecution/fizzbuzz.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "1 2 Fizz 4 Buzz Fizz 7 8 Fizz Buzz 11 Fizz 13 14 Fizz "
                    "Buzz 16 17 Fizz 19 Buzz");
}

TEST_F(JITExecutionTest, Exp)
{
  const std::string FileName = "/JITExecution/exp.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "729");
}

TEST_F(JITExecutionTest, Loops)
{
  const std::string FileName = "/JITExecution/loops.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "*\n**\n***\n****\n*****\n");
}

TEST_F(JITExecutionTest, 99Bottles)
{
  const std::string FileName = "/JITExecution/99-bottles.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();

  ASSERT_FALSE(Output.empty());

  unsigned Pos = 0;
  std::stringstream sstream;
  const std::string TakeOneDown = "\nTake one down and pass it around";
  for (unsigned BottlesLeft = 99; BottlesLeft >= 1; --BottlesLeft)
  {
    sstream << "\n " << BottlesLeft << " bottles of beer on the wall";
    ASSERT_EQ(Output.substr(Pos, sstream.str().length()), sstream.str());
    Pos += sstream.str().length();
    sstream.str("");

    sstream << "\n " << BottlesLeft << " bottles of beer";
    ASSERT_EQ(Output.substr(Pos, sstream.str().length()), sstream.str());
    Pos += sstream.str().length();
    sstream.str("");

    ASSERT_EQ(Output.substr(Pos, sstream.str().length()), sstream.str());
    Pos += sstream.str().length();
    sstream.str("");

    ASSERT_EQ(Output.substr(Pos, TakeOneDown.length()), TakeOneDown);
    Pos += TakeOneDown.length();
  }

  const std::string NoMoreLeft = "\nNo more bottles of beer\n";
  ASSERT_EQ(Output.substr(Pos, NoMoreLeft.length()), NoMoreLeft);
  Pos += NoMoreLeft.length();

  ASSERT_EQ(Pos, Output.length());
}

TEST_F(JITExecutionTest, MultiTable)
{
  const std::string FileName = "/JITExecution/multi-table.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();

  ASSERT_FALSE(Output.empty());

  unsigned Pos = 0;
  std::stringstream sstream;

  const std::string FirstRow =
      "x\t 1\t 2\t 3\t 4\t 5\t 6\t 7\t 8\t 9\t 10\t 11\t 12\n";
  ASSERT_EQ(Output.substr(Pos, Pos + FirstRow.length()), FirstRow);
  Pos += FirstRow.length();

  for (unsigned y = 1; y <= 12; ++y)
  {
    sstream << " " << y;
    for (unsigned x = 1; x <= 12; ++x)
    {
      sstream << "\t";
      if (x >= y)
      {
        sstream << " " << (x * y);
      }
    }
    sstream << "\n";

    ASSERT_EQ(Output.substr(Pos, sstream.str().length()), sstream.str());
    Pos += sstream.str().length();
    sstream.str("");
  }

  ASSERT_EQ(Pos, Output.length());
}

TEST_F(JITExecutionTest, ResetIngredientValues)
{
  const std::string FileName = "/JITExecution/reset-ingredient-values.ch";
  DoTest(FileName.c_str());
  const std::string Output = getStandardOut();
  ASSERT_EQ(Output, "2 2 2");
}
