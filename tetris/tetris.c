#include <math.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define WIDTH 50
#define HEIGHT 40
#define LVL_UP 3200
#define NUM_TYPES 7
#define NUM_COLORS 6
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
typedef struct Block {
  short x, y;
  short color;
} Block;
typedef struct Form {
  Block blocks[4];
  int x, y;
  int type;
  int rotation;
  int color;
} Form;

static WINDOW *field, *preview, *currView, *scoreView;
static Form *current;
static Block *data[WIDTH / 2][HEIGHT];
static Block *previewData[5][5];
static int writeToData = 1;
static int keepRunning = 1;
static int gameOver = 0;
// point
static int score = 0;
static int level = 0;

static void setBlock(Block *b, const int y, const int x) {
  if (writeToData == 1)
    data[x][y] = b;
  else if (writeToData == 2) {
    previewData[x][y] = b;
  }
  b->x = x;
  b->y = y;
}
/*
 #
###
*/
static void drawFigure1(Form *form) {
  int x = form->x;
  int y = form->y;
  int rotation = form->rotation;
  int blockidx = 0;
  switch (rotation) {
  case 0:
    setBlock(&form->blocks[blockidx++], y, x + 1);
    for (int i = 0; i < 3; i++)
      setBlock(&form->blocks[blockidx++], y + 1, x + i);
    break;
  case 1:
    setBlock(&form->blocks[blockidx++], y + 1, x + 1);
    for (int i = 0; i < 3; i++)
      setBlock(&form->blocks[blockidx++], y + i, x);
    break;
  case 2:
    setBlock(&form->blocks[blockidx++], y + 1, x + 1);
    for (int i = 0; i < 3; i++)
      setBlock(&form->blocks[blockidx++], y, x + i);
    break;
  case 3:
    setBlock(&form->blocks[blockidx++], y + 1, x);
    for (int i = 0; i < 3; i++)
      setBlock(&form->blocks[blockidx++], y + i, x + 1);
    break;
  }
}
/*
####
*/
static void drawFigure2(Form *form) {
  int x = form->x;
  int y = form->y;
  int rotation = form->rotation;
  int blockidx = 0;
  int start = 0;
  switch (rotation) {
  case 0:
    start = 1;
  case 2:
    for (int i = 0; i < 4; i++)
      setBlock(&form->blocks[blockidx++], y + 2, x + i + start);
    break;
  case 1:
    start = 1;
  case 3:
    for (int i = 0; i < 4; i++)
      setBlock(&form->blocks[blockidx++], y + i + start, x + 2);
    break;
  }
}
/*
##
##
*/
static void drawFigure3(Form *form) {
  int x = form->x;
  int y = form->y;
  int rotation = form->rotation;
  int blockidx = 0;
  int startx = 0;
  int starty = 0;
  switch (rotation) {
  case 0:
    startx = 1;
    starty = 1;
    break;
  case 1:
    starty = 1;
    break;
  case 3:
    startx = 1;
    break;
  }
  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 2; j++)
      setBlock(&form->blocks[blockidx++], y + j + starty, x + i + startx);
}
/*
#
####
*/
static void drawFigure4(Form *form) {
  int x = form->x;
  int y = form->y;
  int rotation = form->rotation;
  int blockidx = 0;
  int start = 0;
  switch (rotation) {
  case 0:
    start = 1;
  case 2:
    setBlock(&form->blocks[blockidx++], start == 1 ? y : y + 2,
             start == 1 ? x + 1 : x + 2);
    for (int i = 0; i < 3; i++)
      setBlock(&form->blocks[blockidx++], y + 1, x + i + start);
    break;
  case 1:
    start = 1;
  case 3:
    setBlock(&form->blocks[blockidx++], start == 1 ? y + 1 : y + 2,
             start == 1 ? x + 2 : x);
    for (int i = 0; i < 3; i++)
      setBlock(&form->blocks[blockidx++], y + i + start, x + 1);
    break;
  }
}
/*
   #
####
*/
static void drawFigure5(Form *form) {
  int x = form->x;
  int y = form->y;
  int rotation = form->rotation;
  int blockidx = 0;
  int start = 0;
  switch (rotation) {
  case 0:
    start = 1;
  case 2:
    setBlock(&form->blocks[blockidx++], start == 1 ? y : y + 2,
             start == 1 ? x + 3 : x);
    for (int i = 0; i < 3; i++)
      setBlock(&form->blocks[blockidx++], y + 1, x + i + start);
    break;
  case 1:
    start = 1;
  case 3:
    setBlock(&form->blocks[blockidx++], start == 1 ? y + 3 : y,
             start == 1 ? x + 2 : x);
    for (int i = 0; i < 3; i++)
      setBlock(&form->blocks[blockidx++], y + i + start, x + 1);
    break;
  }
}
/*
##
 ##
*/
static void drawFigure6(Form *form) {
  int x = form->x;
  int y = form->y;
  int rotation = form->rotation;
  int blockidx = 0;
  int start = 0;
  switch (rotation) {
  case 0:
    start = 1;
  case 2:
    for (int i = 0; i < 4; i++)
      setBlock(&form->blocks[blockidx++], y + 1 + (i > 1 ? 1 : 0),
               x + i + start + (i > 1 ? -1 : 0));
    break;
  case 1:
    start = 1;
  case 3:
    for (int i = 0; i < 4; i++)
      setBlock(&form->blocks[blockidx++], y + i + start + (i > 1 ? -1 : 0),
               x + 1 + (i > 1 ? -1 : 0));
    break;
  }
}
/*
 ##
##
*/
static void drawFigure7(Form *form) {
  int x = form->x;
  int y = form->y;
  int rotation = form->rotation;
  int blockidx = 0;
  int start = 0;
  switch (rotation) {
  case 0:
    start = 1;
  case 2:
    for (int i = 0; i < 4; i++)
      setBlock(&form->blocks[blockidx++], y + 1 + (i > 1 ? 0 : 1),
               x + i + start + (i > 1 ? -1 : 0));
    break;
  case 1:
    start = 1;
  case 3:
    for (int i = 0; i < 4; i++)
      setBlock(&form->blocks[blockidx++], y + i + start + (i > 1 ? -1 : 0),
               x + 1 + (i > 1 ? 0 : -1));
    break;
  }
}
static void drawField(const int sx, const int sy) {
  wclear(currView);
  int currcol = 1;
  wattron(currView, COLOR_PAIR(currcol));
  for (int x = sx; x < sx + 5; x++) {
    for (int y = sy; y < sy + 5; y++) {
      if (x >= 0 && x < WIDTH / 2 && y >= 0 && y < HEIGHT && data[x][y]) {
        if (currcol != data[x][y]->color) {
          wattroff(currView, COLOR_PAIR(currcol));
          currcol = data[x][y]->color;
          wattron(currView, COLOR_PAIR(currcol));
        }
        mvwaddch(currView, data[x][y]->y - sy, (data[x][y]->x - sx) * 2, ' ');
        mvwaddch(currView, data[x][y]->y - sy, (data[x][y]->x - sx) * 2 + 1,
                 ' ');
      }
    }
  }
  wattroff(currView, COLOR_PAIR(currcol));
  wrefresh(currView);
}
static void drawCompleteField() {
  wclear(field);
  box(field, 0, 0);
  int currcol = 1;
  wattron(field, COLOR_PAIR(currcol));
  for (int x = 0; x < WIDTH / 2; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      if (data[x][y]) {
        if (currcol != data[x][y]->color) {
          wattroff(field, COLOR_PAIR(currcol));
          currcol = data[x][y]->color;
          wattron(field, COLOR_PAIR(currcol));
        }
        mvwaddch(field, data[x][y]->y, (data[x][y]->x) * 2, ' ');
        mvwaddch(field, data[x][y]->y, (data[x][y]->x) * 2 + 1, ' ');
      }
    }
  }
  wattroff(field, COLOR_PAIR(currcol));
  wrefresh(field);
}
static void drawFigure(Form *fig) {
  for (int i = 0; i < 4; i++) {
    Block *b = &fig->blocks[i];
    if (writeToData == 2)
      previewData[b->x][b->y] = NULL;
    else
      data[b->x][b->y] = NULL;
  }
  switch (fig->type) {
  case 1:
    drawFigure1(fig);
    break;
  case 2:
    drawFigure2(fig);
    break;
  case 3:
    drawFigure3(fig);
    break;
  case 4:
    drawFigure4(fig);
    break;
  case 5:
    drawFigure5(fig);
    break;
  case 6:
    drawFigure6(fig);
    break;
  case 7:
    drawFigure7(fig);
    break;
  }
}
static int newType = 1;
static int newColor = 1;

