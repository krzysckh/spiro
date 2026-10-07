CFLAGS=-Wall -Wextra
LDFLAGS=-lraylib -lm

all: raygui.h spiro
raygui.h:
	wget -O raygui.h https://github.com/raysan5/raygui/raw/refs/tags/5.0/src/raygui.h
spiro: spiro.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
