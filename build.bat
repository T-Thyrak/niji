@echo off

set CFLAGS=/W4 /WX /wd4996 /std:c11 /FC /TC /Zi /nologo
set INCLUDES=/I dependencies\SDL2\include /I dependencies\GLFW\include /I depedencies\GLEW\include
set LIBS=dependencies\SDL2\lib\x64\SDL2.lib ^
		 dependencies\SDL2\lib\x64\SDL2main.lib ^
		 dependencies\GLFW\lib\glfw3.lib ^
		 dependencies\GLEW\lib\glew32s.lib ^
		 opengl32.lib User32.lib Gdi32.lib Shell32.lib

cl.exe %CFLAGS% %INCLUDES% /Feniji src\main.c src\la.c src\editor.c src\free_glyph.c src\simple_renderer.c src\common.c src\file_browser.c src\lexer.c /link %LIBS% -SUBSYSTEM:windows
