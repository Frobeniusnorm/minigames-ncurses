#include "pacman.h"
#include "graph.h"
#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define WIDTH 62
#define HEIGHT 27

static const char backup_map[HEIGHT][WIDTH] = {
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
  "            | . | |   a------    ------b   | | . |            ",
  "------------d . c-d   |                |   c-d . c------------",
  ". . . . . . . .       |                |       . . . . . . . .",
  "------------b . a-b   c----------------d   a-b . a------------",
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
static char map[HEIGHT][WIDTH];
typedef enum Direction{
  NONE, UP, LEFT, DOWN, RIGHT
} Direction;
typedef struct Pacman{
  int x, y;
  Direction desiredDir;
  Direction dir;
} Pacman;
static Pacman pacman;
static int mouthOpen = 0;
static int canMove(int y, int x){
  if(y < 0 || x < 0 || y >= HEIGHT || x >= WIDTH) return true; //for teleporting
  char a = map[y][x];
  char b = x < WIDTH - 1 ? map[y][x+1] : ' ';
  char c = x > 0 ? map[y][x-1] : ' ';
  return a != '-' && a != '|' && (a < 'a' || a > 'd') && b != '-' && b != '|' && (b < 'a' || b > 'd') && c != '-' && c != '|' && (c < 'a' || c > 'd');
}
static void drawField(WINDOW* win){
//  box(win, 0, 0);
  wattron(win, COLOR_PAIR(1));
  for(int i = 0; i < HEIGHT; i++){
    for(int j = 0; j < WIDTH; j++){
        wmove(win, i, j);
        switch(map[i][j]){
          case '.':
            wattroff(win, COLOR_PAIR(1));
            wattron(win, COLOR_PAIR(2));
            waddch(win, '.');
            wattroff(win, COLOR_PAIR(2));
            wattron(win, COLOR_PAIR(1));
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
          case '*':
            waddch(win, '*');
            break;
          case 'p':
            wattroff(win, COLOR_PAIR(1));
            wattron(win, COLOR_PAIR(3));
            char pacchar = 'o';
            if(mouthOpen >= 4){
              switch(pacman.dir){
                case DOWN: pacchar = '^'; break;
                case LEFT: pacchar = '>'; break;
                case RIGHT: pacchar = '<'; break;
                case UP: pacchar = 'v'; break;
                case NONE:
                default:
                break;
              }
            }
            waddch(win, pacchar);
            wattroff(win, COLOR_PAIR(3));
            wattron(win, COLOR_PAIR(1));
        }

    }
  }
  wrefresh(win);
}
static void findWay(int sy, int sx, int gy, int gx){
  //clear field
  for(int i = 0; i < HEIGHT; i++)
    for(int j = 0; j < WIDTH; j++)
      if(map[i][j] == '*')
        map[i][j] = ' ';

  //find Way
  Way w = aStar(sy, sx, gy, gx, &map[0][0], HEIGHT, WIDTH);
  mvprintw(5, WIDTH + 3, "size: %d\n", w.size);
  for(int i = 0; i < w.size; i++){
    int y = w.way[i*2];
    int x = w.way[i*2 + 1];
    map[y][x] = '*';
  }
  free(w.way);
}

static void translateDir(Direction dir, int* mvy, int* mvx){
  switch(dir){
    case UP:
      *mvx = 0;
      *mvy = -1;
    break;
    case RIGHT:
      *mvx = 1;
      *mvy = 0;
    break;
    case DOWN:
      *mvx = 0;
      *mvy = 1;
    break;
    case LEFT:
      *mvx = -1;
      *mvy = 0;
    break;
    case NONE:
    default:
    //ignore
    break;
  }
}
static void gameLogic(int doYTick){
  int mvx, mvy;
  //check for desired dir
  if(pacman.desiredDir != NONE && pacman.desiredDir != pacman.dir){
    translateDir(pacman.desiredDir, &mvy, &mvx);
    if(canMove(pacman.y + mvy, pacman.x + mvx)){
      pacman.dir = pacman.desiredDir;
      pacman.desiredDir = NONE;
    }
  }
  //movement tick
  translateDir(pacman.dir, &mvy, &mvx);
  if(!doYTick) mvy = 0;
  if(canMove(pacman.y + mvy, pacman.x + mvx)){
    map[pacman.y][pacman.x] = ' ';
    pacman.y += mvy;
    pacman.x += mvx;
    if(pacman.x < 0) pacman.x = WIDTH-1;
    if(pacman.x >= WIDTH) pacman.x = 0;

    map[pacman.y][pacman.x] = 'p';
  }
  //visualize way
  findWay(1, 2, pacman.y, pacman.x);
}
void runPacman(int highscore){
  timeout(1);
  clear();
	refresh();
  //init pacman
  pacman.y = 21;
  pacman.x = 28;
  pacman.dir = RIGHT;
  pacman.desiredDir = NONE;
  //init map
  for(int j = 0; j < HEIGHT; j++)
    for(int i = 0; i < WIDTH; i++)
      map[j][i] = backup_map[j][i];

  //create window
  WINDOW* win = newwin(HEIGHT + 1, WIDTH, 2, 2);
  int closeRequest = 0;
  //colors
  init_color(COLOR_BLACK, 0, 0, 0);
  init_color(COLOR_BLUE, 100, 100, 700);
  init_color(COLOR_MAGENTA, 600, 500, 400);
  init_color(COLOR_YELLOW, 950, 900, 400);
  init_pair(1, COLOR_BLUE, COLOR_BLACK); //walls
  init_pair(2, COLOR_MAGENTA, COLOR_BLACK); //dots
  init_pair(3, COLOR_YELLOW, COLOR_BLACK); //pacman
  //for time
  struct timespec time;
	uint64_t nanos = 0;
  //game loop
  int doYTick = 1;
  while(!closeRequest){
    //input handling runs nvtl
    int c = getch();
    switch(c){
      case KEY_LEFT:
        pacman.desiredDir = LEFT;
        break;
      case KEY_RIGHT:
        pacman.desiredDir = RIGHT;
        break;
      case KEY_DOWN:
        pacman.desiredDir = DOWN;
        break;
      case KEY_UP:
        pacman.desiredDir = UP;
        break;
      case 'q':
        closeRequest = true;
      break;
    }
    clock_gettime(CLOCK_REALTIME, &time);
    long curr = time.tv_nsec;
    //we want each frame to take 100000000 nano seconds (0.1 seconds)
    if((curr - nanos) > 100000000l){
      gameLogic(doYTick);
      //only move every second frame on y
      doYTick = (doYTick + 1) % 2;
      mouthOpen = (mouthOpen + 1) % 8;
      wclear(win);
      drawField(win);
      nanos = curr;
    }
  }
  delwin(win);
}
