#ifndef __NIJI_LEXER_H
#define __NIJI_LEXER_H

#include "la.h"

#include <stdlib.h>

typedef enum {
  TOKEN_HASH = 0,
  TOKEN_SYMBOL,
  TOKEN_INVALID,
    
  TOKEN_END,
  COUNT_TOKENS,
} Token_Kind;

// typedef struct {
//   const char *filepath;
//   size_t row;
//   size_t col;
// } Loc;

typedef struct {
  Token_Kind kind;
  const char *text;
  size_t text_len;
  Vec2f position;
} Token;

typedef struct {
  const char *content;
  size_t content_len;
  size_t cursor;
  size_t line;
  size_t bol;
} Lexer;

Lexer lexer_new(const char *content, size_t content_len);
Token lexer_next(Lexer *l);

#endif // __NIJI_LEXER_H
