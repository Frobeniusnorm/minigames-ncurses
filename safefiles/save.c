#include "save.h"
#include <math.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__unix__) || defined(__unix) ||                                    \
    (defined(__APPLE__) && defined(__MACH__))
#define PLATFORM_UNIX
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#else
#if defined(_WIN32)
#define PLATFORM_WINDOWS
#endif
#endif
static char *readAndMallocString(FILE *file) {
  char read[1024];
  int index = 0;
  do {
    read[index++] = fgetc(file);
  } while (read[index - 1] != '\0' && read[index - 1] != EOF);
  char *foo = malloc(index);
  memcpy(foo, &read[0], index);
  return foo;
}
static void findPath(char **dir, char **file) {
  struct passwd *pw = getpwuid(getuid());
  const char *homedir = pw->pw_dir;

  char *append = "/.config/minigames-ncurses/";
  int lenhome = strlen(homedir);
  char *path = (char *)malloc(lenhome + strlen(append) + 1);
  memcpy(path, homedir, lenhome + 1);
  strcat(path, append);
  if (dir != NULL)
    *dir = path;
  if (file != NULL) {
    char *fpath = (char *)malloc(lenhome + strlen(append) + 7);
    memcpy(fpath, path, lenhome + strlen(append) + 1);
    strcat(fpath, "config");
    *file = fpath;
  }
}
SaveFile* loadSaveFile();
void updateSaveFile();
void freeSaveFile(SaveFile*);

GameData* findGame(SaveFile* file, char* gameName);
char* findValue(GameData* data, char* keyName);
GameData* createGame(SaveFile* file, char* gameName);
void putValue(GameData* data, char* key, char* val);



