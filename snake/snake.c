#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define NUM_POINTS 5
typedef struct snake{
	int x, y; //top left border of snake box, extends one right and downward
	int firstIt;
	struct snake* tail;
} snake;
typedef struct point{
	int x, y;
} point;
static void gameover();
static char* board;
static point points[5];
static int score = 0;
static int width, height;
static snake* snake_obj;
static void newPoint(point* p){
	int placed = 0;
	while(!placed){
		int r1 = 1 + rand() % (height-2);
		int r2 = 1 + rand() % (width-2);
		if(board[r1 * width + r2] == 0){
			board[r1 * width + r2] = 2;
			placed = 1;
			p->x = r2;
			p->y = r1;
		}
	}
}
static void eatPoint(int x, int y){
	for(int i = 0; i < NUM_POINTS; i++){
		if(points[i].x == x && points[i].y == y){
			newPoint(&(points[i]));
			snake** j = &snake_obj;
			for(; *j != NULL; j = &((*j)->tail));
			*j = (snake*) malloc(sizeof(snake));
			(*j)->tail = NULL;
			(*j)->firstIt = 1;
			score++;
			break;
		}
	}
}
static void moveSnake(snake* snake, int x, int y){
	if(!snake->firstIt){

		board[snake->y * width + snake->x] = 0;
		if(x < 0) gameover();
		if(y < 0) gameover();
		if(y + 1 >= height) gameover();
		if(x + 1 >= width) gameover();
		if(board[y * width + x] == 1){
				gameover();
		}else if(board[y * width + x] == 2){
				eatPoint(x, y);
		}else if(x + 1 < width && board[y * width + x + 1] == 2){
				eatPoint(x + 1, y);
		}else if(y + 1 < height && board[y * width + x + width] == 2){
				eatPoint(x, y + 1);
		}else if(y + 1 < height && x + 1 < width && board[y * width + x + width + 1] == 2){
				eatPoint(x + 1, y + 1);
		}
		if(snake->tail)
			moveSnake(snake->tail, snake->x, snake->y);
	}else snake->firstIt = 0;
	snake->x = x;
	snake->y = y;
	board[y * width + x] = 1;
}
static void freeSnake(snake* snake){
	if(snake->tail)
		freeSnake(snake->tail);
	free(snake);
}
static snake* initSnake(int x, int y){
	snake* foo = (snake*)(malloc(sizeof(snake)));
	foo->x = x;
	foo->y = y;
	snake* curr = foo;
	for(int i = 0; i < 5; i++){
		curr->tail = (snake*)(malloc(sizeof(snake)));
		curr->tail->x = curr->x - 2;
		curr->tail->y = curr->y;
		curr->tail->firstIt = 0;
		curr = curr->tail;
	}
	curr->tail = NULL;
	return foo;
}

static WINDOW* win = NULL, *scoreboard = NULL;
static int keepRunning = 1;

static void gameover(){
	timeout(10000);
	keepRunning = 0;
}
static void drawSnake(int prev, snake* curr){
	int draw_sides[4];
	draw_sides[0] = draw_sides[1] = draw_sides[2] = draw_sides[3] = 1;
	int nprev = -1;
	if(prev > -1) //prev exists
			draw_sides[prev] = 0;
	snake* next = curr->tail;
	if(next){
		//determine current side
		int cs;
		if(curr->x > next->x) //next is on left side
			cs = 0;
		else if(curr->x < next->x) //next is on right side
			cs = 2;
		else if(curr->y > next->y) //next is on top side
			cs = 1;
		else cs = 3; //next is on bottom side
		draw_sides[cs] = 0;
		nprev = (cs + 2) % 4;
	}
	int corners = 0;
	//drawing
	if(draw_sides[0] && draw_sides[2]){ //vertical snake
		mvwvline(win, curr->y, curr->x, 0, 1);
		mvwvline(win, curr->y, curr->x+2, 0, 1);
		if(draw_sides[1]){ //additional side on top
			mvwaddch(win, curr->y, curr->x, ACS_ULCORNER);
			mvwhline(win, curr->y, curr->x+1, 0, 1);
			mvwaddch(win, curr->y, curr->x+2, ACS_URCORNER);
		}
		if(draw_sides[3]){
			mvwaddch(win, curr->y + 1, curr->x, ACS_LLCORNER);
			mvwhline(win, curr->y + 1, curr->x+1, 0, 1);
			mvwaddch(win, curr->y + 1, curr->x+2, ACS_LRCORNER);
		}
	}else if(draw_sides[1] && draw_sides[3]){ //horizontal snake
		mvwhline(win, curr->y, curr->x, 0, 2);
		mvwhline(win, curr->y + 1, curr->x, 0, 2);
		if(draw_sides[0]){ //additional side on left
			mvwaddch(win, curr->y, curr->x, ACS_ULCORNER);
			mvwaddch(win, curr->y + 1, curr->x, ACS_LLCORNER);
		}
		if(draw_sides[2]){
			mvwaddch(win, curr->y, curr->x+2, ACS_URCORNER);
			mvwaddch(win, curr->y + 1, curr->x+2, ACS_LRCORNER);
		}
	}else corners = 1;
	//recursion if needed, must be before corners, so they can overwrite
	if(next) drawSnake(nprev, next);
	//corner piece or something
	if(corners){
		//top left
		if(draw_sides[0] && draw_sides[1]){
			mvwhline(win, curr->y, curr->x, 0, 2);
			mvwvline(win, curr->y, curr->x, 0, 1);
			mvwaddch(win, curr->y, curr->x, ACS_ULCORNER);
			mvwaddch(win, curr->y+1, curr->x+2, ACS_ULCORNER);
		}
		//bottom left
		if(draw_sides[0] && draw_sides[3]){
			mvwhline(win, curr->y+1, curr->x, 0, 2);
			mvwvline(win, curr->y, curr->x, 0, 1);
			mvwaddch(win, curr->y, curr->x+2, ACS_LLCORNER);
			mvwaddch(win, curr->y+1, curr->x, ACS_LLCORNER);
		}
		//bottom right
		if(draw_sides[2] && draw_sides[3]){
			mvwhline(win, curr->y+1, curr->x, 0, 2);
			mvwvline(win, curr->y, curr->x+2, 0, 1);
			mvwaddch(win, curr->y, curr->x, ACS_LRCORNER);
			mvwaddch(win, curr->y+1, curr->x+2, ACS_LRCORNER);
		}
		//top right
		if(draw_sides[2] && draw_sides[1]){
			mvwhline(win, curr->y, curr->x, 0, 2);
			mvwvline(win, curr->y, curr->x+2, 0, 1);
			mvwaddch(win, curr->y, curr->x+2, ACS_URCORNER);
			mvwaddch(win, curr->y+1, curr->x, ACS_URCORNER);
		}
	}
}
static void drawPoints(){
	for(int i = 0; i < NUM_POINTS; i++){
		mvwaddch(win, points[i].y, points[i].x, '*');
	}
}

