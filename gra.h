#ifndef GRA_H
#define GRA_H

#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <ncurses.h>

#define MAX_UNITS 100
#define MAX_ROWS 100
#define MAX_COLUMNS 100

typedef struct {
    int id;
    char ownerType;
    char type;
    int x;
    int y;
} Unit;

pthread_mutex_t lock;
volatile int inputFlag;
pthread_t timerThread;
int currentPlayer;
long goldAmount;

void* timerFunction(void *arg);
void saveStatus(Unit units[], int unitCount);
void clearMap(char map[][MAX_COLUMNS], int numRows, int numColumns);
int generateUniqueID();
void createUnit(char ownerType, char unitType, int baseID, Unit units[], int *unitCount);
void moveUnit(int unitID, int x, int y, Unit units[], int unitCount, char map[][MAX_COLUMNS], int numRows, int numColumns);
void saveMap(char map[][MAX_COLUMNS], int numRows, int numColumns);
void updateMap(Unit units[], int unitCount, char map[][MAX_COLUMNS], int numRows, int numColumns);
void processCommand(char commandType, char ownerType, int firstArg, char unitType, int x, int y, Unit units[], int *unitCount, int *activePlayer, char map[][MAX_COLUMNS], int numRows, int numColumns);

#endif
