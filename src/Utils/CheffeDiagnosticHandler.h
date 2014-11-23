#ifndef CHEFFE_DIAGNOSTIC_HANDLER
#define CHEFFE_DIAGNOSTIC_HANDLER

#include "cheffe.h"
#include "Lexer/CheffeToken.h"
#include <iostream>
#include <sstream>
#include <vector>

namespace cheffe
{

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

public:
  CheffeDiagnosticHandler()
  {
  }

  void formatAndLogMessage(const std::string &Message,
                           const SourceLocation SourceLoc,
                           const LineContext Context)
  {
    std::stringstream ss;
    ss << getFileAndLineNumberInfoAsString(
              SourceLoc.getLineNo(), SourceLoc.getColumnNo()) << std::endl;
    ss << Message << std::endl;
    if (Context == LineContext::WithContext)
    {
      ss << getLineAsString(SourceLoc.getLineNo()) << std::endl;
      ss << getContextAsString(SourceLoc.getColumnNo(), SourceLoc.getLength())
         << std::endl;
    }
    Errors.push_back(ss.str());
  }

  void flushDiagnostics()
  {
    for (auto &Message : Errors)
    {
      errs() << Message << std::endl;
    }
  }

  void setSourceFile(const CheffeSourceFile &SrcFile)
  {
    File = SrcFile;
  }

  inline CheffeDiagnosticBuilder report(const SourceLocation SourceLoc,
                                        const LineContext Context);

  std::string getLineAsString(const unsigned LineNo);
  std::string getContextAsString(const unsigned ColumnNo,
                                 const unsigned Length);
  std::string getFileAndLineNumberInfoAsString(const unsigned LineNo,
                                               const unsigned ColumnNo);
};

class CheffeDiagnosticBuilder
{
private:
  CheffeDiagnosticHandler *Handler;
  SourceLocation SourceLoc;
  LineContext Context;
  std::stringstream MessageStream;

public:
  CheffeDiagnosticBuilder(CheffeDiagnosticHandler *Diags,
                          const SourceLocation SourceLoc,
                          const LineContext Context)
      : Handler(Diags), SourceLoc(SourceLoc), Context(Context), MessageStream()
  {
  }

  CheffeDiagnosticBuilder(const CheffeDiagnosticBuilder &Builder)
  {
    Handler = Builder.Handler;
    SourceLoc = Builder.SourceLoc;
    Context = Builder.Context;
    MessageStream << Builder.MessageStream.rdbuf();
  }

  ~CheffeDiagnosticBuilder()
  {
    const std::string Message = MessageStream.str();
    Handler->formatAndLogMessage(Message, SourceLoc, Context);
  }

  inline CheffeDiagnosticBuilder &operator<<(const char *Str)
  {
    MessageStream << Str;
    return *this;
  }

inline CheffeDiagnosticBuilder &operator<<( std::string Str)
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
                                const LineContext Context)
{
  return CheffeDiagnosticBuilder(this, SourceLoc, Context);
}

} // end namespace cheffe

#endif
