#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "editor.h"

void editor_insert_char(Editor *e, const char x) {
  if (e->cursor > e->data.count) {
    e->cursor = e->data.count;
  }

  da_append(&e->data, '\0');
  memmove(&e->data.items[e->cursor + 1], &e->data.items[e->cursor],
          e->data.count - e->cursor - 1);

  e->data.items[e->cursor] = x;
  e->cursor += 1;

  editor_recompute_lines(e);
}

void editor_recompute_lines(Editor *e) {
  e->lines.count = 0;

  Line line;
  line.begin = 0;

  for (size_t i = 0; i < e->data.count; ++i) {
    if (e->data.items[i] == '\n') {
      line.end = i;
      da_append(&e->lines, line);
      line.begin = i + 1;
    }
  }

  line.end = e->data.count;
  da_append(&e->lines, line);
}

void editor_backspace(Editor *e) {
  if (e->cursor > e->data.count)
    e->cursor = e->data.count;
  if (e->cursor == 0)
    return;

  memmove(&e->data.items[e->cursor - 1], &e->data.items[e->cursor],
          e->data.count - e->cursor);

  e->data.count -= 1;
  e->cursor -= 1;

  editor_recompute_lines(e);
}

void editor_delete(Editor *e) {
  if (e->cursor >= e->data.count)
    return;

  memmove(&e->data.items[e->cursor], &e->data.items[e->cursor + 1],
          e->data.count - e->cursor - 1);

  e->data.count -= 1;

  editor_recompute_lines(e);
}

Errno editor_save_as(Editor *e, const char *filepath) {
  Errno err = write_entire_file(filepath, e->data.items, e->data.count);
  if (err != 0)
    return err;
  e->filepath.count = 0;
  sb_append_cstr(&e->filepath, filepath);
  sb_append_null(&e->filepath);

  return 0;
}

Errno editor_save(const Editor *e) {
  assert(e->filepath.count > 0);
  return write_entire_file(e->filepath.items, e->data.items, e->data.count);
}

Errno editor_load_from_file(Editor *e, const char *filepath) {
  e->data.count = 0;

  Errno err = read_entire_file(filepath, &e->data);
  if (err != 0)
    return err;

  e->cursor = 0;

  editor_recompute_lines(e);

  e->filepath.count = 0;
  sb_append_cstr(&e->filepath, filepath);
  sb_append_null(&e->filepath);

  return 0;
}

size_t editor_cursor_row(const Editor *e) {
  assert(e->lines.count > 0);

  for (size_t row = 0; row < e->lines.count; ++row) {
    Line line = e->lines.items[row];
    if (line.begin <= e->cursor && e->cursor <= line.end) {
      return row;
    }
  }

  return e->lines.count - 1;
}

void editor_move_line_up(Editor *e) {
  size_t cursor_row = editor_cursor_row(e);
  size_t cursor_col = e->cursor - e->lines.items[cursor_row].begin;
  if (cursor_row > 0) {
    Line prev_line = e->lines.items[cursor_row - 1];
    size_t prev_line_size = prev_line.end - prev_line.begin;
    if (cursor_col > prev_line_size)
      cursor_col = prev_line_size;

    e->cursor = prev_line.begin + cursor_col;
  }
}

void editor_move_line_down(Editor *e) {
  size_t cursor_row = editor_cursor_row(e);
  size_t cursor_col = e->cursor - e->lines.items[cursor_row].begin;
  if (cursor_row < e->lines.count - 1) {
    Line next_line = e->lines.items[cursor_row + 1];
    size_t next_line_size = next_line.end - next_line.begin;
    if (cursor_col > next_line_size)
      cursor_col = next_line_size;

    e->cursor = next_line.begin + cursor_col;
  }
}

void editor_move_char_left(Editor *e) {
  if (e->cursor > 0)
    e->cursor -= 1;
}

void editor_move_char_right(Editor *e) {
  if (e->cursor < e->data.count)
    e->cursor += 1;
}

