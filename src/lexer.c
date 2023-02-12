#include "lexer.h"

#include "common.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
  Token_Kind kind;
  const char *text;
} Literal_Token;

Literal_Token literal_tokens[] = {
    {.text = "(", .kind = TOKEN_OPEN_PAREN},
    {.text = ")", .kind = TOKEN_CLOSE_PAREN},
    {.text = "[", .kind = TOKEN_OPEN_BRACKET},
    {.text = "]", .kind = TOKEN_CLOSE_BRACKET},
    {.text = "{", .kind = TOKEN_OPEN_BRACE},
    {.text = "}", .kind = TOKEN_CLOSE_BRACE},
    {.text = ";", .kind = TOKEN_SEMICOLON},
};

#define literal_tokens_count                                                   \
  (sizeof(literal_tokens) / sizeof(literal_tokens[0]))

const char *keywords[] = {
    "auto",          "break",
    "case",          "char",
    "const",         "continue",
    "default",       "do",
    "double",        "else",
    "enum",          "extern",
    "float",         "for",
    "goto",          "if",
    "int",           "long",
    "register",      "return",
    "short",         "signed",
    "sizeof",        "static",
    "struct",        "switch",
    "typedef",       "union",
    "unsigned",      "void",
    "volatile",      "while",
    "alignas",       "alignof",
    "and",           "and_eq",
    "asm",           "atomic_cancel",
    "atomic_commit", "atomic_noexcept",
    "bitand",        "bitor",
    "bool",          "catch",
    "char16_t",      "char32_t",
    "char8_t",       "class",
    "co_await",      "co_return",
    "co_yield",      "compl",
    "concept",       "const_cast",
    "consteval",     "constexpr",
    "constinit",     "decltype",
    "delete",        "dynamic_cast",
    "explicit",      "export",
    "false",         "friend",
    "inline",        "mutable",
    "namespace",     "new",
    "noexcept",      "not",
    "not_eq",        "nullptr",
    "operator",      "or",
    "or_eq",         "private",
    "protected",     "public",
    "reflexpr",      "reinterpret_cast",
    "requires",      "static_assert",
    "static_cast",   "synchronized",
    "template",      "this",
    "thread_local",  "throw",
    "true",          "try",
    "typeid",        "typename",
    "using",         "virtual",
    "wchar_t",       "xor",
    "xor_eq",
};

#define keywords_count (sizeof(keywords) / sizeof(keywords[0]))

Lexer lexer_new(Free_Glyph_Atlas *atlas, const char *content,
                size_t content_len) {
  Lexer l = {0};
  l.content = content;
  l.content_len = content_len;
  l.atlas = atlas;

  return l;
}

bool lexer_starts_with(Lexer *l, const char *prefix) {
  size_t prefix_len = strlen(prefix);
  if (prefix_len == 0) {
    return true;
  }
  if (l->cursor + prefix_len - 1 >= l->content_len) {
    return false;
  }

  for (size_t i = 0; i < prefix_len; ++i) {
    if (prefix[i] != l->content[l->cursor + i]) {
      return false;
    }
  }

  return true;
}

void lexer_chop_chars(Lexer *l, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    assert(l->cursor < l->content_len);
    char x = l->content[l->cursor];
    l->cursor += 1;
    if (x == '\n') {
      l->line += 1;
      l->bol = l->cursor;
      l->x = 0;
    } else {
      if (l->atlas) {
        size_t glyph_index = x;
        if (glyph_index >= GLYPH_METRICS_CAPACITY) {
          glyph_index = '?';
        }
        Glyph_Metric metric = l->atlas->metrics[glyph_index];
        l->x += metric.ax;
      }
    }
  }
}

void lexer_trim_left(Lexer *l) {
  while (l->cursor < l->content_len && isspace(l->content[l->cursor])) {
    lexer_chop_chars(l, 1);
  }
}

bool is_symbol_start(char x) { return isalpha(x) || x == '_'; }

