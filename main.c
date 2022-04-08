#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include "snake/snake.h"
int main(){
  srand(time(0));
	initscr();
  if(!has_colors()){
    endwin();
		printf("Your terminal does not support colors!\n");
		exit(-1);
	}
  start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE); //make keys work
	curs_set(0); //hide cursor

  runSnake(0);

  endwin();
}
