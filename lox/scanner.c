#include "scanner.h"

#include <stdio.h>
#include <string.h>

#include "common.h"

void initScanner(Scanner* scanner, const char* source) {
  scanner->start = source;
  scanner->current = source;
  scanner->line = 1;

  scanner->numParens = 0;
}

static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

static bool isAtEnd(Scanner* scanner) {
  return *scanner->current == '\0';
}

static char advance(Scanner* scanner) {
  scanner->current++;
  return scanner->current[-1];
}

static char peek(Scanner* scanner) {
  return *scanner->current;
}

static char peekNext(Scanner* scanner) {
  if (isAtEnd(scanner)) {
    return '\0';
  }
  return scanner->current[1];
}

static bool match(Scanner* scanner, char expected) {
  if (isAtEnd(scanner)) {
    return false;
  }
  if (*scanner->current != expected) {
    return false;
  }
  scanner->current++;
  return true;
}

static Token makeToken(Scanner* scanner, TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner->start;
  token.length = (int)(scanner->current - scanner->start);
  token.line = scanner->line;
  return token;
}

static Token makeTokenWith(Scanner* scanner, TokenType type, const char* start, int length) {
    Token token;
    token.type = type;
    token.start = start;
    token.length = length;
    token.line = scanner->line;
    return token;
}

static Token errorToken(Scanner* scanner, const char* message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = scanner->line;
  return token;
}

static void skipWhitespace(Scanner* scanner) {
  for (;;) {
    char c = peek(scanner);
    switch (c) {
      case ' ':
      case '\r':
      case '\t': advance(scanner); break;
      case '\n':
        scanner->line++;
        advance(scanner);
        break;
      case '/':
        if (peekNext(scanner) == '/') {
            // A comment goes until the end of the line.
            while (peek(scanner) != '\n' && !isAtEnd(scanner)) advance(scanner);
        } else {
            return;
        }
        break;
      default: return;
    }
  }
}

static TokenType checkKeyword(Scanner* scanner, int start, int length,
    const char* rest, TokenType type) {
  if (scanner->current - scanner->start == start + length &&
      memcmp(scanner->start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_IDENTIFIER;
}

static TokenType identifierType(Scanner* scanner) {
  switch (scanner->start[0]) {
    case '_': 
        if (scanner->current - scanner->start == 1) return TOKEN_UNDERSCORE;
        break;
    case 'a': return checkKeyword(scanner, 1, 2, "nd", TOKEN_AND);
    case 'b': return checkKeyword(scanner, 1, 4, "reak", TOKEN_BREAK);
    case 'c': 
        if (scanner->current - scanner->start > 1) {
            switch (scanner->start[1]) {
            case 'l': return checkKeyword(scanner, 2, 3, "ass", TOKEN_CLASS);
            case 'o': return checkKeyword(scanner, 2, 6, "ntinue", TOKEN_CONTINUE);
            }
        }
        break;
    case 'e': return checkKeyword(scanner, 1, 3, "lse", TOKEN_ELSE);
    case 'f':
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'a':
            return checkKeyword(scanner, 2, 3, "lse", TOKEN_FALSE);
          case 'o': return checkKeyword(scanner, 2, 1, "r", TOKEN_FOR);
          case 'u': return checkKeyword(scanner, 2, 1, "n", TOKEN_FUN);
        }
      }
      break;
    case 'i': return checkKeyword(scanner, 1, 1, "f", TOKEN_IF);
    case 'n': return checkKeyword(scanner, 1, 2, "il", TOKEN_NIL);
    case 'm': return checkKeyword(scanner, 1, 4, "atch", TOKEN_MATCH);
    case 'o': return checkKeyword(scanner, 1, 1, "r", TOKEN_OR);
    case 'p': return checkKeyword(scanner, 1, 4, "rint", TOKEN_PRINT);
    case 'r': return checkKeyword(scanner, 1, 5, "eturn", TOKEN_RETURN);
    case 's': return checkKeyword(scanner, 1, 4, "uper", TOKEN_SUPER);
    case 't':
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'h':
            return checkKeyword(scanner, 2, 2, "is", TOKEN_THIS);
          case 'r':
            return checkKeyword(scanner, 2, 2, "ue", TOKEN_TRUE);
        }
      }
      break;
    case 'v': return checkKeyword(scanner, 1, 2, "ar", TOKEN_VAR);
    case 'w': return checkKeyword(scanner, 1, 4, "hile", TOKEN_WHILE);
  }

  return TOKEN_IDENTIFIER;
}

