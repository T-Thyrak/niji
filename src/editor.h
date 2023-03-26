#ifndef __NIJI_EDITOR_H
#define __NIJI_EDITOR_H

#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#include "common.h"
#include "free_glyph.h"
#include "lexer.h"
#include "simple_renderer.h"

typedef struct {
  size_t begin;
  size_t end;
} Line;

typedef struct {
  Line *items;
  size_t count;
  size_t capacity;
} Lines;

typedef struct {
  Token *items;
  size_t count;
  size_t capacity;
} Tokens;

typedef struct {
  Free_Glyph_Atlas *atlas;

  String_Builder data;
  Lines lines;
  Tokens tokens;
  String_Builder filepath;

  bool searching;
  String_Builder search;

  bool selection;
  size_t sel_begin;
  size_t cursor;

  Uint32 last_stroke;

  String_Builder clipboard;
} Editor;

Errno editor_save_as(Editor *editor, const char *filepath);
Errno editor_save(const Editor *editor);
Errno editor_load_from_file(Editor *editor, const char *filepath);

void editor_retokenize(Editor *editor);

void editor_insert_char(Editor *editor, const char ch);
void editor_insert_buf(Editor *editor, char *buf, size_t buf_len);
void editor_backspace(Editor *editor);
void editor_delete(Editor *editor);

size_t editor_cursor_row(const Editor *editor);

void editor_move_line_up(Editor *editor);
void editor_move_line_down(Editor *editor);
void editor_move_char_left(Editor *editor);
void editor_move_char_right(Editor *editor);
void editor_move_word_left(Editor *e);
void editor_move_word_right(Editor *e);
void editor_move_to_begin(Editor *e);
void editor_move_to_end(Editor *e);
void editor_move_to_line_begin(Editor *e);
void editor_move_to_line_end(Editor *e);
void editor_move_paragraph_up(Editor *e);
void editor_move_paragraph_down(Editor *e);

void editor_update_selection(Editor *e, bool shift);

void editor_start_search(Editor *e);
void editor_stop_search(Editor *e);
bool editor_search_matches_at(Editor *e, size_t pos);

void editor_render(SDL_Window *window, Free_Glyph_Atlas *atlas,
                   Simple_Renderer *sr, Editor *e);

void editor_clipboard_copy(Editor *editor);
void editor_clipboard_paste(Editor *editor);

#endif // __NIJI_EDITOR_H
