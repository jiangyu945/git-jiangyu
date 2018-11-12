BIN = ff_decode
IDIR = include/
SRC = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRC))

LDIR = lib
CFLAGS = -I$(IDIR) -g
LIBS = -L$(LDIR) -lavcodec -lavutil -lavformat -lswresample -lx264

CC = /usr/local/arm/arm-2009q3/bin/arm-none-linux-gnueabi-gcc

#$@：所有的目标文件
#$^：所有的依赖文件

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)


.PHONY: clean

clean:
	rm -rf $(OBJS) $(BIN)
