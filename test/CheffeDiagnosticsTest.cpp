#include "gtest/gtest.h"

#include "cheffe.h"
#include "Driver/CheffeDriver.h"
#include "Parser/CheffeParser.h"
#include "IR/CheffeProgramInfo.h"
#include "Utils/CheffeFileHandler.h"

#include <regex>
#include <string>
#include <fstream>
#include <sstream>

struct ErrorStreamRedirector
{
  ErrorStreamRedirector()
      : Buffer(), OldErrorStream(std::cerr.rdbuf(Buffer.rdbuf()))
  {
  }

  // Release the std::cerr stream again
  ~ErrorStreamRedirector()
  {
    std::cerr.rdbuf(OldErrorStream);
  }

  std::string getErrorString()
  {
    return Buffer.str();
  }

private:
  std::stringstream Buffer;
  std::streambuf *OldErrorStream;
};

using namespace cheffe;

class DiagnosticsTest : public ::testing::Test
{
public:
  DiagnosticsTest() : Redirector()
  {
  }

  void DoTest(const char *Name,
              const std::pair<unsigned, unsigned> ExpectedDiagnosticCount)
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
    Driver.compileProgram(ProgramInfo);

    ASSERT_EQ(ExpectedDiagnosticCount.first, Diagnostics->getErrorCount())
        << "Expected " << ExpectedDiagnosticCount.first << " errors, found "
        << Diagnostics->getErrorCount() << "!";
    ASSERT_EQ(ExpectedDiagnosticCount.second, Diagnostics->getWarningCount())
        << "Expected " << ExpectedDiagnosticCount.second << " warnings, found "
        << Diagnostics->getWarningCount() << "!";
    Diagnostics->flushDiagnostics();
  }

  void CheckFileNameDiagnostic(const std::string &DiagnosticString,
                               const std::string &FileName, const char *LineNo,
                               const char *ColumnNo)
  {
    std::stringstream ss;
    ss << FileName << ":([[:digit:]]+):([[:digit:]]+)";
    std::regex FileRegex(ss.str());

    std::smatch FileMatch;
    ASSERT_TRUE(std::regex_search(DiagnosticString, FileMatch, FileRegex));

    ASSERT_EQ(0, FileMatch.str(1).compare(LineNo));
    ASSERT_EQ(0, FileMatch.str(2).compare(ColumnNo));
  }

  std::string getStandardError()
  {
    return Redirector.getErrorString();
  }

private:
  ErrorStreamRedirector Redirector;
};

TEST_F(DiagnosticsTest, MismatchedTimeUnit)
{
  const std::string FileName = "/Parser/mismatched-cooking-time.ch";
  DoTest(FileName.c_str(), std::make_pair(0u, 1u));

  const std::string Warnings = getStandardError();

  CheckFileNameDiagnostic(Warnings, FileName, "8", "17");

  std::regex DiagnosticRegex("Singular cooking time used alongside 'hours'");

  ASSERT_TRUE(std::regex_search(Warnings, DiagnosticRegex));
}

TEST_F(DiagnosticsTest, MismatchedTimeUnit2)
{
  const std::string FileName = "/Parser/mismatched-cooking-time-2.ch";
  DoTest(FileName.c_str(), std::make_pair(0u, 1u));

  const std::string Warnings = getStandardError();

  CheckFileNameDiagnostic(Warnings, FileName, "8", "17");

  std::regex DiagnosticRegex("Plural cooking time used alongside 'hour'");

  ASSERT_TRUE(std::regex_search(Warnings, DiagnosticRegex));
}

TEST_F(DiagnosticsTest, IllegalMethodKeyword)
{
  const std::string FileName = "/Parser/illegal-method-keyword.ch";
  DoTest(FileName.c_str(), std::make_pair(1u, 0u));

  const std::string Errors = getStandardError();

  CheckFileNameDiagnostic(Errors, FileName, "9", "1");

  std::regex DiagnosticRegex("Invalid Method Step Keyword: '(\\w+)'");

  std::smatch KeywordMatch;
  ASSERT_TRUE(std::regex_search(Errors, KeywordMatch, DiagnosticRegex));

  ASSERT_EQ(KeywordMatch.str(1), "Test");
}
