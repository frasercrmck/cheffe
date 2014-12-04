#ifndef CHEFFE_LEXER
#define CHEFFE_LEXER

#include "cheffe.h"
#include "CheffeToken.h"

#include <string>

namespace cheffe
{

class CheffeLexer
{
private:
  std::size_t CurrentPos;
  CheffeSourceFile File;

  unsigned LineNumber;
  unsigned ColumnNumber;

  bool IgnoreNewLines;

public:
  CheffeLexer(const CheffeSourceFile &SrcFile)
      : CurrentPos(0), File(SrcFile), LineNumber(1), ColumnNumber(1),
        IgnoreNewLines(false)
  {
  }

  // Gets a char from the input
  int getNextChar();

  // Peeks at the next char from the input
  int peekNextChar();

  // Returns a token from the input, consuming one or more characters.
  Token getToken();

  // Looks ahead NumChars chars, or until the end of the file.
  std::string lookAhead(const std::size_t NumChars);

  // Returns a copy of the span of text from the input file.
  std::string getTextSpan(const std::size_t Begin, const std::size_t End);

  void setIgnoreNewLines(const bool Ignore)
  {
    IgnoreNewLines = Ignore;
  }
};

} // end namespace cheffe

#endif
