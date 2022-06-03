#include "graph.h"
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
//min heap implementation
typedef struct vec2i{
  int x, y;
  int distance;
  float score;
  int prev_x, prev_y;
} vec2i;

static vec2i* newMinHeap(vec2i initial){
  vec2i* res = (vec2i*)malloc(sizeof(vec2i));
  *res = initial;
  return res;
}
static void seepUp(vec2i* arr, int i){
  for(int parent = (i-1)/2; arr[i].score < arr[parent].score && i > 0;){
    //swap(i, parent)
    vec2i tmp = arr[parent];
    arr[parent] = arr[i];
    arr[i] = tmp;
    //update indices
    i = parent;
    parent = (i-1)/2;
  }
}
static void seepDown(vec2i* arr, int size, int i){
  int c1, c2;
  double c1v, c2v;
  while(1){
    c1 = 2*i + 1;
    c2 = 2*i + 2;
    c1v = c1 < size ? arr[c1].score : FLT_MAX;
    c2v = c2 < size ? arr[c2].score : FLT_MAX;
    vec2i tmp = arr[i];
    //check if c1 is smaller
    if(c1v < arr[i].score && c1v <= c2v){
      //swap(i, c1)
      arr[i] = arr[c1];
      arr[c1] = tmp;
      i = c1;
    //check if c2 is larger
    }else if(c2v < arr[i].score){
      //swap(i, c2)
      arr[i] = arr[c2];
      arr[c2] = tmp;
      i = c2;
    }else break;
  }
}
static void enqueue(vec2i** arr, int* size, vec2i el){
  *arr = realloc(*arr, sizeof(vec2i) * (++(*size)));
  vec2i* a = *arr;
  a[(*size) - 1] = el;
  seepUp(a, (*size) - 1);
}
static void pop(vec2i* arr, int* size, vec2i* res){
  res->score = arr[0].score;
  res->distance = arr[0].distance;
  res->x = arr[0].x;
  res->y = arr[0].y;
  res->prev_x = arr[0].prev_x;
  res->prev_y = arr[0].prev_y;
  arr[0] = arr[--(*size)];
  seepDown(arr, *size, 0);
}
static void update(vec2i* arr, int size, int y, int x, int distance, double score){
    vec2i* p = arr;
    for(int k = 0; k < size; p++, k++){
      if(p->x == x && p->y == y){
          double old = p->score;
          p->score = score;
          p->distance = distance;
          if(old > score) seepUp(arr, k);
          else seepDown(arr, size, k);
          return;
      }
    }
}
int isVisitable(char c){
  return c == ' ' || c == 'p' || c == '.' || c == 'q' || c == '*' || c == 'r' || c == 's' || c == 'u' || c == 't';
}
/**
 * A star algorithm
 */
Way aStar(int starty, int startx, int goaly, int goalx, char* pacmanField, int height, int width, int prevDir){
  vec2i* allnodes = calloc(height * width, sizeof(vec2i));
  char* visited = calloc(height * width, sizeof(char));
  //init
  vec2i start = {.distance = 0, .x = startx, .y = starty, .score = 0, .prev_x = -1, .prev_y = -1};
  allnodes[starty * width + startx] = start;
  vec2i* queue = newMinHeap(start);
  int size = 1;
  Way res;
  res.way  = NULL;
  res.size = 0;
  //shortest straight distance
  int shortestidx = starty * width + startx;
  double shortestdis;
  {
    int xdiff = goalx - startx, ydiff = goaly - starty;
    shortestdis = sqrt(xdiff*xdiff + ydiff*ydiff);
  }
  while(size > 0){
    vec2i curr;
    pop(queue, &size, &curr);
    if(curr.x == goalx && curr.y == goaly){
BUILD_WAY_FROM_NODE:
      curr = allnodes[curr.y * width + curr.x]; //to have newest predecessor
      //determine size
      long nodecount = 0;
      for(vec2i i = curr; i.prev_x >= 0;){
        i = allnodes[i.prev_y*width + i.prev_x];
        nodecount++;
      }
      //build way
      res.way = malloc(nodecount * 2 * sizeof(int));
      res.size = nodecount;
      int index = (nodecount - 1) * 2;
      for(vec2i i = curr; i.prev_x >= 0;){
        res.way[index] = i.y;
        res.way[index + 1] = i.x;
        i = allnodes[i.prev_y*width + i.prev_x];
        index -= 2;
      }
      break;
    }
    //first we add the node to the closed set
    visited[curr.y * width + curr.x] = 1;
    allnodes[curr.y * width + curr.x] = curr;
    //get neighbours from pacman field
    const int offsets[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for(int i = 0; i < 4; i++){
      //check if it would't mean a 180 degree turn
      if(prevDir != 0){
        //   left and right           right and left              up and down             down and up
        if(i == 2 && prevDir == 4 || i == 3 && prevDir == 2 || i == 0 && prevDir == 3 || i == 1 && prevDir == 1)
          continue;
      }else{
        int diffPx = curr.x - curr.prev_x, diffPy = curr.y - curr.prev_y;
        //teleporting
        if(diffPx > 1) diffPx = -1;
        if(diffPx < -1) diffPx = 1;
        if(diffPx != 0 && diffPx == -offsets[i][1] || diffPy != 0 && diffPy == -offsets[i][0]) continue;
      }
      int cy = curr.y + offsets[i][0];
      int cx = curr.x + offsets[i][1];
      if(cy < height && cy >= 0){
        if(cx >= width)
          cx = 0;
        if(cx < 0)
          cx = width - 1;
        //check if visitable
        int doVisit = 1;
        for(int off = -1; off <= 1; off+=2){
          int nx = cx + off;
          if(nx < width && nx >= 0 && !isVisitable(pacmanField[cy*width + nx])){
            doVisit = 0;
            break;
          }
        }
        if(!doVisit) continue;
        if(cx < 0 || cy < 0 || cx >= width || cy >= height) continue;
        if(!isVisitable(pacmanField[cy*width + cx])) continue;
        //check if already visited
        if(visited[cy * width + cx]) continue;
        int newdis = curr.distance + 1;
        int xdiff = goalx - cx, ydiff = goaly - cy;
        double airdis = sqrt(xdiff * xdiff + ydiff * ydiff);
        double newscore = newdis + airdis;
        if(airdis < shortestdis){
          shortestdis = airdis;
          shortestidx = cy * width + cx;
        }
        //check if already present
        vec2i* el = &allnodes[cy * width + cx];
        //if not present
        if(el->distance == 0){ //legit check, since the first node is already visited and only one with distance = 0
          el->x = cx;
          el->y = cy;
          el->distance = newdis;
          el->score = newscore;
          el->prev_x = curr.x;
          el->prev_y = curr.y;
          enqueue(&queue, &size, *el);
        }else{ //if present
          if(el->score > newscore){ //update when better
            el->distance = newdis;
            el->score = newscore;
            el->prev_x = curr.x;
            el->prev_y = curr.y;
            update(queue, size, el->y, el->x, newdis, newscore);
          }
        }
      }
    }
    prevDir = 0;
    if(size == 0){ //cant find a way
        curr = allnodes[shortestidx];
        goto BUILD_WAY_FROM_NODE;
    }
  }
  free(queue);
  free(allnodes);
  free(visited);
  return res;
}
