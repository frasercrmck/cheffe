#include "Lexer/CheffeLexer.h"

#include <cassert>

namespace cheffe
{

int CheffeLexer::getChar()
{
  return File[++CurrentPos];
}

std::string CheffeLexer::getTextSpan(const std::size_t Begin, const std::size_t End)
{
  assert(Begin >= 0 && Begin <= End && "");
  return File.substr(Begin, End - Begin);
}

std::string CheffeLexer::lookAhead(const std::size_t NumChars)
{
  assert(CurrentPos < File.size() && "");
  // This std::string constructor handles the case if
  // CurrentPos + NumChars exceeds File.size().
  return File.substr(CurrentPos, NumChars);
}

Token CheffeLexer::getToken()
{
  Token Tok(TokenKindCount);

  // Skip any whitespace.
  while (isspace(LastChar))
  {
    if (LastChar == NEW_LINE)
    {
      Tok.Begin = CurrentPos;
      Tok.Kind = TokenKindNewLine;

      const char NextChar = CurrentPos < (File.size() - 1) ? File[CurrentPos + 1] : '\0';
      if (NextChar == NEW_LINE)
      {
        LastChar = getChar();
        Tok.Kind = TokenKindEndOfParagraph;
      }

      LastChar = getChar();

      Tok.End = CurrentPos;
      return Tok;
    }

    LastChar = getChar();
  }

  Tok.Begin = CurrentPos;
  if (isalpha(LastChar))
  {
    std::string IdentifierString{ LastChar };
    while (isalpha(LastChar = getChar()))
    {
      IdentifierString += LastChar;
    }

    Tok.End = CurrentPos;
    Tok.Kind = TokenKindIdentifier;
    Tok.IdentifierString = std::move(IdentifierString);
    return Tok;
  }

  if (isdigit(LastChar))
  {
    std::string NumStr;
    do
    {
      NumStr += LastChar;
      LastChar = getChar();
    } while (isdigit(LastChar));

    Tok.NumVal = strtod(NumStr.c_str(), 0);

    Tok.End = CurrentPos;
    Tok.Kind = TokenKindNumber;
    return Tok;
  }

  if (LastChar == END_OF_FILE)
  {
    Tok.End = CurrentPos;
    Tok.Kind = TokenKindEOF;
    return Tok;
  }

  if (ispunct(LastChar))
  {
    const char ThisChar = LastChar;
    LastChar = getChar();

    Tok.End = CurrentPos;
    Tok.Kind = TokenKindTODO;

    switch (ThisChar)
    {
    default:
      break;
    case '.':
      Tok.Kind = TokenKindFullStop;
      break;
    case '-':
      Tok.Kind = TokenKindHyphen;
      break;
    case ':':
      Tok.Kind = TokenKindColon;
      break;
    case '(':
      Tok.Kind = TokenKindOpenBrace;
      break;
    case ')':
      Tok.Kind = TokenKindCloseBrace;
      break;
    }

    return Tok;
  }

  LastChar = getChar();

  Tok.End = CurrentPos;
  Tok.Kind = TokenKindTODO;
  return Tok;
}

} // end namespace cheffe
