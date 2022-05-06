#CC = gcc -Werror -Wpedantic -Wall -c
CC = gcc -Werror -c
CFLAGS = -lncursesw -lm
minigames: build/snake.o build/tetris.o build/main.o build/pacman.o build/graph.o | build
	gcc -o minigames build/snake.o build/tetris.o build/pacman.o build/graph.o build/main.o $(CFLAGS)

build/snake.o: snake/snake.c snake/snake.h
	$(CC) -o build/snake.o snake/snake.c

build/tetris.o: tetris/tetris.c tetris/tetris.h
	$(CC) -o build/tetris.o tetris/tetris.c

build/pacman.o: pacman/pacman.c pacman/pacman.h pacman/graph.h build/graph.o
	$(CC) -o build/pacman.o pacman/pacman.c

build/graph.o: pacman/graph.h pacman/graph.c
	$(CC) -o build/graph.o pacman/graph.c

build/main.o: main.c snake/snake.h tetris/tetris.h
	$(CC) -o build/main.o main.c

clean:
	rm build/*

build:
	mkdir build
