#ifndef CHEFFE_DIAGNOSTIC_HANDLER
#define CHEFFE_DIAGNOSTIC_HANDLER

#include "Lexer/CheffeToken.h"
#include "Utils/CheffeFileHandler.h"
#include "Utils/CheffeErrorHandling.h"
#include <iostream>
#include <sstream>
#include <vector>

namespace cheffe
{

enum class DiagnosticKind
{
  Warning,
  Error
};

enum class LineContext
{
  WithContext,
  WithoutContext
};

class CheffeDiagnosticBuilder;

class CheffeDiagnosticHandler
{
private:
  CheffeSourceFile File;

  std::ostream &errs();

  std::vector<std::string> Errors;
  std::vector<std::string> Warnings;

public:
  CheffeDiagnosticHandler()
  {
  }

  void formatAndLogMessage(const std::string &Message,
                           const SourceLocation SourceLoc,
                           const DiagnosticKind Kind,
                           const LineContext Context);

  void flushDiagnostics();

  unsigned getErrorCount() const;
  unsigned getWarningCount() const;

  void setSourceFile(const CheffeSourceFile &SrcFile)
  {
    File = SrcFile;
  }

  inline CheffeDiagnosticBuilder report(const SourceLocation SourceLoc,
                                        const DiagnosticKind Kind,
                                        const LineContext Context);

  std::string getLineAsString(const SourceLocation SourceLoc);
  std::string getContextAsString(const SourceLocation SourceLoc);
  std::string getFileAndLineNumberInfoAsString(const unsigned LineNo,
                                               const unsigned ColumnNo);
};

class CheffeDiagnosticBuilder
{
private:
  CheffeDiagnosticHandler *Handler;
  SourceLocation SourceLoc;
  DiagnosticKind Kind;
  LineContext Context;
  std::stringstream MessageStream;

public:
  CheffeDiagnosticBuilder(CheffeDiagnosticHandler *Diags,
                          const SourceLocation SourceLoc,
                          const DiagnosticKind Kind, const LineContext Context)
      : Handler(Diags), SourceLoc(SourceLoc), Kind(Kind), Context(Context),
        MessageStream()
  {
  }

  CheffeDiagnosticBuilder(const CheffeDiagnosticBuilder &Builder)
  {
    Handler = Builder.Handler;
    SourceLoc = Builder.SourceLoc;
    Kind = Builder.Kind;
    Context = Builder.Context;
    MessageStream << Builder.MessageStream.rdbuf();
  }

  ~CheffeDiagnosticBuilder()
  {
    const std::string Message = MessageStream.str();
    Handler->formatAndLogMessage(Message, SourceLoc, Kind, Context);
  }

  inline CheffeDiagnosticBuilder &operator<<(const char *Str)
  {
    MessageStream << Str;
    return *this;
  }

  inline CheffeDiagnosticBuilder &operator<<(std::string Str)
  {
    MessageStream << Str;
    return *this;
  }

  inline CheffeDiagnosticBuilder &operator<<(Token Tok)
  {
    MessageStream << Tok;
    return *this;
  }
};

inline CheffeDiagnosticBuilder
CheffeDiagnosticHandler::report(const SourceLocation SourceLoc,
                                const DiagnosticKind Kind,
                                const LineContext Context)
{
  return CheffeDiagnosticBuilder(this, SourceLoc, Kind, Context);
}

} // end namespace cheffe

#endif
