CC=gcc
CFLAGS=-std=c99 -D_POSIX_C_SOURCE=199309L -pthread
SRC=./src
BIN=./bin
INC=-I/home/cluster/local/include -I.
LIBS=/home/cluster/local/lib64/libGPI2-dbg.la

all: $(BIN)/one_sided_comm

$(BIN):
	mkdir -p $(BIN)

$(BIN)/one_sided_comm: $(BIN)/one_sided_comm.o
	libtool --mode=link gcc -pthread -o $@ $^ $(LIBS)

$(BIN)/%.o: $(SRC)/%.c Makefile | $(BIN)
	$(CC) $(CFLAGS) $(INC) $< -c -o $@

.PHONY clean:
	rm bin/*
