
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <ncurses.h>

#define MAX_NUM_BASES 10
#define MAX_UNITS 100
#define MAX_ROWS 100
#define MAX_COLUMNS 100
typedef struct {
    int baseID;
    int x;
    int y;
} BaseCoordinate;

BaseCoordinate baseCoordinates[MAX_NUM_BASES];  // Zakładamy, że jest maksymalna liczba baz.
int numBases = 0;
typedef struct {
    int id;
    char ownerType;
    char type;
    int x;
    int y;
} Unit;
volatile int totalTurns = 0;

pthread_mutex_t lock;
volatile int inputFlag = 0;
pthread_t timerThread;
int currentPlayer = 1;
long goldAmount = 2500;

void* timerFunction(void *arg) {
    struct timeval tp;
    while(1) {
        sleep(5);
        pthread_mutex_lock(&lock);
        if (inputFlag == 0) {
            printf("Twój czas minął, gracz nr %d wykonuje ruch.\n", currentPlayer == 1 ? 2 : 1);
            currentPlayer = currentPlayer == 1 ? 2 : 1;
            totalTurns++;
            if (totalTurns >= 10) {
                printf("Gra zakończona po %d turach.\n", totalTurns);
                exit(0);
            }
        }
        pthread_mutex_unlock(&lock);
        if (inputFlag != 0) {
            break;
        }
    }
    return NULL;
}


void saveStatus(Unit units[], int unitCount) {
    FILE *file;

    file = fopen("status.txt", "w");
    if (file == NULL) {
        printf("Nie można otworzyć pliku status.txt\n");
        return;
    }

    fprintf(file, "%ld\n", goldAmount);

    for (int i = 0; i < unitCount; i++) {
        fprintf(file, "%c %c %d %d %d %d\n", units[i].ownerType, units[i].type, units[i].id, units[i].x, units[i].y, 100);
    }

    fclose(file);
}

void clearMap(char map[][MAX_COLUMNS], int numRows, int numColumns) {
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numColumns; j++) {
            if (map[i][j] != '9' && map[i][j] != '1' && map[i][j] != '2' && map[i][j] != '6') {
                map[i][j] = '0';
            }
        }
    }
}

int generateUniqueID() {
    static int id = 1;
    return id++;
}

void createUnit(char ownerType, char unitType, int baseID, Unit units[], int *unitCount) {
    // Sprawdzenie czy baza już produkuje jednostkę tego typu
    for (int i = 0; i < *unitCount; i++) {
        if (units[i].id == baseID && units[i].type == unitType) {
            printf("Polecenie budowy jednostki podczas gdy baza jest już zajęta produkcją. Gracz zdyskwalifikowany!\n");
            exit(0);
        }
    }
    if(unitType != 'K' && unitType != 'S' && unitType != 'A' && unitType != 'P' && unitType != 'C' && unitType != 'R' && unitType != 'W' && unitType != '1' && unitType != '2') {
        printf("Podano nieprawidłowy typ jednostki. Gracz zdyskwalifikowany!\n");
        if (currentPlayer == 1)
        {
            currentPlayer = 2; 

        }
        else if (currentPlayer == 2)
        {
            currentPlayer = 1; 
        }
        printf("Gracz %d zwycięża !\n",  currentPlayer);
        exit(0);
    }
    int unitID = generateUniqueID();
    Unit unit;
    unit.id = unitID;
    unit.ownerType = ownerType;
    unit.type = unitType;
    unit.x = -1;  // Inicjalizacja jednostki poza mapą
    unit.y = -1;
    units[*unitCount] = unit;
    (*unitCount)++;
    printf("Utworzono jednostkę typu %c o ID %d w bazie o ID %d\n", unitType, unitID, baseID);
    saveStatus(units, *unitCount);
}


void moveUnit(int unitID, int x, int y, Unit units[], int unitCount, char map[][MAX_COLUMNS], int numRows, int numColumns) {
    // Sprawdzanie czy jednostka o podanym ID istnieje
    int unitExists = 0;
    for (int i = 0; i < unitCount; i++) {
        if (units[i].id == unitID) {
            unitExists = 1;
            break;
        }
    }
 int index = -1;
    for (int i = 0; i < unitCount; i++) {
        if (units[i].id == unitID) {
            index = i;
            break;
        }
    }

    // Sprawdzanie, czy jednostka istnieje
    if (index == -1) {
        printf("Jednostka o podanym ID nie istnieje. Gracz zdyskwalifikowany!\n");
        exit(0);
    }

    // Sprawdzanie, czy jednostka jest bazą
    if (units[index].type == 'B') {
        printf("Próba przesunięcia bazy. Gracz zdyskwalifikowany!\n");
        exit(0);
    }
    if (!unitExists) {
        printf("Jednostka o ID %d nie istnieje. Gracz zdyskwalifikowany!\n", unitID);
        exit(0);
    }

    // Sprawdzanie czy współrzędne docelowe są zgodne z punktami ruchu
    if (x < 0 || x >= numColumns || y < 0 || y >= numRows) {
        printf("Współrzędne docelowe niezgodne z punktami ruchu. Gracz zdyskwalifikowany!\n");
        exit(0);
    }

    // Sprawdzanie czy w miejscu docelowym nie ma obcej jednostki lub przeszkody
    if (map[y][x] != '0') {
        printf("W miejscu docelowym jest obca jednostka lub przeszkoda. Gracz zdyskwalifikowany!\n");
        exit(0);
    }

    for (int i = 0; i < unitCount; i++) {
        if (units[i].id == unitID) {
            units[i].x = x;
            units[i].y = y;
            printf("Jednostka o ID %d przemieściła się do pozycji (%d, %d)\n", unitID, x, y);
            clearMap(map, numRows, numColumns);
            for (int j = 0; j < unitCount; j++) {
                int unitX = units[j].x;
                int unitY = units[j].y;
                if (unitX >= 0 && unitX < numColumns && unitY >= 0 && unitY < numRows) {
                    map[unitY][unitX] = units[j].type;
                }
            }
            saveStatus(units, unitCount);
            return;
        }
    }
    printf("Nie znaleziono jednostki o ID %d\n", unitID);
}

