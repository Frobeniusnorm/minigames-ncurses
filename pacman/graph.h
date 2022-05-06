#ifndef GRAPH_H
#define GRAPH_H

typedef struct Way{
  int* way; //yx array
  int size; //number of yx pairs
} Way;

Way aStar(int starty, int startx, int goaly, int goalx, char* pacmanField, int height, int width);
int isVisitable(char c);
#endif