static int currdirx = 2, currdiry = 0;
void runSnake(int maxscore){
	timeout(70);
	refresh();
	getmaxyx(stdscr, height, width);
	init_color(COLOR_GREEN, 200, 900, 200);
	init_color(COLOR_RED, 900, 200, 200);
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_color(COLOR_BLACK, 0, 0, 0);
	init_color(COLOR_BLUE, 500, 500, 500);
	init_pair(3, COLOR_BLUE, COLOR_BLACK);
	win = newwin(height-2, width, 2, 0);
	scoreboard = newwin(2, width, 0, 0);
	//width and height now describe the win window
	height = height - 2;
	board = (char*) malloc(sizeof(char) * height * width);
	snake_obj = initSnake(width/2 + 1, height/2);
	for(int i = 0; i < 5; i++) newPoint(&(points[i]));
	box(win, 0, 0);
	wrefresh(win);
	//drawing loop
	for(int ch; keepRunning && (ch = getch()) != 'q';){
		//input handling
		int currx = snake_obj->x, curry = snake_obj->y;
		switch(ch){
			case KEY_UP:
				if(currdiry != 1){
					currdirx = 0;
					currdiry = -1;
				}
				break;
			case KEY_DOWN:
				if(currdiry != -1){
					currdirx = 0;
					currdiry = 1;
				}
				break;
			case KEY_LEFT:
				if(currdirx != 2){
					currdirx = -2;
					currdiry = 0;
				}
				break;
			case KEY_RIGHT:
				if(currdirx != -2){
					currdirx = 2;
					currdiry = 0;
				}
				break;
		}
		moveSnake(snake_obj, currx + currdirx, curry + currdiry);
		wclear(win);
		box(win, 0, 0);
		mvwprintw(scoreboard, 0, 1, "score: %d", score);
		wattron(scoreboard, COLOR_PAIR(3));
		mvwprintw(scoreboard, 0, width-14, "highscore: %d", maxscore);
		wattroff(scoreboard, COLOR_PAIR(3));
		wattron(win, COLOR_PAIR(2));
		drawPoints();
		wattroff(win, COLOR_PAIR(2));
		wattron(win, COLOR_PAIR(1));
		drawSnake(-1, snake_obj);
		wattroff(win, COLOR_PAIR(1));
		wrefresh(win);
		wrefresh(scoreboard);
	}
	move(height/2, width/2);
	mvwprintw(win, height/2-1, width/2-3, "GAME OVER");
	move(height/2, width/2);
	wattron(win, COLOR_PAIR(3));
	mvwprintw(win, height/2, width/2-7, "[press q to exit]");
	wattroff(win, COLOR_PAIR(3));
	wrefresh(win);
	sleep(1);
	while(getch() != 'q')
	;
	//cleanup
	score = 0;
	keepRunning = 1;
	delwin(win);
	freeSnake(snake_obj);
	free(board);
}
