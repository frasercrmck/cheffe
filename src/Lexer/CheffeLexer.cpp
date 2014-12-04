#include "Lexer/CheffeLexer.h"

#include <cassert>

namespace cheffe
{

int CheffeLexer::getNextChar()
{
  if (static_cast<std::size_t>(CurrentPos) >= File.Source.size())
  {
    return -1;
  }
  const char Char = File[CurrentPos++];
  if (Char == '\n')
  {
    ++LineNumber;
    ColumnNumber = 1;
  }
  else
  {
    ++ColumnNumber;
  }
  return Char;
}

int CheffeLexer::peekNextChar()
{
  if (static_cast<std::size_t>(CurrentPos) >= File.Source.size())
  {
    return -1;
  }
  return File[CurrentPos];
}

std::string CheffeLexer::getTextSpan(const std::size_t Begin,
                                     const std::size_t End)
{
  assert(Begin >= 0 && Begin <= End && "");
  return File.Source.substr(Begin, End - Begin);
}

std::string CheffeLexer::lookAhead(const std::size_t NumChars)
{
  assert(CurrentPos < File.Source.size() && "");
  // This std::string constructor handles the case if
  // CurrentPos + NumChars exceeds File.size().
  return File.Source.substr(CurrentPos, NumChars);
}

Token CheffeLexer::getToken()
{
  Token Tok(TokenKind::Unknown);

  Tok.SourceLoc.Begin = CurrentPos;

  // Skip any whitespace.
  while (isspace(peekNextChar()))
  {
    if (peekNextChar() == '\n')
    {
      Tok.Kind = TokenKind::NewLine;
      Tok.SourceLoc.LineNo = LineNumber;
      Tok.SourceLoc.ColumnNo = ColumnNumber;

      getNextChar();
      if (peekNextChar() == '\n')
      {
        getNextChar();
        Tok.Kind = TokenKind::EndOfParagraph;
      }
      else if (IgnoreNewLines)
      {
        continue;
      }

      Tok.SourceLoc.End = CurrentPos;
      return Tok;
    }

    getNextChar();
  }

  Tok.SourceLoc.Begin = CurrentPos;
  Tok.SourceLoc.LineNo = LineNumber;
  Tok.SourceLoc.ColumnNo = ColumnNumber;

  const int Char = getNextChar();

  if (Char == -1)
  {
    Tok.SourceLoc.End = CurrentPos;
    Tok.Kind = TokenKind::EndOfFile;
    return Tok;
  }

  if (isalpha(Char))
  {
    std::string IdentifierString{(char)Char};
    while (isalpha(peekNextChar()))
    {
      IdentifierString += getNextChar();
    }

    Tok.IdentifierString = std::move(IdentifierString);

    Tok.SourceLoc.End = CurrentPos;
    Tok.Kind = TokenKind::Identifier;
    return Tok;
  }

  if (isdigit(Char))
  {
    std::string NumStr = {(char)Char};
    while (isdigit(peekNextChar()))
    {
      NumStr += getNextChar();
    }

    Tok.NumVal = strtod(NumStr.c_str(), 0);

    Tok.SourceLoc.End = CurrentPos;
    Tok.Kind = TokenKind::Number;
    return Tok;
  }

  if (ispunct(Char))
  {
    Tok.SourceLoc.End = CurrentPos;
    Tok.Kind = TokenKind::Unknown;

    switch (Char)
    {
    default:
      break;
    case '.':
      Tok.Kind = TokenKind::FullStop;
      break;
    case '-':
      Tok.Kind = TokenKind::Hyphen;
      break;
    case ':':
      Tok.Kind = TokenKind::Colon;
      break;
    case '(':
      Tok.Kind = TokenKind::OpenBrace;
      break;
    case ')':
      Tok.Kind = TokenKind::CloseBrace;
      break;
    }

    return Tok;
  }

  Tok.SourceLoc.End = CurrentPos;
  Tok.Kind = TokenKind::Unknown;
  return Tok;
}

} // end namespace cheffe
