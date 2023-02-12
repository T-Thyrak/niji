#ifndef __NIJI_COMMON_H
#define __NIJI_COMMON_H

#include <stdio.h>
#include <stdlib.h>

#include "la.h"
#include <stdint.h>

typedef int Errno;

#define UNHEX(color)                                                           \
  ((color) >> 8 * 0) & 0xFF, ((color) >> 8 * 1) & 0xFF,                        \
      ((color) >> 8 * 2) & 0xFF, ((color) >> 8 * 3) & 0xFF

#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768
#define FPS 60
#define DELTA_TIME (1.0f / FPS)

#define ASCII_DISPLAY_LOW 32
#define ASCII_DISPLAY_HIGH 126

#define TAB_SIZE 2

#define SWAP(T, a, b)                                                          \
  do {                                                                         \
    T t = a;                                                                   \
    a = b;                                                                     \
    b = t;                                                                     \
  } while (0)

#define UNIMPLEMENTED(...)                                                     \
  do {                                                                         \
    fprintf(stderr, "%s:%d: UNIMPLEMENTED: %s\n", __FILE__, __LINE__,          \
            __VA_ARGS__);                                                      \
    exit(1);                                                                   \
  } while (false)

#define UNREACHABLE(...)                                                       \
  do {                                                                         \
    fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__,            \
            __VA_ARGS__);                                                      \
    exit(1);                                                                   \
  } while (false)

#define UNUSED(var) (void)(var)

#define return_defer(value)                                                    \
  do {                                                                         \
    result = (value);                                                          \
    goto defer;                                                                \
  } while (0)

#define DA_INIT_CAP 256

#define da_append(da, item)                                                    \
  do {                                                                         \
    if ((da)->count >= (da)->capacity) {                                       \
      (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity * 2; \
      (da)->items =                                                            \
          realloc((da)->items, (da)->capacity * sizeof(*(da)->items));         \
      assert((da)->items != NULL && "Buy more RAM lol.");                      \
    }                                                                          \
    (da)->items[(da)->count++] = (item);                                       \
  } while (0)

#define da_append_many(da, new_items, new_items_count)                         \
  do {                                                                         \
    if ((da)->count + new_items_count > (da)->capacity) {                      \
      if ((da)->capacity == 0) {                                               \
        (da)->capacity = DA_INIT_CAP;                                          \
      }                                                                        \
      while ((da)->count + new_items_count > (da)->capacity) {                 \
        (da)->capacity *= 2;                                                   \
      }                                                                        \
      (da)->items =                                                            \
          realloc((da)->items, (da)->capacity * sizeof(*(da)->items));         \
      assert((da)->items != NULL && "Buy more RAM lol");                       \
    }                                                                          \
    memcpy((da)->items + (da)->count, new_items,                               \
           new_items_count * sizeof(*(da)->items));                            \
    (da)->count += new_items_count;                                            \
  } while (0)

char *temp_strdup(const char *s);
void temp_reset(void);

typedef struct {
  char *items;
  size_t count;
  size_t capacity;
} String_Builder;

#define sb_append_buf da_append_many
#define sb_append_cstr(sb, cstr)                                               \
  do {                                                                         \
    size_t n = strlen(cstr);                                                   \
    da_append_many(sb, cstr, n);                                               \
  } while (0)

#define sb_append_null(sb) da_append_many(sb, "", 1)

typedef struct {
  const char **items;
  size_t count;
  size_t capacity;
} Files;

Errno read_entire_file(const char *filepath, String_Builder *sb);
Errno read_entire_dir(const char *dirpath, Files *files);
Errno write_entire_file(const char *filepath, const char *buf, size_t buf_size);

Vec4f hex_to_vec4f(uint32_t color);

#endif // __NIJI_COMMON_H
