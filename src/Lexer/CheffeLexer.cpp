#include "Lexer/CheffeLexer.h"

#include <cassert>

namespace cheffe
{

int CheffeLexer::getNextChar()
{
  const char Char = File[CurrentPos++];
  if (Char == '\n')
  {
    ++LineNumber;
    ColumnNumber = 0;
  }
  else
  {
    ++ColumnNumber;
  }
  return Char;
}

int CheffeLexer::peekNextChar()
{
  if (CurrentPos < File.size())
  {
    return File[CurrentPos];
  }
  return END_OF_FILE;
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
  Token Tok(TokenKindUnknown);

  Tok.Begin = CurrentPos;

  // Skip any whitespace.
  while (isspace(peekNextChar()))
  {
    if (peekNextChar() == NEW_LINE)
    {
      Tok.Kind = TokenKindNewLine;

      getNextChar();
      if (peekNextChar() == NEW_LINE)
      {
        getNextChar();
        Tok.Kind = TokenKindEndOfParagraph;
      }

      Tok.LineNumber = LineNumber;
      Tok.ColumnNumber = ColumnNumber;
      Tok.End = CurrentPos;
      return Tok;
    }

    getNextChar();
  }

  Tok.Begin = CurrentPos;
  Tok.LineNumber = LineNumber;
  Tok.ColumnNumber = ColumnNumber;

  const int Char = getNextChar();
  if (isalpha(Char))
  {
    std::string IdentifierString{ (char)Char };
    while (isalpha(peekNextChar()))
    {
      IdentifierString += getNextChar();
    }

    Tok.IdentifierString = std::move(IdentifierString);

    Tok.End = CurrentPos;
    Tok.Kind = TokenKindIdentifier;
    return Tok;
  }

  if (isdigit(Char))
  {
    std::string NumStr = { (char)Char };
    while (isdigit(peekNextChar()))
    {
      NumStr += getNextChar();
    }

    Tok.NumVal = strtod(NumStr.c_str(), 0);

    Tok.End = CurrentPos;
    Tok.Kind = TokenKindNumber;
    return Tok;
  }

  if (Char == END_OF_FILE)
  {
    Tok.End = CurrentPos;
    Tok.Kind = TokenKindEOF;
    return Tok;
  }

  if (ispunct(Char))
  {
    Tok.End = CurrentPos;
    Tok.Kind = TokenKindUnknown;

    switch (Char)
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

  Tok.End = CurrentPos;
  Tok.Kind = TokenKindUnknown;
  return Tok;
}

} // end namespace cheffe
