#include "pacman/pacman.h"
#include "safefiles/save.h"
#include "snake/snake.h"
#include "tetris/tetris.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
static const char *choices[] = {"Snake", "Tetris", "Pacman", "Exit"};
static const int num_choices = (sizeof(choices) / sizeof(char *));
static void printMenu(WINDOW *win, int currentChoice) {
  wclear(win);

  box(win, 0, 0);
  for (int i = 0; i < num_choices; i++) {
    if (i == currentChoice)
      wattron(win, A_REVERSE);
    mvwprintw(win, 2 + i, 2, "%s", choices[i]);
    if (i == currentChoice)
      wattroff(win, A_REVERSE);
  }
  wrefresh(win);
}
static void initMenu() {
  int width, height;

  getmaxyx(stdscr, height, width);
  const int sizex = 30, sizey = 10;
  WINDOW *menu =
      newwin(sizey, sizex, height / 2 - sizey / 2, width / 2 - sizex / 2);
  timeout(-1);
  mvprintw(height / 2 - sizey / 2 - 2, width / 2 - 7, "Select a game");
  refresh();
  int currentChoice = 0;
  while (true) {

    printMenu(menu, currentChoice);
    int startedGame = 0;
    switch (getch()) {
    case KEY_UP:
      currentChoice = currentChoice == 0 ? num_choices - 1 : currentChoice - 1;
      break;
    case KEY_DOWN:
      currentChoice = (currentChoice + 1) % num_choices;
      break;
    case 10: {
      int cHigh = 0;
      switch (currentChoice) {
      case 0: {
        runSnake(&cHigh);
        startedGame = 1;
      } break;
      case 1:
        runTetris(&cHigh);
        startedGame = 1;
        break;
      case 2:
        runPacman(&cHigh);
        startedGame = 1;
        break;
      case 3:
        delwin(menu);
        refresh();
        return;
      }
      if (startedGame) {
        clear();
        timeout(-1);
        mvprintw(height / 2 - sizey / 2 - 2, width / 2 - 7, "Select a game");
        refresh();
      }
      break;
    }
    }
  }
}

int main() {
  srand(time(0));
  initscr();
  if (!has_colors()) {
    endwin();
    printf("Your terminal does not support colors!\n");
    exit(-1);
  }
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE); // make keys work
  curs_set(0);          // hide cursor
  initMenu();

  endwin();
}
