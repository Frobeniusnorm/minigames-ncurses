#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include "snake/snake.h"

static const char* choices[] = {"Snake", "Tetris", "Exit"};
static const int num_choices = (sizeof(choices) / sizeof(char*));
static void printMenu(WINDOW* win, int currentChoice){
  wclear(win);

  box(win, 0, 0);
  for(int i = 0; i < num_choices; i++){
    if(i == currentChoice) wattron(win, A_REVERSE);
    mvwprintw(win, 2 + i, 2, "%s", choices[i]);
    if(i == currentChoice) wattroff(win, A_REVERSE);
  }
  wrefresh(win);
}
static void initMenu(){
  int width, height;
  getmaxyx(stdscr, height, width);
  const int sizex = 30, sizey = 10;
  WINDOW* menu = newwin(sizey, sizex, height/2 - sizey/2, width/2 - sizex/2);
  timeout(-1);
  int currentChoice = 0;
  while(true){
    mvprintw(height/2 - sizey/2 - 2, width/2 - 7, "Select a game");
    printMenu(menu, currentChoice);
    refresh();
    switch(getch()){
      case KEY_UP:
        currentChoice = currentChoice == 0 ? num_choices - 1 : currentChoice - 1;
        break;
      case KEY_DOWN:
        currentChoice = (currentChoice + 1) % num_choices;
        break;
      case 10:
        switch(currentChoice){
          case 0:
            runSnake(0);
            clear();
            timeout(-1);
            break;
          case 1:
            break;
          case 2:
            delwin(menu);
            refresh();
            return;
        }
        break;
    }
  }
}

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
  initMenu();

  endwin();
}
