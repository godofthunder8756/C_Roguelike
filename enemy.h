//#include "engine.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct enemy{
    int health;
    char icon;
    char* name;
    int x;
    int y;
}enemy;

int followPlayer(int* collisionMap, int enemyX, int enemyY, int playerPosX, int playerPosY){
    int c;

}
/*
int moveRandom(int* collisionMap, int enemyX, int enemyY, int playerPosX, int playerPosY){
    int direction = rand()%4;
    if(direction == 0)
        int f;
    if(direction == 1)
        int k;
    if(direction == 2)
        int r;
    if(direction == 3)
        int c;
}
*/
