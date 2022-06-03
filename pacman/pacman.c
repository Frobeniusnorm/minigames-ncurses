#include "pacman.h"
#include "graph.h"
#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#define WIDTH 62
#define HEIGHT 27
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
static const char backup_map[HEIGHT][WIDTH] = {
  "a-----------------------------ba-----------------------------b",
  "| . . . . . . . . . . . . . . || . . . . . . . . . . . . . . |",
  "| * a-------b . a---------b . || . a---------b . a-------b * |",
  "| . c-------d . c---------d . cd . c---------d . c-------d . |",
  "| . . . . . . . . . . . . . .    . . . . . . . . . . . . . . |",
  "| . a-------b . a-b . a----------------b . a-b . a-------b . |",
  "| . c-------d . | | . c-----b    a-----d . | | . c-------d . |",
  "| . . . . . . . | | . . . . |    | . . . . | | . . . . . . . |",
  "c-----------b . | c-----b . |    | . a-----d | . a-----------d",
  "            | . | a-----d . c----d . c-----b | . |            ",
  "            | . | |                        | | . |            ",
  "            | . | |   a------tttt------b   | | . |            ",
  "------------d . c-d   | q              |   c-d . c------------",
  ". . . . . . . .       |                |       . . . . . . . .",
  "------------b . a-b   c----------------d   a-b . a------------",
  "            | . | |                        | | . |            ",
  "            | . | |   a----------------b   | | . |            ",
  "a-----------d . c-d   c-----b    a-----d   c-d . c-----------b",
  "| . . . . . . . . . . . . . |    | . . . . . . . . . . . . . |",
  "| . a-------b . a-------b . |    | . a-------b . a-------b . |",
  "| . c-----b | . c-------d . c----d . c-------d . | a-----d . |",
  "| * . . . | | . . . . . . .        . . . . . . . | | . . . * |",
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
typedef enum GhostMode{
  CHASE, SCATTER, FRIGHTENED, FRIGHTENED_BLINK
} GhostMode;
typedef struct Pacman{
  int x, y;
  Direction desiredDir;
  Direction dir;
} Pacman;
typedef struct Ghost{
  int x, y;
  char prevFieldContent;
  int doYTick;
  int scattering;
  double inhouse;
  int walkx, walky;
  Direction dir;
} Ghost;
static Pacman pacman;
static Ghost blinky, pinky, inky, clyde;
static Ghost* ghosts[4] = {&blinky, &pinky, &inky, &clyde};
static GhostMode mode;
static int mouthOpen = 0;
static int gameover = 0;
static int setFrightened = 0;
static int canMove(int y, int x){
  if(y < 0 || x < 0 || y >= HEIGHT || x >= WIDTH) return true; //for teleporting
  char a = map[y][x];
  char b = x < WIDTH - 1 ? map[y][x+1] : ' ';
  char c = x > 0 ? map[y][x-1] : ' ';
  //isVisitable would allow the door of the ghost room
  return isVisitable(a) && isVisitable(b) && isVisitable(c) && a != 't' && b != 't' && c != 't';
}
//checks if a position is in the house
static int inHouse(int starty, int startx){
    return starty > 11 && starty < 14 && startx > 22 && startx < 39;
}
static void drawField(WINDOW* win){
//  box(win, 0, 0);
  wattron(win, COLOR_PAIR(1));
  for(int i = 0; i < HEIGHT; i++){
    for(int j = 0; j < WIDTH; j++){
        wmove(win, i, j);
        switch(map[i][j]){
          case '*':
          case '.':
            wattroff(win, COLOR_PAIR(1));
            wattron(win, COLOR_PAIR(2));
            waddch(win, map[i][j]);
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
          case 't':
            wattroff(win, COLOR_PAIR(1));
            wattron(win, COLOR_PAIR(2));
            whline(win, 0, 1);
            wattroff(win, COLOR_PAIR(2));
            wattron(win, COLOR_PAIR(1));
            break;
          case 'T':
            waddch(win, ACS_TTEE);
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
            break;
          case 'q': //blinky
            if(mode != FRIGHTENED && mode != FRIGHTENED_BLINK || (mode == FRIGHTENED_BLINK && mouthOpen >= 4)){
              wattroff(win, COLOR_PAIR(1));
              wattron(win, COLOR_PAIR(4));
            }

            if(mouthOpen >= 4)
              waddch(win, '&');
            else waddch(win, 'O');
            if(mode != FRIGHTENED && mode != FRIGHTENED_BLINK || (mode == FRIGHTENED_BLINK && mouthOpen >= 4)){
              wattroff(win, COLOR_PAIR(4));
              wattron(win, COLOR_PAIR(1));
            }
            break;
          case 'r': //pinky
            if(mode != FRIGHTENED && mode != FRIGHTENED_BLINK || (mode == FRIGHTENED_BLINK && mouthOpen >= 4)){
              wattroff(win, COLOR_PAIR(1));
              wattron(win, COLOR_PAIR(6));
            }
            if(mouthOpen >= 4)
              waddch(win, '&');
            else waddch(win, 'O');
            if(mode != FRIGHTENED || (mode == FRIGHTENED_BLINK && mouthOpen >= 4)){
              wattroff(win, COLOR_PAIR(6));
              wattron(win, COLOR_PAIR(1));
            }
            break;
          case 's': //inky
            if(mode != FRIGHTENED && mode != FRIGHTENED_BLINK || (mode == FRIGHTENED_BLINK && mouthOpen >= 4)){
              wattroff(win, COLOR_PAIR(1));
              wattron(win, COLOR_PAIR(7));
            }
            if(mouthOpen >= 4)
              waddch(win, '&');
            else waddch(win, 'O');
            if(mode != FRIGHTENED || (mode == FRIGHTENED_BLINK && mouthOpen >= 4)){
              wattroff(win, COLOR_PAIR(7));
              wattron(win, COLOR_PAIR(1));
            }
            break;
          case 'u': //clyde
            if(mode != FRIGHTENED && mode != FRIGHTENED_BLINK || (mode == FRIGHTENED_BLINK && mouthOpen >= 4)){
              wattroff(win, COLOR_PAIR(1));
              wattron(win, COLOR_PAIR(8));
            }
            if(mouthOpen >= 4)
              waddch(win, '&');
            else waddch(win, 'O');
            if(mode != FRIGHTENED || (mode == FRIGHTENED_BLINK && mouthOpen >= 4)){
              wattroff(win, COLOR_PAIR(8));
              wattron(win, COLOR_PAIR(1));
            }
            break;
        }

    }
  }
  wrefresh(win);
}
static void translateMove(int mvy, int mvx, Direction* dir){
    if(mvy > 0) *dir = DOWN;
    else if(mvx > 0) *dir = RIGHT;
    else if(mvy < 0) *dir = UP;
    else if(mvx < 0) *dir = LEFT;
    //else *dir = NONE;
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
static void frightened(Ghost* ghost, double time){
  if(ghost->inhouse > 0){
    if(time - ghost->inhouse > 5)
      ghost->inhouse = -1;
    return;
  }
  //if it is in the house it first has to get out
  if(inHouse(ghost->y, ghost->x)){
    if(ghost->x != 30)
      ghost->x += (ghost->x > 30 ? -1 : 1);
    else if(ghost->doYTick) ghost->y--;
    ghost->dir = UP;
    return;
  }
  int options[4] = {1, 1, 1, 1};
  int countop = 0;
  int offsets[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
  int didy = ghost->walky != 0;
  for(int i = 0; i < 4; i++){
    int cy = ghost->y + offsets[i][0], cx = ghost->x + offsets[i][1];
    if(cy < 0 || cx < 0 || cy >= HEIGHT || cx >= WIDTH || !isVisitable(map[cy][cx]) || map[cy][cx] == 't'){
      options[i] = 0;
      continue;
    }
    int doVisit = 1;
    for(int off = -1; off <= 1; off+=2){
      int nx = cx + off;
      if(nx < WIDTH && nx >= 0 && !isVisitable(map[cy][nx])){
        doVisit = 0;
        break;
      }
    }
    //do not turn around
    if((ghost->walky != 0 || ghost->walkx != 0) &&
         ((offsets[i][0] == 0 && ghost->walky == 0 && offsets[i][1] == -ghost->walkx) ||
          (offsets[i][1] == 0 && ghost->walkx == 0 && offsets[i][0] == -ghost->walky)))
       doVisit = 0;

    if(!doVisit) options[i] = 0;
    else countop++;
  }

  if(countop == 0){
    //at portal (as long as i haven't fucked up)
    if(ghost->x + ghost->walkx >= WIDTH) ghost->x = 0;
    if(ghost->x + ghost->walkx < 0) ghost->x = WIDTH - 1;
  }else{
    //randomly choose option
    int optidx = rand() % countop;
    int posidx = 0;
    for(int j = 0; j < 4; j++){
      if(options[j]){
        if(posidx++ == optidx){
          ghost->walky = offsets[j][0];
          ghost->walkx = offsets[j][1];
          if(ghost->doYTick || !didy) ghost->y += ghost->walky;
          ghost->x += ghost->walkx;
          break;
        }
      }
    }
  }
}
static void shortestWayChaseLogic(Ghost* g, int targety, int targetx){
  g->scattering = 0; //to reset
  const Way nw = aStar(g->y, g->x, targety, targetx, &map[0][0], HEIGHT, WIDTH, g->dir);
  int oldx = g->x, oldy = g->y;
  if(nw.size >= 1){
    const int ny = nw.way[0], nx = nw.way[1];
    if(ny != g->y && g->doYTick){
      g->y = ny;
    }else g->x = nx;
  }
  translateMove(g->y - oldy, g->x - oldx, &g->dir);
  free(nw.way);
}
static void scatter(Ghost* ghost, int* way, int waysize){
  if(ghost->x == way[0] && ghost->y == way[1])
    ghost->scattering = 1;
  if(ghost->scattering){
    for(int i = 0; i < waysize; i++){
      if(way[i*2] == ghost->x && way[i*2 + 1] == ghost->y){
        const int nexti = i < waysize - 1 ? i + 1 : 0;
        const int xdiff = way[nexti * 2] - way[i * 2];
        const int ydiff = way[nexti * 2 + 1] - way[i * 2 + 1];
        ghost->walkx = xdiff < 0 ? -1 : xdiff == 0 ? 0 : 1;
        ghost->walky = ydiff < 0 ? -1 : ydiff == 0 ? 0 : 1;
        break;
      }
    }
    ghost->x += ghost->walkx;
    if(ghost->doYTick)
      ghost->y += ghost->walky;
    translateMove(ghost->walky, ghost->walkx, &ghost->dir);
  }else{
    shortestWayChaseLogic(ghost, way[1], way[0]);
  }
}

static void ghostCollisionLogic(Ghost*g, double time, int housey, int housex, int origy, int origx, char ghostChar){
  if(g->y == pacman.y && g->x == pacman.x || origy == pacman.y && origx == pacman.x){
    if(mode == FRIGHTENED || mode == FRIGHTENED_BLINK){
      sleep(1);
      g->y = housey;
      g->x = housex;
      g->dir = UP;
      g->inhouse = time;
    }else gameover = 1;
  }
  char prev = map[g->y][g->x];
  if(prev == ' ' || prev == '*' || prev == '.' || prev == 't'){
    g->prevFieldContent = prev;
    map[g->y][g->x] = ghostChar;
  }else g->prevFieldContent = 127;
}
static void inkyLogic(double time){
  char prev = inky.prevFieldContent;
  if(prev != 127) map[inky.y][inky.x] = prev;
  int origx = inky.x, origy = inky.y;
  switch(mode){
    case CHASE:
    {
      inky.scattering = 0;
      int mvpx = (pacman.dir == UP || pacman.dir == DOWN)? 0 : (pacman.dir == LEFT ? -1 : (pacman.dir == RIGHT ? 1 : 0));
      int mvpy = (pacman.dir == LEFT || pacman.dir == RIGHT) ? 0 : (pacman.dir == UP ? -1 : (pacman.dir == DOWN ? 1 : 0));
      int targetx = pacman.x + mvpx * 2;
      int targety = pacman.y + mvpy * 2;
      //two ahead now mirror by blinky
      targetx += targetx - blinky.x;
      targety += targety - blinky.y;
      targetx = MIN(WIDTH-1, MAX(0, targetx));
      targety = MIN(HEIGHT-1, MAX(0, targety));

      shortestWayChaseLogic(&inky, targety, targetx);
      break;
    }
    case SCATTER:
      {
        int way[12] = {59, 18, 59, 21, 53, 21, 53, 25, 47, 25, 47, 18};
        scatter(&inky, &way[0], 6);
      }
      break;
    case FRIGHTENED_BLINK:
    case FRIGHTENED:
      inky.scattering = 0;
      frightened(&inky, time);
      break;
  }
  ghostCollisionLogic(&inky, time, 12, 28, origy, origx, 's');
}
static void blinkyLogic(double time){
  char prev =  blinky.prevFieldContent;
  if(prev != 127) map[blinky.y][blinky.x] = prev;
  int origx = blinky.x, origy = blinky.y;
  switch(mode){
    case CHASE:
      shortestWayChaseLogic(&blinky, pacman.y, pacman.x);
      break;
    case SCATTER:
      {
        int way[8] = {47, 1, 59, 1, 59, 4, 47, 4};
        scatter(&blinky, &way[0], 4);
      }
      break;
    case FRIGHTENED_BLINK:
    case FRIGHTENED:
      blinky.scattering = 0;
      frightened(&blinky, time);
      break;
  }
  ghostCollisionLogic(&blinky, time, 12, 24, origy, origx, 'q');
}
static void clydeLogic(double time){
  char prev = clyde.prevFieldContent;
  if(prev != 127) map[clyde.y][clyde.x] = prev;
  int origx = clyde.x, origy = clyde.y;
  switch(mode){
    case CHASE:
      {
        int xdiff = pacman.x - clyde.x;
        int ydiff = pacman.y - clyde.y;
        double distance = sqrt(xdiff * xdiff - ydiff * ydiff);
        if(distance > 8){
          //chase pacman
          shortestWayChaseLogic(&clyde, pacman.y, pacman.x);
          break;
        }//else no break to scatter
      }
    case SCATTER:
      {
        int way[12] = {2, 18, 2, 21, 8, 21, 8, 25, 14, 25, 14, 18};
        scatter(&clyde, &way[0], 6);
        break;
      }
    case FRIGHTENED_BLINK:
    case FRIGHTENED:
      clyde.scattering = 0;
      frightened(&clyde, time);
      break;
  }
  ghostCollisionLogic(&clyde, time, 12, 32, origy, origx, 'u');
}
static void pinkyLogic(double time){
  char prev =  pinky.prevFieldContent;
  if(prev != 127) map[pinky.y][pinky.x] = prev;
  int origx = pinky.x, origy = pinky.y;
  switch(mode){
    case CHASE:
    {
      pinky.scattering = 0;
      int mvpx = (pacman.dir == UP || pacman.dir == DOWN)? 0 : (pacman.dir == LEFT ? -1 : (pacman.dir == RIGHT ? 1 : 0));
      int mvpy = (pacman.dir == LEFT || pacman.dir == RIGHT) ? 0 : (pacman.dir == UP ? -1 : (pacman.dir == DOWN ? 1 : 0));
      int targetx = MIN(WIDTH - 1, MAX(0, pacman.x + mvpx * 4));
      int targety = MIN(HEIGHT - 1, MAX(0, pacman.y + mvpy * 4));
      shortestWayChaseLogic(&pinky, targety, targetx);
      break;
    }
    case SCATTER:
      {
        int way[8] = {2, 1, 2, 4, 14, 4, 14, 1};
        scatter(&pinky, &way[0], 4);
      }
      break;
    case FRIGHTENED_BLINK:
    case FRIGHTENED:
      pinky.scattering = 0;
      frightened(&pinky, time);
      break;
  }
  ghostCollisionLogic(&pinky, time, 12, 26, origy, origx, 'r');
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
    if(map[pacman.y][pacman.x] == '*') setFrightened = 1;
    map[pacman.y][pacman.x] = 'p';
  }
}
void runPacman(int highscore){
  timeout(1);
  clear();
	refresh();
  gameover = 0;
  setFrightened = 0;
  //init pacman
  pacman.y = 21;
  pacman.x = 28;
  pacman.dir = RIGHT;
  pacman.desiredDir = NONE;
  //init ghosts
  mode = SCATTER;
  //init speedy
  blinky.y = 12;
  blinky.x = 24;
  //init blinky
  pinky.y = 12;
  pinky.x = 26;
  //init inky
  inky.y = 12;
  inky.x = 30;
  //init clyde
  clyde.y = 12;
  clyde.x = 32;

  for(int i = 0; i < 4; i++){
    Ghost* g = ghosts[i];
    g->prevFieldContent = ' ';
    g->doYTick = 0;
    g->scattering = 0;
    g->inhouse = -1;
    g->dir = NONE;
  }
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
  init_color(COLOR_MAGENTA, 700, 500, 400);
  init_color(COLOR_YELLOW, 950, 900, 400);
  init_color(COLOR_RED, 1000, 300, 200);
  init_color(COLOR_WHITE, 1000, 1000, 1000);
  init_color(COLOR_GREEN, 800, 600, 700);
  init_color(COLOR_CYAN, 100, 600, 700);
  init_pair(1, COLOR_BLUE, COLOR_BLACK); //walls
  init_pair(2, COLOR_MAGENTA, COLOR_BLACK); //dots
  init_pair(3, COLOR_YELLOW, COLOR_BLACK); //pacman
  init_pair(4, COLOR_RED, COLOR_BLACK); //blinky
  init_pair(5, COLOR_WHITE, COLOR_BLACK);
  init_pair(6, COLOR_GREEN, COLOR_BLACK); //pinky
  init_pair(7, COLOR_CYAN, COLOR_BLACK); //inky
  init_pair(8, COLOR_MAGENTA, COLOR_BLACK); //clyde
  //for time
  const double chaseTime = 20.0, scatterTime = 7.0; //TODO: change for different levels
  struct timespec time;
	double pacmanTimeStamp = 0, speedyTimeStamp = 0, modeTimeStamp = 0;
  //game loop
  int doYTick = 1;
  while(!closeRequest && !gameover){
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
    double seconds = time.tv_sec + (time.tv_nsec / 1000000000.0);
    int toDraw = 0;
    if(pacmanTimeStamp == 0){
      pacmanTimeStamp = speedyTimeStamp = modeTimeStamp = seconds;
    }
    //mode switch timer
    if(setFrightened){
      setFrightened = 0;
      mode = FRIGHTENED;
      modeTimeStamp = seconds;
      for(int i = 0; i < 4; i++){
        Ghost* g = ghosts[i];
        g->walky = 0;
        g->walkx = 0;
      }
    }
    if(mode != FRIGHTENED && mode != FRIGHTENED_BLINK){
      if(mode == SCATTER && (seconds - modeTimeStamp) > scatterTime){
        mode = CHASE;
        modeTimeStamp = seconds;
      }else if(mode == CHASE  && (seconds - modeTimeStamp) > chaseTime){
        mode = SCATTER;
        modeTimeStamp = seconds;
      }
    }else if(seconds - modeTimeStamp > 13.0){
      mode = CHASE;
      modeTimeStamp = seconds;
    }else if(seconds - modeTimeStamp > 10.0)
      mode = FRIGHTENED_BLINK;

    //blinky timer
    if((seconds - speedyTimeStamp) > 0.12){
      pinky.doYTick = (pinky.doYTick + 1) % 2;
      pinkyLogic(seconds); //todo: pinky must be slower than blinky
      inky.doYTick = (blinky.doYTick + 1) % 2;
      inkyLogic(seconds); //todo: inky must be slower than blinky
      blinky.doYTick = (blinky.doYTick + 1) % 2;
      blinkyLogic(seconds);
      clyde.doYTick = (clyde.doYTick + 1) % 2;
      clydeLogic(seconds); //todo: clyde must be slower than blinky
      speedyTimeStamp = seconds;
    }
    if((seconds - pacmanTimeStamp) > 0.1){
      gameLogic(doYTick);
      //only move every second frame on y
      doYTick = (doYTick + 1) % 2;
      mouthOpen = (mouthOpen + 1) % 8;
      pacmanTimeStamp = seconds;
      wclear(win);
      drawField(win);
    }
  }
  //game ended
  if(gameover){
    wattron(win, COLOR_PAIR(5));
    mvwprintw(win, HEIGHT/2-1, WIDTH/2-5, "GAME OVER");
  	mvwprintw(win, HEIGHT/2, WIDTH/2-9, "[press q to exit]");
  	wrefresh(win);
  	sleep(1);
  	while(getch() != 'q')
    ;
  }
  delwin(win);
}
