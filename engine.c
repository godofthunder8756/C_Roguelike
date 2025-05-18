#include "engine.h"

char world[HEIGHT][WIDTH];
int collisionMap[HEIGHT][WIDTH];
enemy* enemyList[20];
int enemyCount = 0;
inventory playerInventory;
int turnCount = 0;

//Definitions---------------------------------------------------------------//
void turn(){
    turnCount++;
}

void clearscreen()
{
    HANDLE hOut;
    COORD Position;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    Position.X = 0;
    Position.Y = 0;
    SetConsoleCursorPosition(hOut, Position);
}

void drawMap(int playerX, int playerY){
    clearscreen();
    printf("\033[93m                Valdmir!\n");
    printf("\033[96mItems: \n");
    printf("Enemy Count: %d\n", enemyCount); //Debug
    //show inventory items
    for(int i=0; i<playerInventory.size; i++){
        printf("%s", (char*)(playerInventory.contents->icon));
    }
    printf("\n");

    // Draw the game world
    for (int x = 0; x < HEIGHT; x++) {
        for (int y = 0; y < WIDTH; y++) {
            if(world[x][y] == '@')                          // Player
                printf("\033[33m%c ", world[x][y]);
            if(world[x][y] == 'w')                          // Wall
                printf("\033[100m  \033[40m");
            if(world[x][y] == '.')                          // Walkable Floor
                printf("\033[47m  ");
            if(world[x][y] == 'G')
                printf("\033[92mG ");
        }
        printf("\n\033[40m");
    }
    printf("Enemy List: \n");
    for(int i=0; i < enemyCount; i++)
        printf("%s\n", enemyList[i]->name);
}

void initColor(){
    //Sets up Color
    system("setup.bat");
    system("cls");
}

void generateCollisionFile(){ //debug
    FILE *collisions;
    collisions = fopen("collisions.txt","w");
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            fprintf(collisions, "%d ", collisionMap[i][j]);
        }
        fprintf(collisions, "\n");
    }
    fclose(collisions);
}

void generateCollisionMap(FILE* fptr){
    char c;
    rewind(fptr);
    for (int x = 0; x < HEIGHT; x++) {
        for (int y = 0; y < WIDTH; y++) {
            c = fgetc(fptr);
            if(c == 'w')
                collisionMap[x][y] = 1;
            if(c == '0')
                collisionMap[x][y] = 0;
            if(c == 'G')
                collisionMap[x][y] = 1;
        }
    }
    generateCollisionFile(); //debug
}

void initEnemy(char type, int x, int y){
    if(type == 'G'){
        enemy* Goblin;
        Goblin = (enemy*)malloc(sizeof(enemy));
        Goblin->x = x;
        Goblin->y = y;
        Goblin->icon = 'G';
        //strcpy(Goblin->name, "Goblin");
        char* gName = "Goblin";
        Goblin->name = gName;
        enemyCount++;
        enemyList[enemyCount-1] = Goblin;
    }
        
}

void initLevel(FILE* fptr){
    char c;
    enemyCount = 0;
    for (int x = 0; x < HEIGHT; x++) {
        for (int y = 0; y < WIDTH; y++) {
            c = fgetc(fptr);
            if(c == 'w')
                world[x][y] = 'w';
            if(c == '0')
                world[x][y] = '.';
            if(c == 'G'){
                world[x][y] = 'G';
                initEnemy('G', x, y);
            }
                
        }
    }
    generateCollisionMap(fptr);
}
