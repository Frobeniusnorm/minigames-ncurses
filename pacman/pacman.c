#include "pacman.h"
#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#define WIDTH 62
#define HEIGHT 27

static const char map[HEIGHT][WIDTH] = {
  "a-----------------------------ba-----------------------------b",
  "| . . . . . . . . . . . . . . || . . . . . . . . . . . . . . |",
  "| . a-------b . a---------b . || . a---------b . a-------b . |",
  "| . c-------d . c---------d . cd . c---------d . c-------d . |",
  "| . . . . . . . . . . . . . .    . . . . . . . . . . . . . . |",
  "| . a-------b . a-b . a----------------b . a-b . a-------b . |",
  "| . c-------d . | | . c-----b    a-----d . | | . c-------d . |",
  "| . . . . . . . | | . . . . |    | . . . . | | . . . . . . . |",
  "c-----------b . | c-----b . |    | . a-----d | . a-----------d",
  "            | . | a-----d . c----d . c-----b | . |            ",
  "            | . | |                        | | . |            ",
  "            | . | |    a-----    -----b    | | . |            ",
  "------------d . c-d    |              |    c-d . c------------",
  ". . . . . . . .        |              |        . . . . . . . .",
  "------------b . a-b    c--------------d    a-b . a------------",
  "            | . | |                        | | . |            ",
  "            | . | |   a----------------b   | | . |            ",
  "a-----------d . c-d   c-----b    a-----d   c-d . c-----------b",
  "| . . . . . . . . . . . . . |    | . . . . . . . . . . . . . |",
  "| . a-------b . a-------b . |    | . a-------b . a-------b . |",
  "| . c-----b | . c-------d . c----d . c-------d . | a-----d . |",
  "| . . . . | | . . . . . . .        . . . . . . . | | . . . . |",
  "c-----b . | | . a-b . a----------------b . a-b . | | . a-----d",
  "      | . | | . | | . |                | . | | . | | . |      ",
  "      | . c-d . c-d . c----------------d . c-d . c-d . |      ",
  "      | . . . . . . . .   .  .  .  .   . . . . . . . . |      ",
  "      c------------------------------------------------d      "

};

static void drawField(WINDOW* win){
//  box(win, 0, 0);
  for(int i = 0; i < HEIGHT; i++){
    for(int j = 0; j < WIDTH; j++){
        wmove(win, i, j);
        switch(map[i][j]){
          case '.':
            waddch(win, '.');
            break;
          case '|':
            wvline(win, 0, 1);
            break;
          case '-':
            whline(win, 0, 1);
            break;
          case 'a':
            waddch(win, ACS_ULCORNER);
            break;
          case 'b':
            waddch(win, ACS_URCORNER);
            break;
          case 'c':
            waddch(win, ACS_LLCORNER);
            break;
          case 'd':
            waddch(win, ACS_LRCORNER);
            break;
          case 'T':
            waddch(win, ACS_TTEE);
            break;
        }

    }
  }
  wrefresh(win);
  getch();
}

void runPacman(int highscore){
  //timeout(70);
  clear();
	refresh();
//  int width, height;
//	getmaxyx(stdscr, height, width);
  WINDOW* win = newwin(HEIGHT + 1, WIDTH, 2, 2);
  drawField(win);

  delwin(win);
}
