#ifndef __NIJI_FILE_BROWSER_H
#define __NIJI_FILE_BROWSER_H

#include "common.h"
#include "free_glyph.h"

#include <SDL2/SDL.h>

typedef struct {
  Files files;
  size_t cursor;
  String_Builder dirpath;
  String_Builder filepath;
} File_Browser;

Errno fb_open_dir(File_Browser *fb, const char *dirpath);
Errno fb_change_dir(File_Browser *fb);
void fb_render(SDL_Window *window, Free_Glyph_Atlas *atlas, Simple_Renderer *sr,
               const File_Browser *fb);
const char *fb_filepath(File_Browser *fb);

#endif // __NIJI_FILE_BROWSER_H