static void drawPreview() {
  wclear(preview);
  // clear field
  for (int x = 0; x < 5; x++) {
    for (int y = 0; y < 5; y++) {
      previewData[x][y] = 0;
    }
  }
  box(preview, 0, 0);
  writeToData = 2;

  Form *next = (Form *)malloc(sizeof(Form));
  next->x = 0;
  next->y = 0;
  next->color = newColor;
  next->type = newType;
  next->rotation = 0;
  for (int i = 0; i < 4; i++) {
    next->blocks[i].color = newColor;
    next->blocks[i].x = 0;
    next->blocks[i].y = 0;
  }
  drawFigure(next);
  writeToData = 1;
  // actually draw it
  int xoff = next->type == 2 ? 4 : next->type == 3 || next->type == 1 ? 6 : 5;
  int yoff = next->type == 2 ? 0 : 1;
  int currcol = 1;
  wattron(preview, COLOR_PAIR(currcol));
  for (int x = 0; x < 5; x++) {
    for (int y = 0; y < 5; y++) {
      Block *cell = previewData[x][y];
      if (cell) {
        if (currcol != cell->color) {
          wattroff(preview, COLOR_PAIR(currcol));
          currcol = cell->color;
          wattron(preview, COLOR_PAIR(currcol));
        }
        mvwaddch(preview, cell->y + yoff, (cell->x) * 2 + xoff, ' ');
        mvwaddch(preview, cell->y + yoff, (cell->x) * 2 + xoff + 1, ' ');
      }
    }
  }
  free(next);
  wattroff(preview, COLOR_PAIR(currcol));
  wrefresh(preview);
}
static void drawScoreView() {
  wclear(scoreView);
  box(scoreView, 0, 0);
  mvwprintw(scoreView, 1, 2, "Score: %d", score);
  mvwprintw(scoreView, 3, 2, "Level: %d", level);
  wrefresh(scoreView);
}
static int collisionDetection(Form *f, int mvx, int mvy) {
  for (int i = 0; i < 4; i++) {
    Block *b = &f->blocks[i];
    int nx = b->x + mvx, ny = b->y + mvy;
    if (nx < 1 || nx * 2 >= WIDTH - 2 || ny >= HEIGHT - 1) {
      return 0;
    }
    Block *db = data[nx][ny];
    if (db) {
      // check if it belongs to same figure
      int same = 0;
      for (int j = 0; j < 4; j++)
        if (db == &f->blocks[j]) {
          same = 1;
          break;
        }
      if (!same) {
        return 0;
      }
    }
  }
  return 1;
}
typedef struct ToFree {
  Form *tofree;
  struct ToFree *next;
} ToFree;
static ToFree *freeing = NULL;
static void newForm() {
  current = (Form *)malloc(sizeof(Form));
  current->type = newType;
  current->color = newColor;
  current->x = 9;
  current->y = newType != 0 ? 1 : 0;

  ToFree *nfreeing = (ToFree *)malloc(sizeof(ToFree));
  nfreeing->next = freeing;
  nfreeing->tofree = current;
  freeing = nfreeing;
  for (int i = 0; i < 4; i++) {
    current->blocks[i].x = current->x;
    current->blocks[i].y = current->y;
    current->blocks[i].color = current->color;
  }
  writeToData = 0;
  drawFigure(current);
  writeToData = 1;
  if (!collisionDetection(current, 0, 0)) {
    keepRunning = 0;
    gameOver = 1;
  }
  newType = rand() % NUM_TYPES + 1;
  newColor = rand() % NUM_COLORS + 1;
}
static void rotate(Form *f) {
  writeToData = 0;
  int oldRotation = f->rotation;
  f->rotation = (current->rotation + 1) % 4;
  drawFigure(current); // rotate without writing
  int res = collisionDetection(f, 0, 0);
  if (!res) // if it didnt work reset
    f->rotation = oldRotation;
  writeToData = 1;
  drawFigure(current); // write out
}
static uint64_t lastTick = 0;
static void logicTick(int c) {
  struct timespec time;
  clock_gettime(CLOCK_REALTIME, &time);
  uint64_t nanos = time.tv_nsec;

  int mvx = 0, mvy = 0;
  long timeDiff = level < 20 ? 500000000 - level * 20000000 : 80000000;
  if (nanos - lastTick > timeDiff) {
    mvy = 1;
    lastTick = nanos;
  }
  switch (c) {
  case KEY_UP:
    rotate(current);
    break;
  case KEY_LEFT:
    mvx = -1;
    break;
  case KEY_RIGHT:
    mvx = 1;
    break;
  case KEY_DOWN:
    if (!mvy) {
      mvy = 1;
      score += 1;
      level = score / LVL_UP;
      drawScoreView();
    }
  }
  if (collisionDetection(current, mvx, 0)) {
    current->x += mvx;
    drawFigure(current);
  }
  if (mvy > 0) {
    if (collisionDetection(current, 0, mvy)) {
      current->y += mvy;
      drawFigure(current);
    } else {
      // it is set, we can check for the lines and generate a new form
      int linesCleared = 0;
      for (int i = 0; i < 4; i++) {
        int y = current->blocks[i].y;
        // did we already check for that?
        int didcheck = 0;
        for (int j = 0; j < i; j++)
          if (current->blocks[j].y == y) {
            didcheck = 1;
            break;
          }
        if (!didcheck) {
          int full = 1;
          for (int j = 1; j < WIDTH / 2 - 2; j++)
            if (!data[j][y]) {
              full = 0;
              break;
            }
          if (full) {
            linesCleared++;
            for (int j = 1; j < WIDTH / 2 - 2; j++)
              for (int k = y; k >= 0; k--) {
                data[j][k] = k == 0 ? NULL : data[j][k - 1];
                if (data[j][k])
                  data[j][k]->y++;
              }
          }
        }
      }
      if (linesCleared > 0) {
        score += 50 *
                 (linesCleared == 1   ? 1
                  : linesCleared == 2 ? 3
                  : linesCleared == 3 ? 5
                                      : 7) *
                 (level + 1);
        level = score / LVL_UP;
        drawScoreView();
      }
      newForm();
      drawPreview();
      drawCompleteField();
    }
  }
}

