
all:
	gcc main.c -o main -O3

debug:
	gcc main.c -o main -Og -ggdb