static Token identifier(Scanner* scanner) {
  while (isAlpha(peek(scanner)) || isDigit(peek(scanner))) {
    advance(scanner);
  }
  return makeToken(scanner, identifierType(scanner));
}

static Token number(Scanner* scanner) {
  while (isDigit(peek(scanner))) {
    advance(scanner);
  }

  // Look for a fractional part.
  if (peek(scanner) == '.' && isDigit(peekNext(scanner))) {
    // Consume the ".".
    advance(scanner);

    while (isDigit(peek(scanner))) {
      advance(scanner);
    }
  }

  return makeToken(scanner, TOKEN_NUMBER);
}

static Token rawString(Scanner* scanner) {
    //consume the second and third "
    advance(scanner);
    advance(scanner);

    int charsDiscard = 2;

    //if there's a newline immediately after, 
    //discard it so it's not part of the literal
    if (peek(scanner) == '\r') {
        charsDiscard++;
        advance(scanner);
    }
    if (peek(scanner) == '\n') {
        charsDiscard++;
        advance(scanner);
    }

    while (!isAtEnd(scanner)) {
        char c = advance(scanner);
        char c1 = peek(scanner);
        char c2 = peekNext(scanner);

        if (c == '\r' || c == '\n') {
            scanner->line++;
        }

        if (c == '"' && c1 == '"' && c2 == '"') break;
    }

    if (isAtEnd(scanner)) {
        return errorToken(scanner, "Unterminated string.");
    }

    //consume the second and third "
    advance(scanner);
    advance(scanner);

    return makeTokenWith(scanner, TOKEN_STRING, scanner->start + charsDiscard, 
        (int)(scanner->current - scanner->start - charsDiscard - 2));
}

static Token string(Scanner* scanner) {
  if (peek(scanner) == '"' && peekNext(scanner) == '"') return rawString(scanner);
  
  while (!isAtEnd(scanner)) {
    if (peek(scanner) == '\n') {
      scanner->line++;
    }
    else if (peek(scanner) == '\\' && (peekNext(scanner) == '"' || peekNext(scanner) == '\\')) {
        advance(scanner);
    }
    else if (peek(scanner) == '\\' && peekNext(scanner) == '%') {
        advance(scanner);
    }
    else if (peek(scanner) == '%') { // interpolated 
        if (scanner->numParens < MAX_INTERPOLATION_NESTING) {
            if (peekNext(scanner) != '(') 
                return errorToken(scanner, "Expect '(' after '%'.");

            advance(scanner); // %
            advance(scanner); // (

            scanner->parens[scanner->numParens++] = 1;

            return makeTokenWith(scanner, TOKEN_INTERPOLATION, scanner->start, 
                (int)(scanner->current - scanner->start - 1));
        }
        return errorToken(scanner, "Interpolation may only nest %d levels deep.");
    }
    else if (peek(scanner) == '"') {
        break;
    }
    advance(scanner);
  }

  if (isAtEnd(scanner)) {
    return errorToken(scanner, "Unterminated string.");
  }

  // The closing quote.
  advance(scanner);
  return makeToken(scanner, TOKEN_STRING);
}

static Token skipBlockComment(Scanner* scanner) {
    int nesting = 1;
    while (nesting > 0) {
        if (isAtEnd(scanner))
            return errorToken(scanner, "Block comment aren't paired.");

        if (peek(scanner) == '/' && peekNext(scanner) == '*') {
            advance(scanner);
            advance(scanner);
            nesting++;
            continue;
        }

        if (peek(scanner) == '*' && peekNext(scanner) == '/') {
            advance(scanner);
            advance(scanner);
            nesting--;
            continue;
        }

        if (peek(scanner) == '\n') scanner->line++;
        advance(scanner);
    }

    return scanToken(scanner);
}

static bool isHexDigit(Scanner* scanner) {
    char c = peek(scanner);
    if (c >= '0' && c <= '9') return true;
    if (c >= 'a' && c <= 'f') return true;
    if (c >= 'A' && c <= 'F') return true;

    return false;
}

static Token hexNumber(Scanner* scanner) {
    while (isHexDigit(scanner)) advance(scanner);

    return makeToken(scanner, TOKEN_NUMBER);
}

