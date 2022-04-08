CC = gcc -Werror -Wpedantic -Wall -c
CFLAGS = -lncurses
minigames: build/snake.o build/main.o
	gcc -o minigames build/snake.o build/main.o $(CFLAGS)

build/snake.o: snake/snake.c snake/snake.h
	$(CC) -o build/snake.o snake/snake.c

build/main.o: main.c snake/snake.h
	$(CC) -o build/main.o main.c
