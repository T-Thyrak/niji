#ifndef __NIJI_FILE_BROWSER_H
#define __NIJI_FILE_BROWSER_H

#include "common.h"

typedef struct {
  Files files;
  size_t cursor;
} File_Browser;

Errno fb_open_dir(File_Browser *fb, const char *dirpath);

#endif // __NIJI_FILE_BROWSER_H