void saveMap(char map[][MAX_COLUMNS], int numRows, int numColumns) {
    FILE *file = fopen("mapa.txt", "w");
    if (file == NULL) {
        printf("Nie można otworzyć pliku mapa.txt\n");
        return;
    }

    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numColumns; j++) {
            fprintf(file, "%c", map[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

void updateMap(Unit units[], int unitCount, char map[][MAX_COLUMNS], int numRows, int numColumns) {
    clearMap(map, numRows, numColumns);
    for (int i = 0; i < unitCount; i++) {
        int unitX = units[i].x;
        int unitY = units[i].y;
        if (unitX >= 0 && unitX < numColumns && unitY >= 0 && unitY < numRows) {
            map[unitY][unitX] = units[i].type;
        }
    }
    saveMap(map, numRows, numColumns);
}

void processCommand(char commandType, char ownerType, int firstArg, char unitType, int x, int y, Unit units[], int *unitCount, int *activePlayer, char map[][MAX_COLUMNS], int numRows, int numColumns) {
    switch (commandType) {
        case 'M':
            moveUnit(firstArg, x, y, units, *unitCount, map, numRows, numColumns);
            break;
        case 'B':
            createUnit(ownerType, unitType, firstArg, units, unitCount);
            break;
        case 'E':
            *activePlayer = 2;
            break;
        case 'P':
            *activePlayer = 1;
            break;
        default:
            printf("Nieznany typ rozkazu: %c\n", commandType);
    }
}

int main() {
    FILE *file;
    char buffer[100];
    pthread_mutex_init(&lock, NULL);
    Unit units[MAX_UNITS];
    int unitCount = 0;

    char map[MAX_ROWS][MAX_COLUMNS];
    int numRows = 0;
    int numColumns = 0;

    file = fopen("mapa.txt", "r");

    if (file == NULL) {
        printf("Nie można otworzyć pliku mapa.txt\n");
        return 1;
    }

    while (fgets(buffer, sizeof(buffer), file)) {
        int len = strlen(buffer) - 1;
        if (len > numColumns) {
            numColumns = len;
        }
        strncpy(map[numRows], buffer, len);
        numRows++;
    }
int base1ID, base2ID;

    // Wczytywanie mapy
   
    fclose(file);

    printf("Wczytana mapa:\n");
    for (int i = 0; i < numRows; i++) {
        printf("%s", map[i]);
    }
    printf("\n");
 for (int i = 0; i < numRows; i++) {
    for (int j = 0; j < numColumns; j++) {
        char c = map[i][j];
        if (c == '1') {
            base1ID = generateUniqueID();
            createUnit('B', '1', base1ID, units, &unitCount);
            baseCoordinates[numBases].baseID = base1ID;
            baseCoordinates[numBases].x = i;
            baseCoordinates[numBases].y = j;
            numBases++;
        } else if (c == '2') {
            base2ID = generateUniqueID();
            createUnit('B', '2', base2ID, units, &unitCount);
            baseCoordinates[numBases].baseID = base2ID;
            baseCoordinates[numBases].x = i;
            baseCoordinates[numBases].y = j;
            numBases++;
        }
    }
}
    updateMap(units, unitCount, map, numRows, numColumns);

    printf("Mapa przed wykonaniem poleceń:\n");
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numColumns; j++) {
            printf("%c", map[i][j]);
        }
        printf("\n");
    }

    char commandType;
    char ownerType;
    int firstArg;
    char unitType;
    int x, y;
    char end_mov = 'X';
    int activePlayer = 1;

    while (1) {
        printf("Gracz %d, wprowadź swoje polecenie (lub wpisz 'exit' aby zakończyć): ", activePlayer);
        inputFlag = 0;
        pthread_mutex_lock(&lock);
        currentPlayer = activePlayer;
        pthread_mutex_unlock(&lock);
        pthread_create(&timerThread, NULL, timerFunction, NULL);

        scanf(" %c", &commandType);
        pthread_mutex_lock(&lock);
        inputFlag = 1;
        pthread_mutex_unlock(&lock);

        if (commandType == end_mov) {
            break;
        }

        if (commandType == 'B') {
            scanf(" %c %d", &unitType, &firstArg);
             
            processCommand('B', (activePlayer == 1) ? 'P' : 'E', firstArg, unitType, 0, 0, units, &unitCount, &activePlayer, map, numRows, numColumns);
        } else if (commandType == 'M') {
            scanf("%d %d %d", &firstArg, &x, &y);
            processCommand('M', ' ', firstArg, ' ', x, y, units, &unitCount, &activePlayer, map, numRows, numColumns);
        }
         if (totalTurns >= 10) {
            printf("Gra zakończona po %d turach.\n", totalTurns);
            break;
        }
 pthread_mutex_lock(&lock);
        totalTurns++;
        pthread_mutex_unlock(&lock);
        printf("Mapa po wykonaniu poleceń:\n");
        for (int i = 0; i < numRows; i++) {
            for (int j = 0; j < numColumns; j++) {
                printf("%c", map[i][j]);
            }
            printf("\n");
        }

        pthread_cancel(timerThread);  // przeniesione tutaj
    pthread_join(timerThread, NULL); 
        inputFlag = 1;
        activePlayer = (activePlayer == 1) ? 2 : 1;
    }

    pthread_mutex_destroy(&lock);
    return 0;
}


