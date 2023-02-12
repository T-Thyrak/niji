#include "common.h"

#define ARENA_IMPLEMENTATION
#include "arena.h"
#ifdef _WIN32
#define MINIRENT_IMPLEMENTATION
#include "minirent.h"
#else
#include <dirent.h>
#endif // _WIN32
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Arena temp_arena = {0};

char *temp_strdup(const char *s) {
  size_t n = strlen(s);
  char *ds = arena_alloc(&temp_arena, n + 1);
  memcpy(ds, s, n);
  ds[n] = '\0';
  return ds;
}

static Errno file_size(FILE *file, size_t *size) {
  long saved = ftell(file);

  if (saved < 0)
    return errno;
  if (fseek(file, 0, SEEK_END) < 0)
    return errno;

  long result = ftell(file);
  if (result < 0)
    return errno;
  if (fseek(file, saved, SEEK_SET) < 0)
    return errno;

  *size = (size_t)result;
  return 0;
}

Errno read_entire_file(const char *filepath, String_Builder *sb) {
  Errno result = 0;

  FILE *f = NULL;
  f = fopen(filepath, "r");
  if (f == NULL)
    return_defer(errno);

  size_t size;
  Errno err = file_size(f, &size);

  if (err != 0)
    return_defer(errno);

  if (sb->capacity < size) {
    sb->capacity = size;
    sb->items = realloc(sb->items, sb->capacity * sizeof(*sb->items));
    assert(sb->items != NULL && "Buy more RAM lol");
  }

  fread(sb->items, size, sizeof(char), f);
  if (ferror(f))
    return_defer(errno);
  sb->count = size;

defer:
  if (f)
    fclose(f);
  return result;
}

Errno read_entire_dir(const char *dirpath, Files *files) {
  Errno result = 0;

  DIR *dir = NULL;
  dir = opendir(dirpath);

  if (dir == NULL) {
    return_defer(errno);
  }

  errno = 0;
  struct dirent *ent = readdir(dir);
  while (ent != NULL) {
    da_append(files, temp_strdup(ent->d_name));
    ent = readdir(dir);
  }

  if (errno != 0) {
    return_defer(errno);
  }

defer:
  if (dir)
    closedir(dir);
  return result;
}

void temp_reset(void) { arena_reset(&temp_arena); }

Errno write_entire_file(const char *filepath, const char *buf,
                        size_t buf_size) {
  Errno result = 0;
  FILE *f = NULL;

  f = fopen(filepath, "wb");
  if (f == NULL)
    return_defer(errno);

  fwrite(buf, sizeof(char), buf_size, f);
  if (ferror(f))
    return_defer(errno);

defer:
  if (f)
    fclose(f);
  return result;
}

Vec4f hex_to_vec4f(uint32_t color) {
  Vec4f result;
  uint32_t r = (color >> (3 * 8)) & 0xFF;
  uint32_t g = (color >> (2 * 8)) & 0xFF;
  uint32_t b = (color >> (1 * 8)) & 0xFF;
  uint32_t a = (color >> (0 * 8)) & 0xFF;

  result.x = r / 255.0f;
  result.y = g / 255.0f;
  result.z = b / 255.0f;
  result.w = a / 255.0f;

  return result;
}