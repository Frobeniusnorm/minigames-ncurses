CC = gcc -Werror -Wpedantic -Wall -c
CFLAGS = -lncursesw
minigames: build/snake.o build/tetris.o build/main.o build/pacman.o
	gcc -o minigames build/snake.o build/tetris.o build/pacman.o build/main.o $(CFLAGS)

build/snake.o: snake/snake.c snake/snake.h
	$(CC) -o build/snake.o snake/snake.c

build/tetris.o: tetris/tetris.c tetris/tetris.h
	$(CC) -o build/tetris.o tetris/tetris.c

build/pacman.o: pacman/pacman.c pacman/pacman.h
	$(CC) -o build/pacman.o pacman/pacman.c

build/main.o: main.c snake/snake.h tetris/tetris.h
	$(CC) -o build/main.o main.c
