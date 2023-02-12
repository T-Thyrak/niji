#include "file_browser.h"

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
  return 0;
}