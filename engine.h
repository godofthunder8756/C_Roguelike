#ifndef ENGINE_H
#define ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <Windows.h>
#include "enemy.h"

#define HEIGHT 14
#define WIDTH 20

extern char world[HEIGHT][WIDTH];
extern int collisionMap[HEIGHT][WIDTH];
extern enemy* enemyList[20];
extern int enemyCount;

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

extern inventory playerInventory;
extern int turnCount;

// Function prototypes
void turn();
void clearscreen();
void drawMap(int playerX, int playerY);
void initColor();
void generateCollisionFile();
void generateCollisionMap(FILE* fptr);
void initEnemy(char type, int x, int y);
void initLevel(FILE* fptr);

#endif /* ENGINE_H */