void editor_render(SDL_Window *window, Free_Glyph_Atlas *atlas,
                   Simple_Renderer *sr, Editor *editor) {
  int w, h;
  SDL_GetWindowSize(window, &w, &h);

  float max_line_len = 0;

  sr->resolution = vec2f(w, h);
  sr->time = (float)SDL_GetTicks() / 1000.0f;

  {
    simple_renderer_set_shader(sr, SHADER_COLOR);

    if (editor->selection) {
      Vec4f sel_color = vec4f(.25, .25, .25, 1);
      for (size_t row = 0; row < editor->lines.count; ++row) {
        size_t sb_c = editor->sel_begin;
        size_t se_c = editor->cursor;

        if (sb_c > se_c) {
          SWAP(size_t, sb_c, se_c);
        }

        Line line_c = editor->lines.items[row];

        if (sb_c < line_c.begin) {
          sb_c = line_c.begin;
        }

        if (se_c > line_c.end) {
          se_c = line_c.end;
        }

        if (sb_c <= se_c) {
          Vec2f sb_s = vec2f(0, -(float)row * FREE_GLYPH_FONT_SIZE);
          free_glyph_atlas_measure_line_sized(atlas,
                                              editor->data.items + line_c.begin,
                                              sb_c - line_c.begin, &sb_s);
          Vec2f se_s = sb_s;
          free_glyph_atlas_measure_line_sized(
              atlas, editor->data.items + sb_c, se_c - sb_c, &se_s);

          simple_renderer_solid_rect(
              sr, sb_s, vec2f(se_s.x - sb_s.x, FREE_GLYPH_FONT_SIZE),
              sel_color);
        }
      }
    }
    simple_renderer_flush(sr);

    simple_renderer_set_shader(sr, SHADER_TEXT);
    for (size_t row = 0; row < editor->lines.count; ++row) {
      Line line = editor->lines.items[row];

      Vec2f begin = vec2f(0, -(float)row * FREE_GLYPH_FONT_SIZE);
      Vec2f end = begin;

      free_glyph_atlas_render_line_sized(
          atlas, sr, editor->data.items + line.begin, line.end - line.begin,
          &end, vec4fs(1));
      float line_len = fabsf(end.x - begin.x);
      if (line_len > max_line_len) {
        max_line_len = line_len;
      }
    }

    simple_renderer_flush(sr);
  }

  Vec2f cursor_pos = vec2fs(0);
  {
    size_t cursor_row = editor_cursor_row(editor);
    Line line = editor->lines.items[cursor_row];

    size_t cursor_col = editor->cursor - line.begin;

    cursor_pos.y = -(float)cursor_row * FREE_GLYPH_FONT_SIZE;
    cursor_pos.x = free_glyph_atlas_cursor_pos(
        atlas, editor->data.items + line.begin, line.end - line.begin,
        vec2f(0, cursor_pos.y), cursor_col);
  }

  simple_renderer_set_shader(sr, SHADER_COLOR);
  {
    float CURSOR_WIDTH = 5.0f;
    Uint32 CURSOR_BLINK_THRESHOLD = 500;
    Uint32 CURSOR_BLINK_PERIOD = 1000;
    Uint32 t = SDL_GetTicks() - editor->last_stroke;

    if (t < CURSOR_BLINK_THRESHOLD || t / CURSOR_BLINK_PERIOD % 2 != 0) {
      simple_renderer_solid_rect(
          sr, cursor_pos, vec2f(CURSOR_WIDTH, FREE_GLYPH_FONT_SIZE), vec4fs(1));
      // simple_renderer_image_rect(sr, cursor_pos,
      // vec2f(CURSOR_WIDTH, FREE_GLYPH_FONT_SIZE));
    }

    simple_renderer_flush(sr);
  }

  {
    float target_scale = 3;
    if (max_line_len > 1000) {
      max_line_len = 1000;
    }
    if (max_line_len > 0) {
      target_scale = fmaxf((w - 300) / (max_line_len * 1.5), 0.5);
    }
    if (target_scale > 3) {
      target_scale = 3;
    }

    sr->camera_vel =
        vec2f_mul(vec2f_sub(cursor_pos, sr->camera_pos), vec2fs(2));
    sr->camera_scale_vel = (target_scale - sr->camera_scale) * 2;

    sr->camera_pos = vec2f_add(sr->camera_pos,
                               vec2f_mul(sr->camera_vel, vec2fs(DELTA_TIME)));

    sr->camera_scale = sr->camera_scale + sr->camera_scale_vel * DELTA_TIME;
  }
}

void editor_update_selection(Editor *e, bool shift) {
  if (shift) {
    if (!e->selection) {
      e->selection = true;
      e->sel_begin = e->cursor;
    }
  } else {
    if (e->selection) {
      e->selection = false;
    }
  }
}