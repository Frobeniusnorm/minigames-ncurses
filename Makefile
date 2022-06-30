#CC = gcc -Werror -Wpedantic -Wall -c
CC = gcc -Werror -g -c
CFLAGS = -lncursesw -lm
minigames: build/snake.o build/tetris.o build/main.o build/pacman.o build/graph.o build/save.o | build
	gcc -o minigames build/snake.o build/tetris.o build/pacman.o build/graph.o build/save.o build/main.o $(CFLAGS)

build/snake.o: snake/snake.c snake/snake.h
	$(CC) -o build/snake.o snake/snake.c

build/tetris.o: tetris/tetris.c tetris/tetris.h
	$(CC) -o build/tetris.o tetris/tetris.c

build/pacman.o: pacman/pacman.c pacman/pacman.h pacman/graph.h build/graph.o
	$(CC) -o build/pacman.o pacman/pacman.c

build/graph.o: pacman/graph.h pacman/graph.c
	$(CC) -o build/graph.o pacman/graph.c

build/save.o: savefiles/save.h savefiles/save.c
	$(CC) -o build/save.o savefiles/save.c

build/main.o: main.c snake/snake.h tetris/tetris.h
	$(CC) -o build/main.o main.c

clean:
	rm build/*

build:
	mkdir build