bool is_symbol(char x) { return isalnum(x) || x == '_'; }

Token lexer_next(Lexer *l) {
  lexer_trim_left(l);
  Token token = {
      .text = &l->content[l->cursor],
  };

  token.position.x = l->x;
  token.position.y = -(float)l->line * FREE_GLYPH_FONT_SIZE;

  if (l->cursor >= l->content_len)
    return token;

  if (l->content[l->cursor] == '"') {
    // TODO: TOKEN_STRING should handle escape sequences
    token.kind = TOKEN_STRING;
    lexer_chop_chars(l, 1);
    while (l->cursor < l->content_len && l->content[l->cursor] != '"' &&
           l->content[l->cursor] != '\n') {
      lexer_chop_chars(l, 1);
    }
    if (l->cursor < l->content_len) {
      lexer_chop_chars(l, 1);
    }
    token.text_len = &l->content[l->cursor] - token.text;
    return token;
  }

  if (l->content[l->cursor] == '#') {
    token.kind = TOKEN_PREPROP;
    while (l->cursor < l->content_len && l->content[l->cursor] != '\n') {
      lexer_chop_chars(l, 1);
    }
    if (l->cursor < l->content_len) {
      lexer_chop_chars(l, 1);
    }
    token.text_len = &l->content[l->cursor] - token.text;
    return token;
  }

  if (lexer_starts_with(l, "//")) {
    token.kind = TOKEN_SINGLE_COMMENT;
    while (l->cursor < l->content_len && l->content[l->cursor] != '\n') {
      lexer_chop_chars(l, 1);
    }
    if (l->cursor < l->content_len) {
      lexer_chop_chars(l, 1);
    }
    token.text_len = &l->content[l->cursor] - token.text;
    return token;
  }

  for (size_t i = 0; i < literal_tokens_count; ++i) {
    if (lexer_starts_with(l, literal_tokens[i].text)) {
      size_t text_len = strlen(literal_tokens[i].text);
      token.kind = literal_tokens[i].kind;
      token.text_len = text_len;
      lexer_chop_chars(l, text_len);
      return token;
    }
  }

  if (is_symbol_start(l->content[l->cursor])) {
    token.kind = TOKEN_SYMBOL;
    while (l->cursor < l->content_len && is_symbol(l->content[l->cursor])) {
      lexer_chop_chars(l, 1);
      token.text_len += 1;
    }
    for (size_t i = 0; i < keywords_count; ++i) {
      size_t keyword_len = strlen(keywords[i]);
      if (keyword_len == token.text_len &&
          memcmp(keywords[i], token.text, keyword_len) == 0) {
        token.kind = TOKEN_KEYWORD;
        break;
      }
    }
    return token;
  }

  lexer_chop_chars(l, 1);
  token.kind = TOKEN_INVALID;
  token.text_len = 1;

  return token;
}

const char *token_kind_name(Token_Kind kind) {
  switch (kind) {
  case TOKEN_END:
    return "end of content";
  case TOKEN_INVALID:
    return "invalid token";
  case TOKEN_PREPROP:
    return "preprocessor directive";
  case TOKEN_OPEN_PAREN:
    return "open parentheses";
  case TOKEN_CLOSE_PAREN:
    return "close parentheses";
  case TOKEN_OPEN_BRACKET:
    return "open brackets";
  case TOKEN_CLOSE_BRACKET:
    return "close brackets";
  case TOKEN_OPEN_BRACE:
    return "open braces";
  case TOKEN_CLOSE_BRACE:
    return "close braces";
  case TOKEN_SEMICOLON:
    return "semicolon";
  case TOKEN_SYMBOL:
    return "symbol";
  case TOKEN_KEYWORD:
    return "keyword";
  case TOKEN_SINGLE_COMMENT:
    return "single line comment";
  default:
    UNREACHABLE("token_kind_name");
  }

  return NULL;
}
