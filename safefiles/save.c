#include "save.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#if defined(__unix__) || defined(__unix) || \
        (defined(__APPLE__) && defined(__MACH__))
    #define PLATFORM_UNIX
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
#else
    #if defined(_WIN32)
        #define PLATFORM_WINDOWS
    #endif
#endif
static char* readAndMallocString(FILE* file){
    char read[1024];
    int index = 0;
    do{
        read[index] = fgetc(file);
    }while(read[index++] != '\0' && read[index++] != EOF);
    char* foo = malloc(index);
    memcpy(foo, &read[0], index);
    return foo;
}
void findPath(char** dir, char** file){
    struct passwd* pw = getpwuid(getuid());
    const char* homedir = pw->pw_dir;

    char* append = "/.config/minigames-ncurses/";
    int lenhome = strlen(homedir);
    char* path = (char*)malloc(lenhome + strlen(append));
    memcpy(path, homedir, lenhome);
    strcat(path, append);
    if(dir != NULL) *dir = path;
    if(file != NULL){
        char* fpath = (char*)malloc(lenhome + strlen(append) + 6);
        memcpy(fpath, path, lenhome + strlen(append));
        strcat(fpath, "config");
        *file = fpath;
    }
}
SafeFile* loadSafeFile(){
    #ifdef PLATFORM_UNIX
    char* dir, *path;
    findPath(&dir, &path);
    struct stat st = {0};
    if(stat(dir, &st) == -1) {
        mkdir(dir, 0700);
    }
    FILE* file = fopen(path, "r");
    SafeFile* safeFile = (SafeFile*) malloc(sizeof(SafeFile));
    safeFile->exists = file != NULL;
    safeFile->number = 0;
    safeFile->games = NULL;
    //how can this NOT segfault?
    int numberOfGames = 0;
    if(file != NULL && fread(&numberOfGames, 4, 1, file) != 0){
        safeFile->games = (GameValues*) malloc(sizeof(GameValues) * numberOfGames);
        safeFile->number = numberOfGames;
        for(int g = 0; g < numberOfGames; g++){
            //read string from file
            GameValues* curr = &safeFile->games[g];
            curr->gameName = readAndMallocString(file);
            if(fread(&(curr->number), 4, 1, file) == 0) goto EXIT_POINT;
            curr->properties = (Property*) malloc(sizeof(Property) * curr->number);
            for(int p = 0; p < curr->number; p++){
                Property* prop = &curr->properties[p];
                prop->key = readAndMallocString(file);
                prop->value = (void*)readAndMallocString(file); //we read it as an char array because char ist just 1 byte
            }
        }
    }//else this file is empty or could not be created and i dont care further
EXIT_POINT:
    if(file != NULL) fclose(file);
    #endif
    free(path);
    //TODO windows
    return safeFile;
}
void freeSaveFile(SafeFile* file){
    for(int g = 0; g < file->number; g++){
        GameValues* gv = &file->games[g];
        for(int p = 0; p < gv->number; p++){
            Property* prop = &gv->properties[p];
            free(prop->key);
            free(prop->value);
        }
        free(gv->properties);
        free(gv->gameName);
    }
    free(file->games);
    free(file);
}
void updateSafeFile(SafeFile* sf){
    char* path = "~/.config/minigames-ncurses/highscores";
    findPath(NULL, &path);
    FILE* file = fopen(path, "w");
    if(file != NULL){
        fwrite(&sf->number, 4, 1, file);
        for(int g = 0; g < sf->number; g++){
            GameValues* gv = &sf->games[g];
            fwrite(gv->gameName, 1, strlen(gv->gameName), file);
            fwrite(&gv->number, 4, 1, file);
            for(int p = 0; p < gv->number; p++){
                Property* prop = &gv->properties[p];
                fwrite(&prop->key, 1, strlen(prop->key), file);
                fwrite(&prop->value, 1, strlen(prop->value), file);
            }
        }
        fclose(file);
    }
}
GameValues* getGame(SafeFile* sf, const char* name){
    for (int i = 0; i < sf->number; i++) {
        GameValues* vals = &sf->games[i];
        if(strcmp(vals->gameName, name) == 0){
            return vals;
        }
    }
    return NULL;
}

