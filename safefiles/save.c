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
#include <io.h>
#define F_OK 0
#define access _access
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
SaveFile *loadSaveFile() {
  char *dir;
  char *file;
  findPath(&dir, &file);
  if (access(file, F_OK) != 0) {
    // file does not exist, create it
    FILE *tmp = fopen(file, "a");
    if (tmp)
      fclose(tmp);
  }
  // create SaveFile
  SaveFile *save = (SaveFile *)malloc(sizeof(SaveFile));
  save->games = NULL;
  save->num_games = 0;
  GameData *curr_gd = NULL;
  int gameindx = 0, propindx = 0;
  // open file for reading
  FILE *fp = fopen(file, "r");
  if (fp) {
    for (char line[256]; fgets(line, sizeof(line), fp);) {
      int line_length = strlen(&line[0]);
      if (!save->games) { // number of games
        save->num_games = strtol(&line[0], NULL, 10);
        if (save->num_games > 0)
          save->games = (GameData *)malloc(sizeof(GameData) * save->num_games);
        else
          break;
      } else if (line[0] != ' ') { // properties start with a initial whitespace
        // start of new game
        if (save->games) {
          curr_gd = &save->games[gameindx];
          int name_length = 0;
          for (; name_length < line_length && line[name_length] != ' ';
               name_length++)
            ;
          curr_gd->gameName = malloc(name_length + 1);
          memcpy(curr_gd->gameName, &line[0], name_length);
          curr_gd->gameName[name_length] = '\0';
          curr_gd->num_properties = atoi(&line[name_length + 1]);
          if (curr_gd->num_properties > 0)
            curr_gd->properties =
                (Property *)malloc(sizeof(Property) * curr_gd->num_properties);
          gameindx++;
          propindx = 0;
        } else
          break;
      } else { // property
        if (curr_gd) {
          Property *prop = &curr_gd->properties[propindx];
          int key_length = 0;
          for (; key_length < line_length && line[key_length + 1] != ' ';
               key_length++)
            ;
          prop->key = (char *)malloc(key_length + 1);
          memcpy(prop->key, &line[1], key_length);
          prop->key[key_length] = '\0';
          // now read value
          //  first whitespace   second
          prop->value = (char *)malloc(line_length - 1 - key_length - 1);
          //\n
          memcpy(prop->value, &line[key_length + 2],
                 (line_length - 1) - (key_length + 2));
          prop->value[(line_length - 1) - (key_length + 2) - 1] = '\0';
        } else
          break;
      }
    }
  }
  fclose(fp);
  return save;
}
void updateSaveFile();
void freeSaveFile(SaveFile *file) {
  for (int g = 0; g < file->num_games; g++) {
    GameData *gd = &file->games[g];
    for (int p = 0; p < gd->num_properties; p++) {
      Property *pd = &gd->properties[p];
      free(pd->key);
      free(pd->value);
    }
    free(gd->properties);
    free(gd->gameName);
  }
  free(file->games);
  free(file);
}

GameData *findGame(SaveFile *file, char *gameName);
char *findValue(GameData *data, char *keyName);
GameData *createGame(SaveFile *file, char *gameName);
void putValue(GameData *data, char *key, char *val);
