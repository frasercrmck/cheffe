#ifndef CHEFFE_TOKEN
#define CHEFFE_TOKEN

#include <string>
#include <cstring>

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
  Token() : Kind(TokenKind::EndOfFile), Begin(0), End(0), LineNumber(1), ColumnNumber(0), NumVal(0)
  {
  }

  Token(TokenKind Tok) : Kind(Tok), Begin(0), End(0), LineNumber(1), ColumnNumber(0), NumVal(0)
  {
  }

  Token(TokenKind Tok, std::size_t B, std::size_t E, int Val)
      : Kind(Tok), Begin(B), End(E), NumVal(Val)
  {
  }

  // Copy constructor
  Token(const Token& Other)
      : Kind(Other.Kind), Begin(Other.Begin), End(Other.End), LineNumber(Other.LineNumber),
        ColumnNumber(Other.ColumnNumber), NumVal(Other.NumVal),
        IdentifierString(Other.IdentifierString)
  {
  }

  // Move constructor
  Token(Token&& Other)
      : Kind(Other.Kind), Begin(Other.Begin), End(Other.End), LineNumber(Other.LineNumber),
        ColumnNumber(Other.ColumnNumber), NumVal(Other.NumVal),
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
  Token& operator=(const Token& Other)
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
  Token& operator=(Token&& Other)
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

  bool is(TokenKind Tok)
  {
    return Kind == Tok;
  }
  bool is(const std::string& Str)
  {
    return is(TokenKind::Identifier) && !IdentifierString.compare(Str);
  }
  bool is(std::string&& Str)
  {
    return is(TokenKind::Identifier) && !IdentifierString.compare(Str);
  }

  bool isAllOf()
  {
    return true;
  }
  template <typename... Tail>
  bool isAllOf(TokenKind&& Tok, Tail&&... Toks)
  {
    if (isNot(std::forward<TokenKind>(Tok)))
    {
      return false;
    }
    return isAllOf(std::forward<Tail>(Toks)...);
  }
  template <typename... Tail>
  bool isAllOf(std::string&& Str, Tail&&... Toks)
  {
    if (isNot(std::forward<std::string>(Str)))
    {
      return false;
    }
    return isAllOf(std::forward<Tail>(Toks)...);
  }

  bool isNot(TokenKind Tok)
  {
    return Kind != Tok;
  }
  bool isNot(const std::string& Str)
  {
    return !is(Str);
  }
  bool isNot(std::string&& Str)
  {
    return !is(Str);
  }

  bool isNotAnyOf()
  {
    return true;
  }
  template <typename... Tail>
  bool isNotAnyOf(TokenKind&& Tok, Tail&&... Toks)
  {
    if (is(std::forward<TokenKind>(Tok)))
    {
      return false;
    }
    return isNotAnyOf(std::forward<Tail>(Toks)...);
  }
  template <typename... Tail>
  bool isNotAnyOf(std::string&& Str, Tail&&... Toks)
  {
    if (is(std::forward<std::string>(Str)))
    {
      return false;
    }
    return isNotAnyOf(std::forward<Tail>(Toks)...);
  }

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
};

} // end namespace cheffe

#endif
