#include "Lexer/CheffeLexer.h"

#include <cassert>

namespace cheffe
{

void CheffeLexer::setSourceFile(const CheffeSourceFile &SrcFile)
{
  File = SrcFile;
}

void CheffeLexer::setIgnoreNewLines(const bool Ignore)
{
  IgnoreNewLines = Ignore;
}

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

// Peeks at the next char from the input stream. Can optionally ignore any
// space or tab characters while peeking.
int CheffeLexer::peekNextChar(IgnoreWhiteSpace Ignore) const
{
  int NextChar = -1;
  std::size_t Pos = CurrentPos;
  do
  {
    if (static_cast<std::size_t>(Pos) >= File.Source.size())
    {
      return -1;
    }
    NextChar = File[Pos++];
  } while (Ignore == IgnoreWhiteSpace::True &&
           (NextChar == ' ' || NextChar == '\t'));

  return NextChar;
}

std::string CheffeLexer::getTextSpan(const std::size_t Begin,
                                     const std::size_t End) const
{
  assert(Begin >= 0 && Begin <= End && "");
  return File.Source.substr(Begin, End - Begin);
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
      if (peekNextChar(IgnoreWhiteSpace::True) == '\n')
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
