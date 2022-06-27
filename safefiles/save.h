#ifndef SAVE_H
#define SAVE_H
struct Property{
    char* key;
    char* value;
};
struct GameData{
    char* gameName;
    Property* properties;
    int num_properties;
};
struct SaveFile{
    GameData* games;
    int num_games;
};

SaveFile* loadSaveFile();
void updateSaveFile();
void freeSaveFile(SaveFile*);

GameData* findGame(SaveFile* file, char* gameName);
char* findValue(GameData* data, char* keyName);
GameData* createGame(SaveFile* file, char* gameName);
void putValue(GameData* data, char* key, char* val);

#endif