static bool isOctDigit(Scanner* scanner) {
    char c = peek(scanner);
    if (c >= '0' && c <= '7') return true;

    return false;
}

static Token octNumber(Scanner* scanner) {
    while (isOctDigit(scanner)) advance(scanner);

    return makeToken(scanner, TOKEN_NUMBER);
}

static bool isBinDigit(Scanner* scanner) {
    char c = peek(scanner);
    if (c == '0' || c == '1') return true;

    return false;
}

static Token binNumber(Scanner* scanner) {
    while (isBinDigit(scanner)) advance(scanner);

    return makeToken(scanner, TOKEN_NUMBER);
}

Token scanToken(Scanner* scanner) {
  skipWhitespace(scanner);
  scanner->start = scanner->current;

  if (isAtEnd(scanner)) {
    return makeToken(scanner, TOKEN_EOF);
  }

  char c = advance(scanner);

  switch (c) {
    case '(': 
        // If we are inside an interpolated expression, count the unmatched "(".
        if (scanner->numParens > 0) scanner->parens[scanner->numParens - 1]++;

        return makeToken(scanner, TOKEN_LEFT_PAREN);
    case ')': 
        // If we are inside an interpolated expression, count the ")".
        if (scanner->numParens > 0 && --scanner->parens[scanner->numParens - 1] == 0)
        {
            // This is the final ")", so the interpolation expression has ended.
            // This ")" now begins the next section of the template string.
            scanner->numParens--;
            return string(scanner);
        }

        return makeToken(scanner, TOKEN_RIGHT_PAREN);
    case '{': return makeToken(scanner, TOKEN_LEFT_BRACE);
    case '}': return makeToken(scanner, TOKEN_RIGHT_BRACE);
    case '[': return makeToken(scanner, TOKEN_LEFT_SQUARE);
    case ']': return makeToken(scanner, TOKEN_RIGHT_SQUARE);
    case ';': return makeToken(scanner, TOKEN_SEMICOLON);
    case ':': return makeToken(scanner, TOKEN_COLON);
    case ',': return makeToken(scanner, TOKEN_COMMA);
    case '.': return makeToken(scanner, TOKEN_DOT);
    case '-': 
        if (match(scanner, '-'))
            return makeToken(scanner, TOKEN_MINUS_MINUS);
        else if (match(scanner, '='))
            return makeToken(scanner, TOKEN_MINUS_EQUAL);
        else
            return makeToken(scanner, TOKEN_MINUS);
    case '%': return makeToken(scanner, TOKEN_PERCENT);
    case '+': 
        if (match(scanner, '+'))
            return makeToken(scanner, TOKEN_PLUS_PLUS);
        else if (match(scanner, '='))
            return makeToken(scanner, TOKEN_PLUS_EQUAL);
        else
            return makeToken(scanner, TOKEN_PLUS);
    case '/': 
        if (match(scanner, '*'))
            return skipBlockComment(scanner);
        else if (match(scanner, '='))
            return makeToken(scanner, TOKEN_SLASH_EQUAL);
        else
            return makeToken(scanner, TOKEN_SLASH);
    case '*': 
      return makeToken(
          scanner, match(scanner, '=') ? TOKEN_STAR_EQUAL : TOKEN_STAR);
    case '?':
        if (match(scanner, '.'))
            return makeToken(scanner, TOKEN_QUESTION_DOT);
        else if (match(scanner, ':'))
            return makeToken(scanner, TOKEN_QUESTION_COLON);
        else
            return makeToken(scanner, TOKEN_QUESTION);
    case '!':
      return makeToken(
          scanner, match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
      return makeToken(scanner,
          match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
      return makeToken(
          scanner, match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
      return makeToken(scanner,
          match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"': return string(scanner);
    case '0': 
        if (match(scanner, 'x') || match(scanner, 'X'))
            return hexNumber(scanner);
        else if (match(scanner, 'c') || match(scanner, 'C'))
            return octNumber(scanner);
        else if (match(scanner, 'b') || match(scanner, 'B'))
            return binNumber(scanner);
        else
            return number(scanner);
  }

  if (isAlpha(c)) {
      return identifier(scanner);
  }
  if (isDigit(c)) {
      return number(scanner);
  }

  return errorToken(scanner, "Unexpected character.");
}
