PKGS=sdl2 glew freetype2
CFLAGS=-Wall -Wextra -std=c11 -pedantic `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm
SRCS=src/main.c src/la.c src/editor.c src/free_glyph.c src/simple_renderer.c src/common.c src/file_browser.c src/lexer.c

niji: $(SRCS)
	$(CC) -ggdb $(CFLAGS) -o niji $(SRCS) $(LIBS)

release: $(SRCS)
	$(CC) -O3 $(CFLAGS) -o niji $(SRCS) $(LIBS)