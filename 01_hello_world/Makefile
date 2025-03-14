CC=gcc
CFLAGS=-std=c99 -D_POSIX_C_SOURCE=199309L -pthread
SRC=./src
BIN=./bin
INC=-I/home/cluster/local/include -I.
LIBS=/home/cluster/local/lib64/libGPI2.la

all: $(BIN)/hello_world

$(BIN)/hello_world: $(BIN)/hello_world.o
	libtool --mode=link gcc -pthread -o $@ $^ /home/cluster/local/lib64/libGPI2-dbg.la

$(BIN)/%.o: $(SRC)/%.c Makefile
	$(CC) $(CFLAGS) $(INC) $< -c -o $@

.PHONY clean:
	rm bin/*
