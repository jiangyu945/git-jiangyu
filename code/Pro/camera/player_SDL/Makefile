BIN = player_SDL
IDIR = include/SDL2
SRC = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRC))

LDIR = lib
CFLAGS = -I$(IDIR) -g
LIBS = -L$(LDIR) -lSDL2 -lpthread

CC = gcc

#$@：所有的目标文件
#$^：所有的依赖文件

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)


.PHONY: clean

clean:
	rm -rf $(OBJS) $(BIN)
