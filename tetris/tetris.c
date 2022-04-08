#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
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
static int writeToData = 1;
static void setBlock(Block* b, const int y, const int x){
	if(writeToData) data[x][y] = b;
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
	wclear(preview);
	box(preview, 0, 0);

	wrefresh(preview);
}
static void drawFigure(){
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
static int collisionDetection(Form* f, int mvx, int mvy){
	for(int i = 0; i < 4; i++){
		Block* b = &f->blocks[i];
		int nx = b->x + mvx, ny = b->y + mvy;
		if(nx < 1 || nx*2 >= WIDTH - 2 || ny >= HEIGHT - 1){
			return 0;
		}
		Block* db = data[nx][ny];
		if(db){
			//check if it belongs to same figure
			int same = 0;
			for(int j = 0; j < 4; j++)
				if(db == &f->blocks[j]){
					same = 1;
					break;
				}
			if(!same){
				return 0;
			}
		}
	}
	return 1;
}
static int newType = 1;
static int newColor = 1;
typedef struct ToFree{
		Form* tofree;
		struct ToFree* next;
} ToFree;
static ToFree* freeing = NULL;
static void newForm(){
	current = (Form*)malloc(sizeof(Form));
	current->type = newType;
	current->color = newColor;
	current->x = 9;
	current->y = newType == 1 ? 1 : 0;

	ToFree* nfreeing = (ToFree*)malloc(sizeof(ToFree));
	nfreeing->next = freeing;
	nfreeing->tofree = current;
	freeing = nfreeing;
	for(int i = 0; i < 4; i++){
		current->blocks[i].x = current->x;
		current->blocks[i].y = current->y;
		current->blocks[i].color = current->color;
	}
	newType = rand() % 2 + 1;
	newColor = rand() % 4 + 1;
}
static void rotate(Form* f){
	writeToData = 0;
	int oldRotation = f->rotation;
	f->rotation = (current->rotation + 1) % 4;
	drawFigure(); //rotate without writing
	int res = collisionDetection(f, 0, 0);
	if(!res)	//if it didnt work reset
		f->rotation = oldRotation;
	writeToData = 1;
	drawFigure(); //write out

}
static uint64_t lastTick = 0;
static void logicTick(int c){
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	uint64_t nanos = time.tv_nsec;

	int mvx = 0, mvy = 0;
	if(nanos - lastTick > 500000000){
		mvy = 1;
		lastTick = nanos;
	}
	switch(c){
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
			mvy = 1;
	}
	if(collisionDetection(current, mvx, 0)){
		current->x += mvx;
		drawFigure();
	}
	if(mvy > 0){
		if(collisionDetection(current, 0, mvy)){
			current->y += mvy;
			drawFigure();
		}else{
			//it is set, we can check for the lines and generate a new form
			for(int i = 0; i < 4; i++){
				int y = current->blocks[i].y;
				//did we already check for that?
				int didcheck = 0;
				for(int j = 0; j < i; j++)
					if(current->blocks[j].y == y){
						didcheck = 1;
						break;
					}
				if(!didcheck){
					int full = 1;
					for(int j = 1; j < WIDTH/2 - 2; j++)
						if(!data[j][y]){
							full = 0;
							break;
						}
					if(full){
						for(int j = 1; j < WIDTH/2 - 2; j++)
							for(int k = y; k >= 0; k--){
								data[j][k] = k == 0 ? NULL : data[j][k-1];
								if(data[j][k]) data[j][k]->y++;							}
					}
				}
			}
			newForm();
		}
	}

}

static int keepRunning = 1;
void runTetris(int maxscore){
	clear();
	field = newwin(HEIGHT, WIDTH, 0, 0);
	preview = newwin(20, 20, 0, WIDTH + 1);

	for (int i = 0; i < WIDTH/2; i++)
		for (int j = 0; j < HEIGHT; j++)
			data[i][j] = NULL;

	timeout(70);
	init_pair(1, COLOR_RED, COLOR_RED);
	init_pair(2, COLOR_GREEN, COLOR_GREEN);
	init_pair(3, COLOR_BLUE, COLOR_BLUE);
	init_pair(4, COLOR_CYAN, COLOR_CYAN);
	newForm();
	refresh();
	for(int ch; keepRunning && (ch = getch()) != 'q';){
		logicTick(ch);
		drawField();
		drawPreview();
	}
	//cleanup
	for(ToFree* i = freeing; i != NULL;){
		free(i->tofree);
		ToFree* k = i;
		i = i->next;
		free(k);
	}
	delwin(field);
	delwin(preview);
}
