#include "tokenizer.h"
#include "helper.h"
#include "parser.h"

namespace Poincare {

const CodePoint Tokenizer::nextCodePoint(PopTest popTest, CodePoint context, bool * testResult) {
  CodePoint c = m_decoder.nextCodePoint();
  bool shouldPop = popTest(c, context);
  if (testResult != nullptr) {
    *testResult = shouldPop;
  }
  if (!shouldPop) {
    m_decoder.previousCodePoint();
  }
  return c;
}

bool Tokenizer::canPopCodePoint(const CodePoint c) {
  bool didPop = false;
  nextCodePoint([](CodePoint nextC, CodePoint context) { return nextC == context; }, c, &didPop);
  return didPop;
}

size_t Tokenizer::popWhile(PopTest popTest, CodePoint context) {
  size_t length = 0;
  bool didPop = true;
  while (true) {
    CodePoint c = nextCodePoint(popTest, context, &didPop);
    if (!didPop) {
      break;
    }
    length += UTF8Decoder::CharSizeOfCodePoint(c);
  }
  return length;
}

bool Tokenizer::ShouldAddCodePointToIdentifier(const CodePoint c) {
  return c.isDecimalDigit() || c.isLatinLetter() || c == UCodePointSystem || c == '_' || c == UCodePointDegreeSign || c.isGreekCapitalLetter() || (c.isGreekSmallLetter() && c != UCodePointGreekSmallLetterPi);
}

size_t Tokenizer::popIdentifier() {
  return popWhile([](CodePoint c, CodePoint context){ return ShouldAddCodePointToIdentifier(c); });
}

size_t Tokenizer::popDigits() {
  return popWhile([](CodePoint c, CodePoint context) { return c.isDecimalDigit(); });
}

size_t Tokenizer::popBinaryDigits() {
  return popWhile([](CodePoint c, CodePoint context) { return c.isBinaryDigit(); });
}

size_t Tokenizer::popHexadecimalDigits() {
  return popWhile([](CodePoint c, CodePoint context) { return c.isHexadecimalDigit(); });
}

Token Tokenizer::popNumber() {
  const char * integralPartText = m_decoder.stringPosition();
  size_t integralPartLength = popDigits();

  const char * fractionalPartText = m_decoder.stringPosition();
  size_t fractionalPartLength = 0;

  // Check for binary or hexadecimal number
  if (integralPartLength == 1 && integralPartText[0] == '0') {
    // Save string position if no binary/hexadecimal number
    const char * string = m_decoder.stringPosition();
    // Look for "0b"
    bool binary = canPopCodePoint('b');
    // Look for "0x"
    bool hexa = canPopCodePoint('x');
    if (binary || hexa) {
      const char * binaryOrHexaText = m_decoder.stringPosition();
      size_t binaryOrHexaLength = binary ? popBinaryDigits() : popHexadecimalDigits();
      if (binaryOrHexaLength > 0) {
        Token result(binary ? Token::BinaryNumber : Token::HexadecimalNumber);
        result.setExpression(BasedInteger::Builder(binaryOrHexaText, binaryOrHexaLength, binary ? Integer::Base::Binary : Integer::Base::Hexadecimal));
        return result;
      } else {
        // Rewind before 'b'/'x' letter
        m_decoder.setPosition(string);
      }
    }
  }

  if (canPopCodePoint('.')) {
    fractionalPartText = m_decoder.stringPosition();
    fractionalPartLength = popDigits();
  } else {
    assert(integralPartLength > 0);
  }

  if (integralPartLength == 0 && fractionalPartLength == 0) {
    return Token(Token::Undefined);
  }

  const char * exponentPartText = m_decoder.stringPosition();
  size_t exponentPartLength = 0;
  bool exponentIsNegative = false;
  if (canPopCodePoint(UCodePointLatinLetterSmallCapitalE)) {
    exponentIsNegative = canPopCodePoint('-');
    exponentPartText = m_decoder.stringPosition();
    exponentPartLength = popDigits();
    if (exponentPartLength == 0) {
      return Token(Token::Undefined);
    }
  }

  Token result(Token::Number);
  result.setExpression(Number::ParseNumber(integralPartText, integralPartLength, fractionalPartText, fractionalPartLength, exponentIsNegative, exponentPartText, exponentPartLength));
  return result;
}

Token Tokenizer::popToken() {
  if (m_identifierTokenizer.canStillPop()) {
    // Popping an implicit multiplication between identifiers
    return m_identifierTokenizer.popIdentifier();
  }
  // Skip whitespaces
  while (canPopCodePoint(' ')) {}

  /* Save for later use (since m_decoder.stringPosition() is altered by
   * popNumber, popUnitAndConstant, popIdentifier). */
  const char * start = m_decoder.stringPosition();

  /* If the next code point is the start of a number, we do not want to pop it
   * because popNumber needs this code point. */
  bool nextCodePointIsNeitherDotNorDigit = true;
  const CodePoint c = nextCodePoint([](CodePoint cp, CodePoint context) { return cp != context && !cp.isDecimalDigit(); }, '.', &nextCodePointIsNeitherDotNorDigit);

  // According to c, recognize the Token::Type.
  if (!nextCodePointIsNeitherDotNorDigit) {
    return popNumber();
  }
  if (c == UCodePointGreekSmallLetterPi)
  {
    Token result(Token::Constant);
    result.setString(start, UTF8Decoder::CharSizeOfCodePoint(c));
    return result;
  }
  if (c == '"') {
    Token result(Token::CustomIdentifier);
    result.setString(start, popIdentifier() + 2);
    // The +2 for the two ""
    if (m_decoder.stringPosition()[0] != '"') {
      return Token(Token::Undefined);
    }
    m_decoder.nextCodePoint();
    return result;
  }

  if (ShouldAddCodePointToIdentifier(c))
  {
    // Decoder is one CodePoint ahead of the beginning of the identifier string
    m_decoder.previousCodePoint();
    assert(!m_identifierTokenizer.canStillPop()); // assert we're done with previous tokenization
    const char * startOfIdentifier = m_decoder.stringPosition();
    int identifierLength = popIdentifier();
    m_identifierTokenizer.startTokenization(startOfIdentifier, identifierLength);
    return m_identifierTokenizer.popIdentifier();
  }
  if ('(' <= c && c <= '/') {
    /* Those code points form a contiguous range in the utf-8 code points set,
     * we can thus search faster with this lookup table. */
    constexpr Token::Type typeForCodePoint[] = {
      Token::LeftParenthesis,
      Token::RightParenthesis,
      Token::Times,
      Token::Plus,
      Token::Comma,
      Token::Minus,
      Token::Undefined,
      Token::Slash
    };
    /* The dot code point is the second last of that range, but it is matched
     * before (with popNumber). */
    assert(c != '.');
    return Token(typeForCodePoint[c - '(']);
  }
  switch (c) {
  case UCodePointMultiplicationSign:
  case UCodePointMiddleDot:
    return Token(Token::Times);
  case UCodePointLeftSystemParenthesis:
    return Token(Token::LeftSystemParenthesis);
  case UCodePointRightSystemParenthesis:
    return Token(Token::RightSystemParenthesis);
  case '^': {
    if (canPopCodePoint(UCodePointLeftSystemParenthesis)) {
      return Token(Token::CaretWithParenthesis);
    }
    return Token(Token::Caret);
  }
  case '!':
    return Token(Token::Bang);
  case UCodePointNorthEastArrow:
    return Token(Token::NorthEastArrow);
  case UCodePointSouthEastArrow:
    return Token(Token::SouthEastArrow);
  case '%':
    return Token(Token::Percent);
  case '=': {
    if (m_parsingContext->parsingMethod() == ParsingContext::ParsingMethod::Assignment) {
      /* Change precedence of equal when assigning a function.
       * This ensures that "f(x) = x and 1" is parsed as
       * "f(x) = (x and 1)" and not "(f(x) = x) and 1" */
      return Token(Token::AssignmentEqual);
    }
    return Token(Token::Equal);
  }
  case '>': {
    if (canPopCodePoint('=')) {
      return Token(Token::SuperiorEqual);
    }
    return Token(Token::Superior);
  }
  case UCodePointSuperiorEqual:
    return Token(Token::SuperiorEqual);
  case '<': {
    if (canPopCodePoint('=')) {
      return Token(Token::InferiorEqual);
    }
    return Token(Token::Inferior);
  }
  case UCodePointInferiorEqual:
    return Token(Token::InferiorEqual);
  case '[':
    return Token(Token::LeftBracket);
  case ']':
    return Token(Token::RightBracket);
  case '{':
    return Token(Token::LeftBrace);
  case '}':
    return Token(Token::RightBrace);
  case UCodePointSquareRoot: {
    Token result(Token::ReservedFunction);
    result.setString(start, UTF8Decoder::CharSizeOfCodePoint(c));
    return result;
  }
  case UCodePointEmpty:
    return Token(Token::Empty);
  case UCodePointRightwardsArrow:
    return Token(Token::RightwardsArrow);
  case 0:
    return Token(Token::EndOfStream);
  default:
    return Token(Token::Undefined);
  }
}

}
