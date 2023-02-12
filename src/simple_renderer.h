#ifndef __NIJI_SIMPLE_RENDERER_H
#define __NIJI_SIMPLE_RENDERER_H

#define GLEW_STATIC
#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include "la.h"

typedef enum {
  UNIFORM_SLOT_TIME = 0,
  UNIFORM_SLOT_RESOLUTION,
  UNIFORM_SLOT_CAMERA_POS,
  UNIFORM_SLOT_CAMERA_SCALE,
  UNIFORM_SLOT_CURSOR_POS,
  UNIFORM_SLOT_CURSOR_HEIGHT,
  UNIFORM_SLOT_LAST_STROKE,
  COUNT_UNIFORM_SLOTS,
} Uniform_Slot;

typedef enum {
  SIMPLE_VERTEX_ATTR_POSITION = 0,
  SIMPLE_VERTEX_ATTR_COLOR,
  SIMPLE_VERTEX_ATTR_UV,
  COUNT_SIMPLE_VERTEX_ATTRS
} Simple_Vertex_Attr;

typedef struct {
  Vec2f position;
  Vec4f color;
  Vec2f uv;
} Simple_Vertex;

#define SIMPLE_VERTICES_CAP (3 * 1024 * 1024)

typedef enum {
  SHADER_COLOR = 0,
  SHADER_IMAGE,
  SHADER_TEXT,
  SHADER_EPIC,
  COUNT_SIMPLE_SHADERS,
} Simple_Shader;

typedef struct {
  GLuint vao;
  GLuint vbo;
  GLuint programs[COUNT_SIMPLE_SHADERS];
  Simple_Shader current_shader;

  GLint uniforms[COUNT_UNIFORM_SLOTS];

  Simple_Vertex vertices[SIMPLE_VERTICES_CAP];
  size_t vertices_count;

  Vec2f resolution;
  float time;

  Vec2f camera_pos;
  float camera_scale;
  float camera_scale_vel;
  Vec2f camera_vel;
} Simple_Renderer;

void simple_renderer_init(Simple_Renderer *sr);
void simple_renderer_flush(Simple_Renderer *sr);
void simple_renderer_set_shader(Simple_Renderer *sr, Simple_Shader shader);

void simple_renderer_vertex(Simple_Renderer *sr, Vec2f p, Vec4f c, Vec2f uv);
void simple_renderer_triangle(Simple_Renderer *sr, Vec2f p0, Vec2f p1, Vec2f p2,
                              Vec4f c0, Vec4f c1, Vec4f c2, Vec2f uv0,
                              Vec2f uv1, Vec2f uv2);
void simple_renderer_quad(Simple_Renderer *sr, Vec2f p0, Vec2f p1, Vec2f p2,
                          Vec2f p3, Vec4f c0, Vec4f c1, Vec4f c2, Vec4f c3,
                          Vec2f uv0, Vec2f uv1, Vec2f uv2, Vec2f uv3);
void simple_renderer_solid_rect(Simple_Renderer *sr, Vec2f p, Vec2f s, Vec4f c);
void simple_renderer_image_rect(Simple_Renderer *sr, Vec2f p, Vec2f s,
                              Vec2f uvp, Vec2f uvs, Vec4f c);
void simple_renderer_clear(Simple_Renderer *sr);
void simple_renderer_sync(Simple_Renderer *sr);
void simple_renderer_draw(Simple_Renderer *sr);

#endif // __NIJI_SIMPLE_RENDERER_H
