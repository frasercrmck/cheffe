#ifndef CHEFFE_TOKEN
#define CHEFFE_TOKEN

#include <string>
#include <cstring>

namespace cheffe
{

class CheffeLexer;

enum TokenKind
{
  TokenKindEOF,
  TokenKindNewLine,
  TokenKindIdentifier,
  TokenKindNumber,
  TokenKindFullStop,
  TokenKindHyphen,
  TokenKindColon,
  TokenKindOpenBrace,
  TokenKindCloseBrace,
  TokenKindEndOfParagraph,
  TokenKindUnknown,
  TokenKindCount
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

  int NumVal;
  std::string IdentifierString;

public:
  // Constructors
  Token() : Kind(TokenKindEOF), Begin(0), End(0), NumVal(0)
  {
  }

  Token(TokenKind Tok) : Kind(Tok), Begin(0), End(0), NumVal(0)
  {
  }

  Token(TokenKind Tok, std::size_t B, std::size_t E, int Val)
      : Kind(Tok), Begin(E), End(E), NumVal(Val)
  {
  }

  // Copy constructor
  Token(const Token& Other)
      : Kind(Other.Kind), Begin(Other.Begin), End(Other.End),
        IdentifierString(Other.IdentifierString), NumVal(Other.NumVal)
  {
  }

  // Move constructor
  Token(Token&& Other)
      : Kind(Other.Kind), Begin(Other.Begin), End(Other.End),
        IdentifierString(std::move(Other.IdentifierString)), NumVal(Other.NumVal)
  {
    Other.Kind = TokenKindUnknown;
    Other.Begin = 0;
    Other.End = 0;
    Other.NumVal = 0;
  }

  // Copy assignment operator
  Token& operator=(const Token& Other)
  {
    Kind = Other.Kind;
    Begin = Other.Begin;
    End = Other.End;
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
    IdentifierString = std::move(Other.IdentifierString);
    NumVal = Other.NumVal;

    Other.Kind = TokenKindUnknown;
    Other.Begin = 0;
    Other.End = 0;
    Other.NumVal = 0;
    return *this;
  }


  bool is(TokenKind Tok)
  {
    return Kind == Tok;
  }

  bool isAllOf(TokenKind Tok)
  {
    return is(Tok);
  }
  template <typename ...Tail>
  bool isAllOf(TokenKind&& Tok, Tail&&... Toks)
  {
      if (Kind != Tok)
      {
        return false;
      }
      return isAllOf(std::forward<Tail>(Toks)...);
  }

  bool isNot(TokenKind Tok)
  {
    return Kind != Tok;
  }

  bool isNotAnyOf(TokenKind Tok)
  {
    return isNot(Tok);
  }
  template <typename ...Tail>
  bool isNotAnyOf(TokenKind&& Tok, Tail&&... Toks)
  {
      if (Kind == Tok)
      {
        return false;
      }
      return isNotAnyOf(std::forward<Tail>(Toks)...);
  }

  std::size_t getBegin()
  {
    return Begin;
  }

  std::size_t getEnd()
  {
    return End;
  }

  std::size_t getLen()
  {
    return End - Begin;
  }

  std::string getIdentifierString()
  {
    return IdentifierString;
  }

  int getNumVal()
  {
    return NumVal;
  }

  bool equals(const std::string& Str)
  {
    return is(TokenKindIdentifier) && !IdentifierString.compare(Str);
  }
  bool equals(std::string&& Str)
  {
    return is(TokenKindIdentifier) && !IdentifierString.compare(Str);
  }
};

} // end namespace cheffe

#endif
