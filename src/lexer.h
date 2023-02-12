#ifndef __NIJI_LEXER_H
#define __NIJI_LEXER_H

#include "la.h"
#include "free_glyph.h"
#include <stdlib.h>

typedef enum {
  TOKEN_END = 0,
  TOKEN_INVALID,
  
  TOKEN_PREPROP,
  TOKEN_SYMBOL,
  TOKEN_OPEN_PAREN,
  TOKEN_CLOSE_PAREN,
  TOKEN_OPEN_BRACE,
  TOKEN_CLOSE_BRACE,
  TOKEN_OPEN_BRACKET,
  TOKEN_CLOSE_BRACKET,

  TOKEN_SEMICOLON,
  TOKEN_KEYWORD,
  TOKEN_SINGLE_COMMENT,
  TOKEN_STRING,
    
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
  Free_Glyph_Atlas *atlas;
  
  const char *content;
  size_t content_len;
  size_t cursor;
  size_t line;
  size_t bol;
  float x;
} Lexer;

Lexer lexer_new(Free_Glyph_Atlas *atlas, const char *content, size_t content_len);
Token lexer_next(Lexer *l);

const char *token_kind_name(Token_Kind kind);
#endif // __NIJI_LEXER_H
