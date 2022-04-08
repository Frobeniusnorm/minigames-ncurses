#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#define WIDTH 50
#define HEIGHT 40
#
typedef struct Block{
	short x, y;
	short color;
} Block;
typedef struct Form{
	Block blocks[4];
	int x, y;
	int type;
	int rotation;
	int color;
} Form;
static WINDOW *field, *preview;
static Form* current;
static Block* data[WIDTH/2][HEIGHT];
static void setBlock(Block* b, const int y, const int x){
	data[x][y] = b;
	b->x = x;
	b->y = y;
}
/*
  ##
######
*/
static void drawFigure1(Form* form){
	int x = form->x;
	int y = form->y;
	int rotation = form->rotation;
	int blockidx = 0;
	switch(rotation){
		case 0:
			setBlock(&form->blocks[blockidx++], y, x + 1);
			for(int i = 0; i < 3; i++)
				setBlock(&form->blocks[blockidx++], y + 1, x + i);
			break;
		case 1:
			setBlock(&form->blocks[blockidx++], y + 1, x + 1);
			for(int i = 0; i < 3; i++)
				setBlock(&form->blocks[blockidx++], y + i, x);
			break;
		case 2:
			setBlock(&form->blocks[blockidx++], y + 1, x + 1);
			for(int i = 0; i < 3; i++)
				setBlock(&form->blocks[blockidx++], y, x + i);
			break;
		case 3:
			setBlock(&form->blocks[blockidx++], y + 1, x);
			for(int i = 0; i < 3; i++)
				setBlock(&form->blocks[blockidx++], y + i, x + 1);
			break;
	}
}
/*
########
*/
static void drawFigure2(Form* form){
	int x = form->x;
	int y = form->y;
	int rotation = form->rotation;
	int blockidx = 0;
	int start = 0;
	switch(rotation){
		case 0: 
			start = 1;
		case 2:
			for(int i = 0; i < 4; i++)
				setBlock(&form->blocks[blockidx++], y + 2, x + i+start);
			break;
		case 1:
			start = 1;
		case 3:
			for(int i = 0; i < 4; i++)
				setBlock(&form->blocks[blockidx++], y + i+start, x + 2);
			break;
	}
}
static void drawField(){
	printf("draw field\n");
	wclear(field);
	box(field, 0, 0);
	int currcol = 1;
	wattron(field, COLOR_PAIR(currcol));
	for(int x = 0; x < WIDTH/2; x++){
		for(int y = 0; y < HEIGHT; y++){
			if(data[x][y]){
				if(currcol != data[x][y]->color){
					wattroff(field, COLOR_PAIR(currcol));
					currcol = data[x][y]->color;
					wattron(field, COLOR_PAIR(currcol));
				}
				mvwaddch(field, data[x][y]->y, data[x][y]->x * 2, ' ');
				mvwaddch(field, data[x][y]->y, data[x][y]->x * 2 + 1, ' ');
			}
		}
	}
	wattroff(field, COLOR_PAIR(currcol));
	wrefresh(field);
}
static void drawPreview(){
	printf("draw previous\n");
	wclear(preview);
	box(preview, 0, 0);
	
	wrefresh(preview);
}
static void drawFigure(){
	printf("draw figure\n");
	for(int i = 0; i < 4; i++){
		Block* b = &current->blocks[i];
		data[b->x][b->y] = NULL;
	}
	switch(current->type){
		case 1:
			drawFigure1(current);
			break;
		case 2:
			drawFigure2(current);
			break;
	}
}
static void logicTick(int c){
	printf("Logic tick\n");
	//int mvx = 0, mvy = 0;
	switch(c){
		case KEY_UP:
			current->rotation = (current->rotation + 1) % 4;
			break;
		case KEY_LEFT:
			break;
	}
	drawFigure();
}

static int keepRunning = 1;
void runTetris(int maxscore){
	clear();
	field = newwin(HEIGHT, WIDTH, 0, 0);
	preview = newwin(20, 20, 0, WIDTH + 1);
	Form b = {.type = 2, .color = 1, .x = 9, .y = 5};
	current = &b;
	for(int i = 0; i < 4; i++){
		b.blocks[i].x = b.x;
		b.blocks[i].y = b.y;
		b.blocks[i].color = b.color;
	}
	for (int i = 0; i < WIDTH/2; i++)
		for (int j = 0; j < HEIGHT; j++)
			data[i][j] = NULL;

	timeout(70);
	init_pair(1, COLOR_RED, COLOR_RED);
	refresh();
	for(int ch; keepRunning && (ch = getch()) != 'q';){
		logicTick(ch);
		drawField();
		drawPreview();
	}
	//cleanup
	
	delwin(field);
	delwin(preview);
}