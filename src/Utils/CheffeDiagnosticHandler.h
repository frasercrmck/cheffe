#ifndef CHEFFE_DIAGNOSTIC_HANDLER
#define CHEFFE_DIAGNOSTIC_HANDLER

#include "cheffe.h"
#include "Lexer/CheffeToken.h"
#include <iostream>
#include <sstream>
#include <vector>

namespace cheffe
{

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

  void formatAndLogMessage(const std::string &Message)
  {
    // TODO: Store in vector 'Errors' for flushing later
    errs() << Message << std::endl;
  }

  void flushDiagnostics()
  {
    for (auto &Message : Errors)
    {
      std::cout << Message << std::endl;
    }
  }

  void setSourceFile(const CheffeSourceFile &SrcFile)
  {
    File = SrcFile;
  }

  inline CheffeDiagnosticBuilder report(const unsigned LineNo,
                                        const unsigned ColumnNo);

  void printLine(const unsigned LineNo, const std::size_t Begin,
                 const std::size_t End);
  void printFileAndLineNumberInformation(const unsigned LineNo,
                                         const unsigned ColumnNo);
};

class CheffeDiagnosticBuilder
{
private:
  CheffeDiagnosticHandler *Handler;
  unsigned LineNo;
  unsigned ColumnNo;
  std::stringstream MessageStream;

public:
  CheffeDiagnosticBuilder(CheffeDiagnosticHandler *Diags, const unsigned Line,
                          const unsigned Column)
      : Handler(Diags), LineNo(Line), ColumnNo(Column), MessageStream()
  {
  }

  CheffeDiagnosticBuilder(const CheffeDiagnosticBuilder &Builder)
  {
    Handler = Builder.Handler;
    LineNo = Builder.LineNo;
    ColumnNo = Builder.ColumnNo;
    MessageStream << Builder.MessageStream.rdbuf();
  }

  ~CheffeDiagnosticBuilder()
  {
    const std::string Message = MessageStream.str();
    Handler->formatAndLogMessage(Message);
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
CheffeDiagnosticHandler::report(const unsigned LineNo, const unsigned ColumnNo)
{
  return CheffeDiagnosticBuilder(this, LineNo, ColumnNo);
}

} // end namespace cheffe

#endif
