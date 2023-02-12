#include "lexer.h"

#include <stdbool.h>
#include "common.h"

Lexer lexer_new(const char *content, size_t content_len) {
  Lexer l = {0};
  l.content = content;
  l.content_len = content_len;

  return l;
}

Token lexer_next(Lexer *l) {
  Token token = {0};

  UNIMPLEMENTED("lexer_next");
  return token;
}
