CC=gcc
CFLAGS=-lncurses -lpthread -g
DEPS = ntetris.h
OBJ = ntetris_main.o ntetris_gui.o ntetris_game.o ntetris_threads.o ntetris_client.o
EXEDIR = ~/bin

all: $(EXEDIR)/ntetris $(EXEDIR)/ntetris_server

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXEDIR)/ntetris: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(EXEDIR)/ntetris_server: ntetris_server.o
	$(CC) -o $@ $^