#ifndef SAVE_H
#define SAVE_H
typedef struct Property{
    char* key;
    void* value;
} Property;

typedef struct GameValues{
    char* gameName;
    int number;
    Property* properties;
} GameValues;

typedef struct SafeFile{
    int exists;
    int number;
    GameValues* games;
} SafeFile;

//searches for possible save files or creates them
SafeFile* loadSafeFile();
//writes every change of the save file to the path it was loaded from
void updateSafeFile(SafeFile*);
//frees any memory associated with this save file
void freeSaveFile(SafeFile*);
//finds the values of the given game, null if it does not exist
GameValues* getGame(SafeFile*, const char*);
//creates a game in the safeFile
void createGame(SafeFile*, const char*);
//finds the property with the given name for the given game and interprets it as an integer,
int getIntValue(GameValues*, const char*);
//finds the property with the given name for the given game and interprets it as an string
char* getStringValue(GameValues*, const char*);
//finds the property with the given name for the given game and interprets it as an double
double getDoubleValue(GameValues*, const char*);
//creates the property with the given name
void storeIntValue(GameValues*, const char*, int);
//finds the property with the given name
void storeStringValue(GameValues*, const char*, char*);
//finds the property with the given name
void storeDoubleValue(GameValues*, const char*, double);


#endif