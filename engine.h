#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <Windows.h>

#define width 20
#define height 20

char world[width][height];
int collisionMap[width][height];

typedef struct player{
    int health;
    int maxhealth;
    int level;
}player;

typedef struct item{
    char *name;
    char *icon;
    int value;
    double weight;

}item;

typedef struct inventory{
    item* contents;
    int size;
}inventory;

inventory playerInventory;

int turnCount=0;


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
    // system("cls");
    clearscreen();
    printf("\033[93m                Valdmir!\n");
    printf("\033[96mItems: ");
    //show inventory items
    for(int i=0; i<playerInventory.size; i++){
        printf((char*)(playerInventory.contents->icon));
    }
    printf("\n");

    // Draw the game world
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if(world[x][y] == '@')                          // Player
                printf("\033[33m%c ", world[x][y]);
            if(world[x][y] == 'w')                          // Wall
                printf("\033[100m  \033[40m", world[x][y]);
            if(world[x][y] == '.')                          // Walkable Floor
                printf("\033[47m  ", world[x][y]);
        }
        printf("\n\033[40m");
    }
}

void initColor(){
    //Sets up Color
    system("setup.bat");
    system("cls");
}

void generateCollisionFile(){ //debug
    FILE *collisions;
    collisions = fopen("collisions.txt","w");
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            fprintf(collisions, "%d ", collisionMap[i][j]);
        }
        fprintf(collisions, "\n");
    }
    fclose(collisions);
}

void generateCollisionMap(FILE* fptr){
    char c;
    rewind(fptr);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            c = fgetc(fptr);
            if(c == 'w')
                collisionMap[x][y] = 1;
            if(c == '0')
                collisionMap[x][y] = 0;
        }
    }
    generateCollisionFile(); //debug
}

void initWorld(FILE* fptr){
    char c;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            c = fgetc(fptr);
            if(c == 'w')
                world[x][y] = 'w';
            if(c == '0')
                world[x][y] = '.';
        }
    }
    generateCollisionMap(fptr);
}

