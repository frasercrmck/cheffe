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

struct SourceLocation
{
public:
  friend class CheffeLexer;

private:
  std::size_t Begin;
  std::size_t End;

  unsigned LineNo;
  unsigned ColumnNo;

public:
  SourceLocation() : Begin(0), End(0), LineNo(1), ColumnNo(0)
  {
  }

  SourceLocation(const std::size_t B, const std::size_t E)
      : Begin(B), End(E), LineNo(1), ColumnNo(0)
  {
  }

  std::size_t getBegin() const
  {
    return Begin;
  }
  std::size_t getEnd() const
  {
    return End;
  }

  std::size_t getLength() const
  {
    return End - Begin;
  }

  unsigned getLineNo() const
  {
    return LineNo;
  }
  unsigned getColumnNo() const
  {
    return ColumnNo;
  }
};

class Token
{
public:
  friend class CheffeLexer;

private:
  TokenKind Kind;

  // Position information
  SourceLocation SourceLoc;

  int NumVal;
  std::string IdentifierString;

public:
  // Constructors
  Token() : Kind(TokenKind::EndOfFile), SourceLoc(), NumVal(0)
  {
  }

  Token(TokenKind Tok) : Kind(Tok), SourceLoc(), NumVal(0)
  {
  }

  Token(TokenKind Tok, std::size_t B, std::size_t E, int Val)
      : Kind(Tok), SourceLoc(B, E), NumVal(Val)
  {
  }

  Token(std::string Str)
      : Kind(TokenKind::Identifier), SourceLoc(), NumVal(0),
        IdentifierString(Str)
  {
  }

  Token(const char *Str)
      : Kind(TokenKind::Identifier), SourceLoc(), NumVal(0),
        IdentifierString(Str)
  {
  }

  // Copy constructor
  Token(const Token &Other)
      : Kind(Other.Kind), SourceLoc(Other.SourceLoc), NumVal(Other.NumVal),
        IdentifierString(Other.IdentifierString)
  {
  }

  // Move constructor
  Token(Token &&Other)
      : Kind(Other.Kind), SourceLoc(Other.SourceLoc), NumVal(Other.NumVal),
        IdentifierString(std::move(Other.IdentifierString))

  {
    Other.Kind = TokenKind::Unknown;
    Other.SourceLoc = SourceLocation();
    Other.NumVal = 0;
  }

  // Copy assignment operator
  Token &operator=(const Token &Other)
  {
    Kind = Other.Kind;
    SourceLoc = Other.SourceLoc;
    IdentifierString = Other.IdentifierString;
    NumVal = Other.NumVal;
    return *this;
  }

  // Move assignment operator
  Token &operator=(Token &&Other)
  {
    Kind = Other.Kind;
    SourceLoc = Other.SourceLoc;
    IdentifierString = std::move(Other.IdentifierString);
    NumVal = Other.NumVal;

    Other.Kind = TokenKind::Unknown;
    Other.SourceLoc = SourceLocation();
    Other.NumVal = 0;
    return *this;
  }

  //==== is() ====//
  bool is(TokenKind Tok) const
  {
    return Kind == Tok;
  }
  bool is(const std::string &Str) const
  {
    return is(TokenKind::Identifier) && !IdentifierString.compare(Str);
  }
  bool is(std::string &&Str) const
  {
    return is(TokenKind::Identifier) && !IdentifierString.compare(Str);
  }
  //==== is() ====//

  //==== isNot() ====//
  bool isNot(TokenKind Tok) const
  {
    return Kind != Tok;
  }
  bool isNot(const std::string &Str) const
  {
    return !is(Str);
  }
  bool isNot(std::string &&Str) const
  {
    return !is(Str);
  }
  //==== isNot() ====//

  //==== isAnyOf() ====//
  bool isAnyOf() const
  {
    return false;
  }
  template <typename... Tail>
  bool isAnyOf(TokenKind &&Tok, Tail &&... Toks) const
  {
    if (is(std::forward<TokenKind>(Tok)))
    {
      return true;
    }
    return isAnyOf(std::forward<Tail>(Toks)...);
  }
  template <typename... Tail>
  bool isAnyOf(std::string &&Str, Tail &&... Toks) const
  {
    if (is(std::forward<std::string>(Str)))
    {
      return true;
    }
    return isAnyOf(std::forward<Tail>(Toks)...);
  }
  //==== isAnyOf() ====//

  //==== isNotAnyOf() ====//
  bool isNotAnyOf() const
  {
    return true;
  }
  template <typename... Tail>
  bool isNotAnyOf(TokenKind &&Tok, Tail &&... Toks) const
  {
    if (is(std::forward<TokenKind>(Tok)))
    {
      return false;
    }
    return isNotAnyOf(std::forward<Tail>(Toks)...);
  }
  template <typename... Tail>
  bool isNotAnyOf(std::string &&Str, Tail &&... Toks) const
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

  SourceLocation getSourceLoc() const
  {
    return SourceLoc;
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
