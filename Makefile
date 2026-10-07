CFLAGS=-Wall -Wextra
LDFLAGS=-lraylib -lm

all: raygui.h spiro
raygui.h:
	wget -O raygui.h https://github.com/raysan5/raygui/raw/refs/tags/5.0/src/raygui.h
spiro: spiro.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
libraylib5.a:
	wget -O libraylib5.a https://pub.krzysckh.org/libraylib5.a
spiro.exe: spiro.c libraylib5.a
	x86_64-w64-mingw32-gcc -mwindows -I/usr/local/include -o $@ spiro.c libraylib5.a -lm -lwinmm -lgdi32 -static