void runTetris(int *maxscore) {
  keepRunning = 1;
  gameOver = 0;
  score = 0;
  level = 0;
  writeToData = 1;
  newType = rand() % NUM_TYPES + 1;
  newColor = rand() % NUM_COLORS + 1;
  clear();
  field = newwin(HEIGHT, WIDTH, 0, 0);
  preview = newwin(6, 20, 0, WIDTH + 1);
  currView = newwin(5, 10, 0, 0);
  scoreView = newwin(8, 20, 7, WIDTH + 1);
  for (int i = 0; i < WIDTH / 2; i++)
    for (int j = 0; j < HEIGHT; j++)
      data[i][j] = NULL;
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 4; j++)
      previewData[i][j] = NULL;

  timeout(70);
  init_color(COLOR_RED, 900, 0, 0);
  init_color(COLOR_GREEN, 100, 800, 0);
  init_color(COLOR_BLUE, 0, 0, 900);
  init_color(COLOR_CYAN, 0, 800, 800);
  init_color(COLOR_MAGENTA, 700, 0, 700);
  init_color(COLOR_YELLOW, 950, 900, 400);
  init_pair(1, COLOR_RED, COLOR_RED);
  init_pair(2, COLOR_GREEN, COLOR_GREEN);
  init_pair(3, COLOR_BLUE, COLOR_BLUE);
  init_pair(4, COLOR_CYAN, COLOR_CYAN);
  init_pair(5, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(6, COLOR_MAGENTA, COLOR_MAGENTA);
  newForm();
  refresh();
  int sx = current->x, sy = current->y;
  mvwin(currView, sy, sx * 2);
  box(field, 0, 0);
  wrefresh(field);
  drawScoreView();
  drawPreview();
  for (int ch; keepRunning && (ch = getch()) != 'q';) {
    logicTick(ch);
    drawField(sx, sy);
    sx = MIN(WIDTH / 2 - 6, MAX(1, current->x));
    sy = MIN(HEIGHT - 6, MAX(1, current->y));
    mvwin(currView, sy, sx * 2);
    drawField(sx, sy);
  }
  if (gameOver) {
    timeout(100000);
    mvwprintw(field, HEIGHT / 2 - 1, WIDTH / 2 - 4, "GAME OVER");
    mvwprintw(field, HEIGHT / 2, WIDTH / 2 - 5, "score: %d", score);
    mvwprintw(field, HEIGHT / 2 + 1, WIDTH / 2 - 8, "[press any button]");
    wrefresh(field);
    sleep(1);
    getch();
  }
  // cleanup
  for (ToFree *i = freeing; i != NULL;) {
    free(i->tofree);
    ToFree *k = i;
    i = i->next;
    free(k);
  }
  delwin(scoreView);
  delwin(currView);
  delwin(field);
  delwin(preview);
}
