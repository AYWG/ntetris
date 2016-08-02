CC=gcc
CFLAGS=-lncurses -lpthread
DEPS = ntetris.h
OBJ = ntetris_main.o ntetris_gui.o ntetris_game.o ntetris_threads.o
EXEDIR = ~/bin

all: $(EXEDIR)/ntetris

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXEDIR)/ntetris: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)