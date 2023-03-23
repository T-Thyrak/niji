#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#define GLEW_STATIC
#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "common.h"
#include "editor.h"
#include "file_browser.h"
#include "free_glyph.h"
#include "la.h"
#include "lexer.h"
#include "simple_renderer.h"
#include "sv.h"

void MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                     GLsizei length, const GLchar *message,
                     const void *userParam) {
  (void)source;
  (void)id;
  (void)length;
  (void)userParam;

  fprintf(stderr,
          "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity,
          message);
}

static Free_Glyph_Atlas atlas = {0};
static Simple_Renderer sr = {0};
static Editor editor = {0};
static File_Browser fb = {0};

// TODO: display errors reported via flash_error right into the text editor
#define flash_error(...)                                                       \
  do {                                                                         \
    fprintf(stderr, __VA_ARGS__);                                              \
    fprintf(stderr, "\n");                                                     \
  } while (0)

int main(int argc, char **argv) {
  Errno err;

  FT_Library library = {0};

  FT_Error error = FT_Init_FreeType(&library);
  if (error) {
    fprintf(stderr, "ERROR: Could not initialize FreeType2 library\n");
    return 1;
  }

  const char *font_filepath = "./fonts/iosevka-ss04-regular.ttc";
  // const char *font_filepath = "./fonts/VictorMono-Regular.ttf";
  // const char *font_filepath = "./fonts/VictorMono-Italic.ttf";
  // const char *font_filepath = "./fonts/ComicNeue-Regular.ttf";
  // const char *font_filepath = "./fonts/JetBrainsMono-Regular.ttf";

  FT_Face face;
  error = FT_New_Face(library, font_filepath, 0, &face);
  if (error == FT_Err_Unknown_File_Format) {
    fprintf(stderr, "ERROR: `%s` has an unknown format\n", font_filepath);
    return 1;
  } else if (error) {
    fprintf(stderr, "ERROR: could not load file `%s`\n", font_filepath);
    return 1;
  }

  FT_UInt pixel_size = FREE_GLYPH_FONT_SIZE;
  error = FT_Set_Pixel_Sizes(face, 0, pixel_size);
  if (error) {
    fprintf(stderr, "ERROR: could not set pixel size to `%u`\n", pixel_size);
    return 1;
  }

  if (argc > 1) {
    const char *filepath = argv[1];
    err = editor_load_from_file(&editor, filepath);
    if (err != 0 && err != 2) {
      fprintf(stderr, "ERROR: Could not read file %s: %s\n", filepath,
              strerror(err));
      return 1;
    }
    sb_append_cstr(&editor.filepath, filepath);
    sb_append_null(&editor.filepath);
  }

  const char *dirpath = ".";
  err = fb_open_dir(&fb, dirpath);
  if (err != 0) {
    fprintf(stderr, "ERROR: Could not read directory %s: %s\n", dirpath,
            strerror(err));
    return 1;
  }

  if (SDL_Init(SDL_INIT_VIDEO < 0)) {
    fprintf(stderr, "ERROR: Could not initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window = (SDL_CreateWindow(
      "Niji Editor", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED));

  if (window == NULL) {
    fprintf(stderr, "ERROR: Could not create SDL window %s\n", SDL_GetError());
    return 1;
  }

  {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    int major;
    int minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    printf("INFO: GL version %d.%d\n", major, minor);
  }

  if (SDL_GL_CreateContext(window) == NULL) {
    fprintf(stderr, "ERROR: Could not create OpenGL context: %s\n",
            SDL_GetError());
    return 1;
  }

  if (GLEW_OK != glewInit()) {
    fprintf(stderr, "ERROR: Could not initialize GLEW!");
    return 1;
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (GLEW_ARB_debug_output) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, NULL);
  } else {
    fprintf(stderr, "WARNING: GL_ARB_debug_output is not available!");
  }

  Vec4f bg_color = hex_to_vec4f(0x181818ff);

  free_glyph_atlas_init(&atlas, face);

  simple_renderer_init(&sr);

  editor.atlas = &atlas;
  editor_retokenize(&editor);

  bool quit = false;
  bool file_browser = false;
  while (!quit) {
    const Uint32 start = SDL_GetTicks();
    SDL_Event event = {0};
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: {
        quit = true;
      } break;

      case SDL_KEYDOWN: {
        editor.last_stroke = SDL_GetTicks();
        if (file_browser) {
          // TODO: File browser keys
          switch (event.key.keysym.sym) {
          case SDLK_F3: {
            file_browser = false;
          } break;

          case SDLK_UP: {
            if (fb.cursor > 0) {
              fb.cursor -= 1;
            }
          } break;

          case SDLK_DOWN: {
            if (fb.cursor + 1 < fb.files.count) {
              fb.cursor += 1;
            }
          } break;

          case SDLK_RETURN: {
            const char *filepath = fb_filepath(&fb);
            if (filepath) {
              File_Type ft;
              err = type_of_file(filepath, &ft);
              if (err != 0) {
                flash_error("Could not determine type of file %s: %s", filepath,
                            strerror(err));
              } else {
                switch (ft) {
                case FT_DIRECTORY: {
                  err = fb_change_dir(&fb);
                  if (err != 0) {
                    flash_error("Could not change directory to %s: %s",
                                filepath, strerror(err));
                  }
                } break;
                case FT_REGULAR: {
                  // TODO: nag about unsaved changes
                  err = editor_load_from_file(&editor, filepath);
                  if (err != 0) {
                    flash_error("Could not open file %s: %s", filepath,
                                strerror(err));
                  } else {
                    file_browser = false;
                  }
                } break;

                case FT_OTHER: {
                  flash_error("%s is neither a regular file nor a directory. "
                              "Get fucked.",
                              filepath);
                } break;

                default:
                  UNREACHABLE("unknown File_Type");
                }
              }
            }
          } break;
          }
        } else {
          switch (event.key.keysym.sym) {
          case SDLK_F2: {
            if (editor.filepath.count > 0) {
              err = editor_save(&editor);
              if (err != 0) {
                flash_error("Could not save file currently edited: %s",
                            strerror(err));
              }
            } else {
              flash_error("Dunno where to save text.");
            }
          } break;

          case SDLK_F3: {
            file_browser = true;
          } break;

          case SDLK_F5: {
            simple_renderer_reload_shaders(&sr);
          } break;

          case SDLK_BACKSPACE: {
            editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
            editor_backspace(&editor);
          } break;

          case SDLK_DELETE: {
            editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
            editor_delete(&editor);
          } break;

          case SDLK_RETURN: {
            editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
            editor_insert_char(&editor, '\n');
          } break;

          case SDLK_TAB: {
            editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
            for (int i = 0; i < TAB_SIZE; ++i) {
              editor_insert_char(&editor, ' ');
            }
          } break;

          case SDLK_a: {
            if (event.key.keysym.mod & KMOD_CTRL) {
              editor.selection = true;
              editor.sel_begin = 0;
              editor.cursor = editor.data.count;
            }
          } break;

          case SDLK_c: {
            if (event.key.keysym.mod & KMOD_CTRL) {
              editor_clipboard_copy(&editor);
            }
          } break;

          case SDLK_v: {
            if (event.key.keysym.mod & KMOD_CTRL) {
              editor_clipboard_paste(&editor);
            }
          } break;

          case SDLK_UP: {
            editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
            editor_move_line_up(&editor);
          } break;

          case SDLK_DOWN: {
            editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
            editor_move_line_down(&editor);
          } break;

          case SDLK_LEFT: {
            editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
            // editor_move_char_left(&editor);
            if (event.key.keysym.mod & KMOD_CTRL) {
              editor_move_word_left(&editor);
            } else {
              editor_move_char_left(&editor);
            }
          } break;

          case SDLK_RIGHT: {
            editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
            // editor_move_char_right(&editor);
            if (event.key.keysym.mod & KMOD_CTRL) {
              editor_move_word_right(&editor);
            } else {
              editor_move_char_right(&editor);
            }
          } break;
          }
        }
      } break;

      case SDL_TEXTINPUT: {
        if (file_browser) {
        } else {
          const char *text = event.text.text;
          size_t text_len = strlen(text);
          for (size_t i = 0; i < text_len; ++i) {
            editor_insert_char(&editor, text[i]);
          }
        }
      } break;

      case SDL_MOUSEBUTTONDOWN: {
        // TODO OR NEVER: Mouse is utterly broken

        //   Vec2f mouse_click = vec2f((float)event.button.x,
        //   (float)event.button.y); switch (event.button.button) { case
        //   SDL_BUTTON_LEFT: {
        //     Vec2f cursor_click = vec2f_add(
        //         mouse_click, vec2f_sub(camera_pos,
        //         vec2f_div(window_size(window),
        //                                                      vec2fs(2.0f))));
        //     if (cursor_click.x > 0.0f && cursor_click.y > 0.0f) {
        //       editor.cursor_row = (size_t)fmax(
        //           floorf(cursor_click.y / ((float)FONT_CHAR_HEIGHT *
        //           FONT_SCALE)), editor.size);
        //       editor.cursor_col = (size_t)fmax(
        //           floorf(cursor_click.x / ((float)FONT_CHAR_WIDTH *
        //           FONT_SCALE)), editor.lines[editor.cursor_row].size);
        //     }
        //   } break;
        //   }
      } break;
      }
    }

    glClearColor(bg_color.x, bg_color.y, bg_color.z, bg_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    {
      int w, h;
      SDL_GetWindowSize(window, &w, &h);
      glViewport(0, 0, w, h);
    }

    if (file_browser) {
      fb_render(window, &atlas, &sr, &fb);
    } else {
      editor_render(window, &atlas, &sr, &editor);
    }

    SDL_GL_SwapWindow(window);

    const Uint32 duration = SDL_GetTicks() - start;
    const Uint32 delta_time_ms = 1000 / FPS;
    if (duration < delta_time_ms) {
      SDL_Delay(delta_time_ms - duration);
    }
  }

  SDL_Quit();

  return 0;
}
