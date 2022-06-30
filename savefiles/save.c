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
  struct stat st = {0};
  if (stat(dir, &st) == -1)
    mkdir(dir, 0700);
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
      } else if ((!curr_gd || propindx >= curr_gd->num_properties) &&
                 gameindx < save->num_games) { // properties start with a
                                               // initial whitespace
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
          else
            curr_gd->properties = NULL;
          gameindx++;
          propindx = 0;
          printf("read new game: %s\n", curr_gd->gameName);
        } else
          break;
      } else { // property
        if (curr_gd && propindx < curr_gd->num_properties) {
          Property *prop = &curr_gd->properties[propindx];
          int key_length = 0;
          for (; key_length < line_length && line[key_length] != ' ';
               key_length++)
            ;
          prop->key = (char *)malloc(key_length + 1);
          memcpy(prop->key, &line[0], key_length);
          prop->key[key_length] = '\0';
          // now read value
          prop->value = (char *)malloc(line_length - key_length - 1);
          //\n
          memcpy(prop->value, &line[key_length + 1],
                 line_length - key_length - 1);
          prop->value[line_length - key_length - 2] = '\0';
          propindx++;
          printf("read new property for game %s: (%s, %s)\n", curr_gd->gameName,
                 curr_gd->properties[propindx - 1].key,
                 curr_gd->properties[propindx - 1].value);
        } else
          break;
      }
    }
    fclose(fp);
  }
  free(file);
  free(dir);
  return save;
}
void updateSaveFile(SaveFile *save) {
  char *dir;
  char *file;
  findPath(&dir, &file);
  FILE *fp = fopen(file, "w");
  if (fp) {
    fprintf(fp, "%d\n", save->num_games);
    for (int g = 0; g < save->num_games; g++) {
      GameData *gd = &save->games[g];
      fprintf(fp, "%s %d\n", gd->gameName, gd->num_properties);
      for (int p = 0; p < gd->num_properties; p++) {
        Property *pd = &gd->properties[p];
        fprintf(fp, "%s %s\n", pd->key, pd->value);
      }
    }
    fclose(fp);
  }
  free(dir);
  free(file);
}
void freeSaveFile(SaveFile *file) {
  for (int g = 0; g < file->num_games; g++) {
    GameData *gd = &file->games[g];
    for (int p = 0; p < gd->num_properties; p++) {
      Property *pd = &gd->properties[p];
      free(pd->key);
      free(pd->value);
    }
    if (gd->properties)
      free(gd->properties);
    free(gd->gameName);
  }
  free(file->games);
  free(file);
}

GameData *findGame(SaveFile *file, const char *gameName) {
  GameData *ptr = file->games;
  for (int i = 0; i < file->num_games; i++) {
    if (strcmp(ptr[i].gameName, gameName) == 0)
      return &ptr[i];
  }
  return NULL;
}
char *findValue(GameData *data, const char *keyName) {
  Property *ptr = data->properties;
  for (int i = 0; i < data->num_properties; i++, ptr++)
    if (strcmp(ptr->key, keyName) == 0)
      return ptr->value;
  return NULL;
}
GameData *createGame(SaveFile *file, const char *gameName) {
  file->num_games++;
  if (file->num_games == 1) {
    file->games = (GameData *)malloc(sizeof(GameData));
  } else {
    file->games =
        (GameData *)realloc(file->games, sizeof(GameData) * file->num_games);
  }
  GameData *data = &file->games[file->num_games - 1];
  data->num_properties = 0;
  data->properties = NULL;
  data->gameName = (char *)malloc(strlen(gameName) + 1);
  strcpy(data->gameName, gameName);
  return data;
}
void putValue(GameData *data, const char *key, const char *val) {
  int i = 0;
  char **toSet = NULL;
  for (Property *ptr = data->properties; i < data->num_properties; i++, ptr++) {
    if (strcmp(ptr->key, key) == 0) {
      toSet = &ptr->value;
      break;
    }
  }
  if (!toSet) {
    data->num_properties++;
    if (data->num_properties == 1) {
      data->properties = (Property *)malloc(sizeof(Property));
    } else {
      data->properties = (Property *)realloc(
          data->properties, sizeof(Property) * data->num_properties);
    }
    Property *prop = &data->properties[data->num_properties - 1];
    prop->key = (char *)malloc(strlen(key) + 1);
    strcpy(prop->key, key);
    toSet = &prop->value;
  }
  *toSet = (char *)malloc(strlen(val) + 1);
  strcpy(*toSet, val);
}
