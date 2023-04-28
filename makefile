CC       = gcc
CPPFLAGS =
CFLAGS   = -Wall -Wextra -Wno-implicit-fallthrough -std=c17 -O2
LDFLAGS  =

.PHONY: all clean

all: game

game: game.o game_example.o
game.o: game.c game.h
game_example.o: game_example.c game.h

clean:
	rm -f *.o game.exe game
