#ifndef SAVE_H
#define SAVE_H
typedef struct Property {
  char *key;
  char *value;
} Property;
typedef struct GameData {
  char *gameName;
  Property *properties;
  int num_properties;
} GameData;
typedef struct SaveFile {
  GameData *games;
  int num_games;
} SaveFile;

SaveFile *loadSaveFile();
void updateSaveFile(SaveFile *);
void freeSaveFile(SaveFile *);

GameData *findGame(SaveFile *file, const char *gameName);
char *findValue(GameData *data, const char *keyName);
GameData *createGame(SaveFile *file, const char *gameName);
void putValue(GameData *data, const char *key, const char *val);

#endif
