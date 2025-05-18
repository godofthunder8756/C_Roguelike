#include "enemy.h"
#include <math.h>

/**
 * Simple AI pathfinding for enemy to follow player
 * Returns the direction to move (0-3) or -1 if no valid move
 * 0 = up, 1 = right, 2 = down, 3 = left
 */
int followPlayer(int* collisionMap, int enemyX, int enemyY, int playerPosX, int playerPosY) {
    // Calculate direction to player
    int dx = playerPosX - enemyX;
    int dy = playerPosY - enemyY;
    
    // Directions: 0=up, 1=right, 2=down, 3=left
    int directions[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    
    // Try to move in the direction of the player
    if (abs(dx) > abs(dy)) {
        // Horizontal movement is priority
        if (dx > 0) {
            // Try right
            if (collisionMap[enemyY * 20 + enemyX + 1] == 0) {
                return 1; // Move right
            }
        } else {
            // Try left
            if (collisionMap[enemyY * 20 + enemyX - 1] == 0) {
                return 3; // Move left
            }
        }
        
        // If horizontal movement blocked, try vertical
        if (dy > 0) {
            // Try down
            if (collisionMap[(enemyY + 1) * 20 + enemyX] == 0) {
                return 2; // Move down
            }
        } else {
            // Try up
            if (collisionMap[(enemyY - 1) * 20 + enemyX] == 0) {
                return 0; // Move up
            }
        }
    } else {
        // Vertical movement is priority
        if (dy > 0) {
            // Try down
            if (collisionMap[(enemyY + 1) * 20 + enemyX] == 0) {
                return 2; // Move down
            }
        } else {
            // Try up
            if (collisionMap[(enemyY - 1) * 20 + enemyX] == 0) {
                return 0; // Move up
            }
        }
        
        // If vertical movement blocked, try horizontal
        if (dx > 0) {
            // Try right
            if (collisionMap[enemyY * 20 + enemyX + 1] == 0) {
                return 1; // Move right
            }
        } else {
            // Try left
            if (collisionMap[enemyY * 20 + enemyX - 1] == 0) {
                return 3; // Move left
            }
        }
    }
    
    // If we can't move directly toward player, try all directions
    for (int i = 0; i < 4; i++) {
        int newX = enemyX + directions[i][0];
        int newY = enemyY + directions[i][1];
        
        // Check bounds and collision
        if (newX >= 0 && newX < 20 && newY >= 0 && newY < 14 && 
            collisionMap[newY * 20 + newX] == 0) {
            return i;
        }
    }
    
    // No valid move found
    return -1;
}
