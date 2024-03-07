CC=gcc
CFLAGS=-std=c17 -Wall -pedantic
OBJFILES=main.o terminal.o vector.o

all: release

release: CFLAGS += -O3
release: main

main: $(OBJFILES)
	@echo Compiling executable...
	@$(CC) $(CFLAGS) $(OBJFILES) -o main

%.o: src/%.c
	@echo Compiling $@
	@$(CC) $(CFLAGS) -c $< -o $@
