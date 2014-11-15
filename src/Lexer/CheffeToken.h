#ifndef CHEFFE_TOKEN
#define CHEFFE_TOKEN

#include <string>
#include <iostream>

namespace cheffe
{

class CheffeLexer;

enum class TokenKind
{
  EndOfFile,
  NewLine,
  Identifier,
  Number,
  FullStop,
  Hyphen,
  Colon,
  OpenBrace,
  CloseBrace,
  EndOfParagraph,
  Unknown,
  Count
};

static std::string TokenKindToString(const TokenKind Kind)
{
  switch (Kind)
  {
  default:
    return "";
  case TokenKind::EndOfFile:
    return "'EndOfFile (\\0)'";
  case TokenKind::NewLine:
    return "'NewLine (\\n)'";
  case TokenKind::Identifier:
    return "'Identifier'";
  case TokenKind::Number:
    return "'Number'";
  case TokenKind::FullStop:
    return "'.'";
  case TokenKind::Hyphen:
    return "'-'";
  case TokenKind::Colon:
    return "':'";
  case TokenKind::OpenBrace:
    return "'('";
  case TokenKind::CloseBrace:
    return "')'";
  case TokenKind::EndOfParagraph:
    return "'EndOfParagraph (\\n\\n)'";
  }
}

static std::ostream &operator<<(std::ostream &stream, const TokenKind &Kind)
{
  stream << TokenKindToString(Kind);
  return stream;
}

class Token
{
public:
  friend class CheffeLexer;

private:
  TokenKind Kind;

  // Position information
  std::size_t Begin;
  std::size_t End;

  unsigned LineNumber;
  unsigned ColumnNumber;

  int NumVal;
  std::string IdentifierString;

public:
  // Constructors
  Token()
      : Kind(TokenKind::EndOfFile), Begin(0), End(0), LineNumber(1),
        ColumnNumber(0), NumVal(0)
  {
  }

  Token(TokenKind Tok)
      : Kind(Tok), Begin(0), End(0), LineNumber(1), ColumnNumber(0), NumVal(0)
  {
  }

  Token(TokenKind Tok, std::size_t B, std::size_t E, int Val)
      : Kind(Tok), Begin(B), End(E), NumVal(Val)
  {
  }

  Token(std::string Str)
      : Kind(TokenKind::Identifier), Begin(0), End(0), LineNumber(1),
        ColumnNumber(0), NumVal(0), IdentifierString(Str)
  {
  }

  Token(const char *Str)
      : Kind(TokenKind::Identifier), Begin(0), End(0), LineNumber(1),
        ColumnNumber(0), NumVal(0), IdentifierString(Str)
  {
  }

  // Copy constructor
  Token(const Token &Other)
      : Kind(Other.Kind), Begin(Other.Begin), End(Other.End),
        LineNumber(Other.LineNumber), ColumnNumber(Other.ColumnNumber),
        NumVal(Other.NumVal), IdentifierString(Other.IdentifierString)
  {
  }

  // Move constructor
  Token(Token &&Other)
      : Kind(Other.Kind), Begin(Other.Begin), End(Other.End),
        LineNumber(Other.LineNumber), ColumnNumber(Other.ColumnNumber),
        NumVal(Other.NumVal),
        IdentifierString(std::move(Other.IdentifierString))

  {
    Other.Kind = TokenKind::Unknown;
    Other.Begin = 0;
    Other.End = 0;
    Other.LineNumber = 0;
    Other.ColumnNumber = 0;
    Other.NumVal = 0;
  }

  // Copy assignment operator
  Token &operator=(const Token &Other)
  {
    Kind = Other.Kind;
    Begin = Other.Begin;
    End = Other.End;
    LineNumber = Other.LineNumber;
    ColumnNumber = Other.ColumnNumber;
    IdentifierString = Other.IdentifierString;
    NumVal = Other.NumVal;
    return *this;
  }

  // Move assignment operator
  Token &operator=(Token &&Other)
  {
    Kind = Other.Kind;
    Begin = Other.Begin;
    End = Other.End;
    LineNumber = Other.LineNumber;
    ColumnNumber = Other.ColumnNumber;
    IdentifierString = std::move(Other.IdentifierString);
    NumVal = Other.NumVal;

    Other.Kind = TokenKind::Unknown;
    Other.Begin = 0;
    Other.End = 0;
    Other.LineNumber = 0;
    Other.ColumnNumber = 0;
    Other.NumVal = 0;
    return *this;
  }

  //==== is() ====//
  bool is(TokenKind Tok)
  {
    return Kind == Tok;
  }
  bool is(const std::string &Str)
  {
    return is(TokenKind::Identifier) && !IdentifierString.compare(Str);
  }
  bool is(std::string &&Str)
  {
    return is(TokenKind::Identifier) && !IdentifierString.compare(Str);
  }
  //==== is() ====//

  //==== isNot() ====//
  bool isNot(TokenKind Tok)
  {
    return Kind != Tok;
  }
  bool isNot(const std::string &Str)
  {
    return !is(Str);
  }
  bool isNot(std::string &&Str)
  {
    return !is(Str);
  }
  //==== isNot() ====//

  //==== isAnyOf() ====//
  bool isAnyOf()
  {
    return false;
  }
  template <typename... Tail> bool isAnyOf(TokenKind &&Tok, Tail &&... Toks)
  {
    if (is(std::forward<TokenKind>(Tok)))
    {
      return true;
    }
    return isAnyOf(std::forward<Tail>(Toks)...);
  }
  template <typename... Tail>
  bool isAnyOf(std::string &&Str, Tail &&... Toks)
  {
    if (is(std::forward<std::string>(Str)))
    {
      return true;
    }
    return isAnyOf(std::forward<Tail>(Toks)...);
  }
  //==== isAnyOf() ====//

  //==== isNotAnyOf() ====//
  bool isNotAnyOf()
  {
    return true;
  }
  template <typename... Tail> bool isNotAnyOf(TokenKind &&Tok, Tail &&... Toks)
  {
    if (is(std::forward<TokenKind>(Tok)))
    {
      return false;
    }
    return isNotAnyOf(std::forward<Tail>(Toks)...);
  }
  template <typename... Tail>
  bool isNotAnyOf(std::string &&Str, Tail &&... Toks)
  {
    if (is(std::forward<std::string>(Str)))
    {
      return false;
    }
    return isNotAnyOf(std::forward<Tail>(Toks)...);
  }
  //==== isNotAnyOf() ====//

  TokenKind getKind() const
  {
    return Kind;
  }

  std::size_t getBegin() const
  {
    return Begin;
  }

  std::size_t getEnd() const
  {
    return End;
  }

  std::size_t getLen() const
  {
    return End - Begin;
  }

  unsigned getLineNumber() const
  {
    return LineNumber;
  }

  unsigned getColumnNumber() const
  {
    return ColumnNumber;
  }

  std::string getIdentifierString() const
  {
    return IdentifierString;
  }

  int getNumVal() const
  {
    return NumVal;
  }

  friend std::ostream &operator<<(std::ostream &stream, const Token &Tok)
  {
    if (Tok.Kind == TokenKind::Identifier)
    {
      stream << Tok.IdentifierString;
      return stream;
    }

    if (Tok.Kind == TokenKind::Number)
    {
      stream << Tok.NumVal;
      return stream;
    }

    stream << Tok.Kind;
    return stream;
  }
};

} // end namespace cheffe

#endif
