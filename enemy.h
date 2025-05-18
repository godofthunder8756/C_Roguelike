#ifndef ENEMY_H
#define ENEMY_H

#include <stdlib.h>
#include <stdio.h>

typedef struct enemy {
    int health;
    char icon;
    char* name;
    int x;
    int y;
} enemy;

// Function declaration only - implementation will be in enemy.c
int followPlayer(int* collisionMap, int enemyX, int enemyY, int playerPosX, int playerPosY);

#endif /* ENEMY_H */
