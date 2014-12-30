#include "gtest/gtest.h"

#include "cheffe.h"
#include "Driver/CheffeDriver.h"
#include "Parser/CheffeParser.h"
#include "IR/CheffeProgramInfo.h"
#include "Utils/CheffeFileHandler.h"

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

  ASSERT_TRUE("111111\n");
}

TEST_F(JITExecutionTest, Put2)
{
  const std::string FileName = "/JITExecution/put-2.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_TRUE("111111\n");
}

TEST_F(JITExecutionTest, Put3)
{
  const std::string FileName = "/JITExecution/put-3.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_TRUE("111111\n");
}

TEST_F(JITExecutionTest, Put4)
{
  const std::string FileName = "/JITExecution/put-4.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_TRUE("1111\n");
}

TEST_F(JITExecutionTest, Put5)
{
  const std::string FileName = "/JITExecution/put-5.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_TRUE("11\n");
}

TEST_F(JITExecutionTest, Put6)
{
  const std::string FileName = "/JITExecution/put-6.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_TRUE("x = 1\n");
}

TEST_F(JITExecutionTest, HelloWorld)
{
  const std::string FileName = "/JITExecution/hello.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_TRUE("Hello world!\n");
}

TEST_F(JITExecutionTest, HelloWorldFull)
{
  const std::string FileName = "/JITExecution/hello-full.ch";
  DoTest(FileName.c_str());

  const std::string Output = getStandardOut();

  ASSERT_TRUE("Hello world!\n");
}
