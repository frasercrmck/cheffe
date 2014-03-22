#ifndef CHEFFE_LEXER
#define CHEFFE_LEXER

#include "CheffeToken.h"

#include <string>

#define END_OF_FILE '\0'
#define NEW_LINE '\n'

namespace cheffe
{

class CheffeLexer
{
private:
  int CurrentPos;
  std::string File;

  char LastChar;

public:
  CheffeLexer(const std::string& SrcFile) : CurrentPos(-1), File(SrcFile), LastChar(' ')
  {
  }

  // Gets a char from the input
  int getChar();

  // Peeks at the next char from the input
  int peekNextChar();

  // Returns a token from the input, consuming one or more characters.
  Token getToken();

  // Looks ahead NumChars chars, or until the end of the file.
  std::string lookAhead(const std::size_t NumChars);

  // Returns a copy of the span of text from the input file.
  std::string getTextSpan(const std::size_t Begin, const std::size_t End);
};

} // end namespace cheffe

#endif
