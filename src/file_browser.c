#include "file_browser.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

static int file_cmp(const void *pa, const void *pb) {
  const char *a = *(const char **)pa;
  const char *b = *(const char **)pb;
  return strcmp(a, b);
}

Errno fb_open_dir(File_Browser *fb, const char *dirpath) {
  fb->files.count = 0;
  fb->cursor = 0;

  Errno err = read_entire_dir(dirpath, &fb->files);
  if (err != 0) {
    return err;
  }
  qsort(fb->files.items, fb->files.count, sizeof(*fb->files.items), file_cmp);

  fb->dirpath.count = 0;
  sb_append_cstr(&fb->dirpath, dirpath);
  sb_append_null(&fb->dirpath);

  return 0;
}

Errno fb_change_dir(File_Browser *fb) {
  assert(fb->dirpath.count > 0 &&
         "You need to call fb_open_dir() before fb_change_dir()");
  assert(fb->dirpath.items[fb->dirpath.count - 1] == '\0');
  if (fb->cursor >= fb->files.count)
    return 0;

  const char *dirname = fb->files.items[fb->cursor];
  fb->dirpath.count -= 1;

  // TODO: fb_change_dir() does not support . and .. properly
  sb_append_cstr(&fb->dirpath, "/");
  sb_append_cstr(&fb->dirpath, dirname);
  sb_append_null(&fb->dirpath);

  fb->files.count = 0;
  fb->cursor = 0;

  Errno err = read_entire_dir(fb->dirpath.items, &fb->files);

  if (err != 0) {
    return err;
  }

  qsort(fb->files.items, fb->files.count, sizeof(*fb->files.items), file_cmp);
  return 0;
}

const char *fb_filepath(File_Browser *fb) {
  assert(fb->dirpath.count > 0 &&
         "You need to call fb_open_dir() before fb_filepath()");
  assert(fb->dirpath.items[fb->dirpath.count - 1] == '\0');

  if (fb->cursor >= fb->files.count)
    return NULL;

  fb->filepath.count = 0;
  sb_append_buf(&fb->filepath, fb->dirpath.items, fb->dirpath.count - 1);
  sb_append_buf(&fb->filepath, "/", 1);
  sb_append_cstr(&fb->filepath, fb->files.items[fb->cursor]);
  sb_append_null(&fb->filepath);

  return fb->filepath.items;
}

void fb_render(SDL_Window *window, Free_Glyph_Atlas *atlas, Simple_Renderer *sr,
               const File_Browser *fb) {

  Vec2f cursor_pos = vec2f(0, -(float)fb->cursor * FREE_GLYPH_FONT_SIZE);
  int w, h;
  SDL_GetWindowSize(window, &w, &h);

  float max_line_len = 0.f;

  sr->resolution = vec2f(w, h);
  sr->time = (float)SDL_GetTicks() / 1000.0f;

  // Render cursor
  simple_renderer_set_shader(sr, SHADER_COLOR);
  if (fb->cursor < fb->files.count) {
    const Vec2f begin = cursor_pos;
    Vec2f end = begin;
    free_glyph_atlas_measure_line_sized(atlas, fb->files.items[fb->cursor],
                                        strlen(fb->files.items[fb->cursor]),
                                        &end);
    simple_renderer_solid_rect(sr, begin,
                               vec2f(end.x - begin.x, FREE_GLYPH_FONT_SIZE),
                               vec4f(.25, .25, .25, 1));
  }

  simple_renderer_flush(sr);

  // Render text
  simple_renderer_set_shader(sr, SHADER_EPIC);
  for (size_t row = 0; row < fb->files.count; ++row) {
    const Vec2f begin = vec2f(0, -(float)row * FREE_GLYPH_FONT_SIZE);
    Vec2f end = begin;

    free_glyph_atlas_render_line_sized(atlas, sr, fb->files.items[row],
                                       strlen(fb->files.items[row]), &end,
                                       vec4fs(0));
    float line_len = fabsf(end.x - begin.x);
    if (line_len > max_line_len) {
      max_line_len = line_len;
    }
  }

  simple_renderer_flush(sr);

  // Update camera
  {
    if (max_line_len > 1000) {
      max_line_len = 1000;
    }
    if (max_line_len <= 0) {
      max_line_len = 1;
    }

    float target_scale = (float)w / 3 / (max_line_len * 0.75);
    Vec2f target = cursor_pos;
    float offset = 0.0f;

    if (target_scale > 2) {
      target_scale = 2;
    } else {
      offset = cursor_pos.x - (float)w / 3 / sr->camera_scale;
      if (offset < 0) offset = 0;
      target = vec2f((float)w / 3 / sr->camera_scale + offset, cursor_pos.y);
    }

    sr->camera_vel =
        vec2f_mul(vec2f_sub(target, sr->camera_pos), vec2fs(2));
    sr->camera_scale_vel = (target_scale - sr->camera_scale) * 2;

    sr->camera_pos = vec2f_add(sr->camera_pos,
                               vec2f_mul(sr->camera_vel, vec2fs(DELTA_TIME)));

    sr->camera_scale = sr->camera_scale + sr->camera_scale_vel * DELTA_TIME;
  }
}
