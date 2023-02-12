#ifndef __NIJI_FREE_GLYPH_H
#define __NIJI_FREE_GLYPH_H

#include <stdbool.h>
#include <stdlib.h>

#include "la.h"
#define GLEW_STATIC
#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "simple_renderer.h"

#define FREE_GLYPH_BUFFER_CAP (1024 * 1024)
#define FREE_GLYPH_FONT_SIZE 48

typedef struct {
  float ax; // advance.x
  float ay; // advance.y

  float bw; // bitmap.width;
  float bh; // bitmap.rows;

  float bl; // bitmap_left;
  float bt; // bitmap_top;

  float tx; // x offset of glyph in texture coordinates
} Glyph_Metric;

#define GLYPH_METRICS_CAPACITY 128

typedef struct {
  FT_UInt atlas_width;
  FT_UInt atlas_height;

  GLuint glyphs_texture;

  Glyph_Metric metrics[GLYPH_METRICS_CAPACITY];

} Free_Glyph_Atlas;

void free_glyph_atlas_init(Free_Glyph_Atlas *atlas, FT_Face face);

float free_glyph_atlas_cursor_pos(const Free_Glyph_Atlas *atlas,
                                  const char *text, size_t text_size, Vec2f pos,
                                  size_t col);
void free_glyph_atlas_measure_line_sized(Free_Glyph_Atlas *atlas,
                                         const char *text, size_t text_size,
                                         Vec2f *pos);
void free_glyph_atlas_render_line_sized(Free_Glyph_Atlas *atlas,
                                        Simple_Renderer *sr, const char *text,
                                        size_t text_size, Vec2f *pos,
                                        Vec4f color);
#endif // __NIJI_FREE_GLYPH_H
