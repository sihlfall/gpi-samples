C=gcc
CFLAGS=-std=c99 -pthread -Wall -pedantic
SRC=./src
BIN=./bin
INC=-I/home/cluster/local/include -I.
LIBS=/home/cluster/local/lib64/libGPI2.la

all: $(BIN) $(BIN)/server $(BIN)/client

$(BIN):
	mkdir -p bin

$(BIN)/server: $(BIN)/server.o $(BIN)/clientserver.o
	$(C) $(CFLAGS) -o $@ $^ -lucp

$(BIN)/client: $(BIN)/client.o $(BIN)/clientserver.o
	$(C) $(CFLAGS) -o $@ $^ -lucp

$(BIN)/%.o: $(SRC)/%.c Makefile
	$(C) $(CFLAGS) $< -c -o $@

.PHONY clean:
	rm bin/*
