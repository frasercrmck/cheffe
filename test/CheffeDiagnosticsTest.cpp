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

TEST_F(DiagnosticsTest, BadPluralityCookingTime1)
{
  const std::string FileName = "/Diagnostics/bad-plurality-cooking-time-1.ch";
  DoTest(FileName.c_str(), std::make_pair(0u, 1u));

  const std::string Warnings = getStandardError();

  CheckFileNameDiagnostic(Warnings, FileName, "8", "17");

  std::regex DiagnosticRegex("Singular cooking time used alongside 'hours'");

  ASSERT_TRUE(std::regex_search(Warnings, DiagnosticRegex));
}

TEST_F(DiagnosticsTest, BadPluralityCookingTime2)
{
  const std::string FileName = "/Diagnostics/bad-plurality-cooking-time-2.ch";
  DoTest(FileName.c_str(), std::make_pair(0u, 1u));

  const std::string Warnings = getStandardError();

  CheckFileNameDiagnostic(Warnings, FileName, "8", "17");

  std::regex DiagnosticRegex("Plural cooking time used alongside 'hour'");

  ASSERT_TRUE(std::regex_search(Warnings, DiagnosticRegex));
}

TEST_F(DiagnosticsTest, BadPluralityServe1)
{
  const std::string FileName = "/Diagnostics/bad-plurality-serve-1.ch";
  DoTest(FileName.c_str(), std::make_pair(0u, 1u));

  const std::string Warnings = getStandardError();

  CheckFileNameDiagnostic(Warnings, FileName, "9", "28");

  std::regex DiagnosticRegex("Singular time period used alongside 'minutes'");

  ASSERT_TRUE(std::regex_search(Warnings, DiagnosticRegex));
}

TEST_F(DiagnosticsTest, BadPluralityServe2)
{
  const std::string FileName = "/Diagnostics/bad-plurality-serve-2.ch";
  DoTest(FileName.c_str(), std::make_pair(0u, 1u));

  const std::string Warnings = getStandardError();

  CheckFileNameDiagnostic(Warnings, FileName, "9", "28");

  std::regex DiagnosticRegex("Plural time period used alongside 'minute'");

  ASSERT_TRUE(std::regex_search(Warnings, DiagnosticRegex));
}

TEST_F(DiagnosticsTest, BadPluralityRefrigerate1)
{
  const std::string FileName = "/Diagnostics/bad-plurality-refrigerate-1.ch";
  DoTest(FileName.c_str(), std::make_pair(0u, 1u));

  const std::string Warnings = getStandardError();

  CheckFileNameDiagnostic(Warnings, FileName, "9", "19");

  std::regex DiagnosticRegex("Singular time period used alongside 'hours'");

  ASSERT_TRUE(std::regex_search(Warnings, DiagnosticRegex));
}

TEST_F(DiagnosticsTest, BadPluralityRefrigerate2)
{
  const std::string FileName = "/Diagnostics/bad-plurality-refrigerate-2.ch";
  DoTest(FileName.c_str(), std::make_pair(0u, 1u));

  const std::string Warnings = getStandardError();

  CheckFileNameDiagnostic(Warnings, FileName, "9", "19");

  std::regex DiagnosticRegex("Plural time period used alongside 'hour'");

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

TEST_F(DiagnosticsTest, OrdinalIdentifiers)
{
  enum : unsigned
  {
    ExpectedWarningCount = 15u
  };
  const std::string FileName = "/Diagnostics/ordinal-identifiers.ch";
  DoTest(FileName.c_str(), std::make_pair(0u, ExpectedWarningCount));

  std::string Warnings = getStandardError();

  std::regex DiagnosticRegex("Incorrect use of ordinal identifier: mismatch "
                             "between number and suffix");

  unsigned MatchCount = 0;
  std::smatch KeywordMatch;

  const std::pair<const char *, const char *>
      LineColumnInfo[ExpectedWarningCount] = {{"7", "16"},
                                              {"8", "16"},
                                              {"9", "16"},
                                              {"10", "14"},
                                              {"11", "19"},
                                              {"11", "54"},
                                              {"11", "88"},
                                              {"12", "25"},
                                              {"13", "26"},
                                              {"14", "12"},
                                              {"15", "23"},
                                              {"16", "11"},
                                              {"17", "9"},
                                              {"18", "23"},
                                              {"18", "50"}};

  while (std::regex_search(Warnings, KeywordMatch, DiagnosticRegex))
  {
    ASSERT_TRUE(MatchCount <= ExpectedWarningCount);
    CheckFileNameDiagnostic(Warnings, FileName,
                            LineColumnInfo[MatchCount].first,
                            LineColumnInfo[MatchCount].second);
    ++MatchCount;
    Warnings = KeywordMatch.suffix().str();
  }

  ASSERT_EQ(MatchCount, ExpectedWarningCount);
}