void createGame(SafeFile* sf, const char* name){
    sf->number++;
    if(sf->number == 1) sf->games = (GameValues*) malloc(sizeof(GameValues));
    else sf->games = (GameValues*) realloc(sf->games, sf->number * sizeof(GameValues));
    GameValues* val = (GameValues*) &sf->games[sf->number-1];
    memcpy(val->gameName, name, strlen(name));
    val->number = 0;
    val->properties = NULL;
}

int getIntValue(GameValues* gv, const char* name){
    for (int i = 0; i < gv->number; i++) {
        if(strcmp(gv->properties[i].value, name) == 0)
            return *((int*)gv->properties[i].value);
    }
    return 0;
}

char* getStringValue(GameValues* gv, const char* name){
    for (int i = 0; i < gv->number; i++) {
        if(strcmp(gv->properties[i].value, name) == 0)
            return (char*)gv->properties[i].value;
    }
    return 0;
}

double getDoubleValue(GameValues* gv, const char* name){
    for (int i = 0; i < gv->number; i++) {
        if(strcmp(gv->properties[i].value, name) == 0)
            return *((double*)gv->properties[i].value);
    }
    return 0;
}

void storeIntValue(GameValues* gv, const char* name, int val){
    int found = 0;
    for (int i = 0; i < gv->number; i++) {
        if(strcmp(gv->properties[i].value, name) == 0){
            if(gv->properties[i].value == NULL)
                gv->properties[i].value = malloc(sizeof(int));
            *((int*)gv->properties[i].value) = val;
            found = 1;
            break;
        }
    } 
    if(!found){
        gv->number++;
        if(gv->number == 1) gv->properties = malloc(sizeof(Property));
        else gv->properties = realloc(gv->properties, sizeof(Property) * gv->number);
        Property* prop = &gv->properties[gv->number - 1];
        int size = strlen(name);
        prop->key = malloc(size);
        memcpy(prop->key, name, size);
        prop->value = malloc(4);
        *((int*)prop->value) = val;
    }
}

void storeStringValue(GameValues* gv, const char* name, char* val){
    int found = 0;
    int valsize = strlen(val);
    for (int i = 0; i < gv->number; i++) {
        if(strcmp(gv->properties[i].value, name) == 0){
            if(gv->properties[i].value != NULL)
                free(gv->properties[i].value);
            gv->properties[i].value = malloc(valsize);
            memcpy(gv->properties[i].value, val, valsize);
            found = 1;
            break;
        }
    } 
    if(!found){
        gv->number++;
        if(gv->number == 1) gv->properties = malloc(sizeof(Property));
        else gv->properties = realloc(gv->properties, sizeof(Property) * gv->number);
        Property* prop = &gv->properties[gv->number - 1];
        int size = strlen(name);
        prop->key = malloc(size);
        memcpy(prop->key, name, size);
        prop->value = malloc(valsize);
        memcpy(prop->value, val, valsize);
    }
}

void storeDoubleValue(GameValues* gv, const char* name, double val){
    int found = 0;
    for (int i = 0; i < gv->number; i++) {
        if(strcmp(gv->properties[i].value, name) == 0){
            if(gv->properties[i].value == NULL)
                gv->properties[i].value = malloc(sizeof(double));
            *((double*)gv->properties[i].value) = val;
            found = 1;
            break;
        }
    } 
    if(!found){
        gv->number++;
        if(gv->number == 1) gv->properties = malloc(sizeof(Property));
        else gv->properties = realloc(gv->properties, sizeof(Property) * gv->number);
        Property* prop = &gv->properties[gv->number - 1];
        int size = strlen(name);
        prop->key = malloc(size);
        memcpy(prop->key, name, size);
        prop->value = malloc(4);
        *((double*)prop->value) = val;
    }
}