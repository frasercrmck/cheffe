#ifndef CHEFFE_LEXER
#define CHEFFE_LEXER

#include "CheffeToken.h"
#include "Utils/CheffeFileHandler.h"

#include <string>

namespace cheffe
{

enum class IgnoreWhiteSpace
{
  True,
  False
};

class CheffeLexer
{
private:
  std::size_t CurrentPos;
  CheffeSourceFile File;

  unsigned LineNumber;
  unsigned ColumnNumber;

  bool IgnoreNewLines;

public:
  CheffeLexer()
      : CurrentPos(0), File(), LineNumber(1), ColumnNumber(1),
        IgnoreNewLines(false)
  {
  }

  // Sets the source file to work on
  void setSourceFile(const CheffeSourceFile &SrcFile);

  // Gets a char from the input
  int getNextChar();

  // Peeks at the next char from the input stream. Can optionally ignore any
  // space or tab characters while peeking.
  int peekNextChar(IgnoreWhiteSpace Ignore = IgnoreWhiteSpace::False) const;

  // Returns a token from the input, consuming one or more characters.
  Token getToken();

  // Returns a copy of the span of text from the input file.
  std::string getTextSpan(const std::size_t Begin, const std::size_t End) const;

  void setIgnoreNewLines(const bool Ignore);
};

} // end namespace cheffe

#endif
