
#include "gamestate.h"
#include "engine.h"
#include <time.h>  // For srand

// Global variables for player position (needed for enemy AI)
int playerPosY = 3;
int playerPosX = 3;

// Function prototypes
void displayPlayerStatus(Player *user);
void processEnemyTurns();
void showMainMenu(GameState *state);
void loadLevelFromFile(GameState *state, const char *filename);
void handleInput(GameState *state, Player *user, int *gameRunning);

int main(void) {
    // Initialize random seed
    srand(time(NULL));
    
    // Create game state
    GameState *gameState = create_game_state();
    if (!gameState) {
        printf("Failed to create game state. Exiting.\n");
        return 1;
    }
      // Set up the game world
    int gameRunning = 1;

    Player user;
    user.health = 20;
    user.max_health = 20;
    user.level = 1;
    
    // Initialize inventory
    playerInventory.size = 0;
    playerInventory.contents = NULL;

    // Load Level
    FILE *fptr;
    fptr = fopen("2.lvl", "r");
    if(fptr == NULL) {
      printf("Error! No level file in directory!");   
      exit(1);             
    }
    printf("Loaded Level\n");
    
    // Initialize the game world
    initLevel(fptr);
    
    // Copy current level data to game state
    init_world(gameState, WIDTH, HEIGHT, time(NULL));
    engine_to_world(gameState);
    
    // Set player position
    gameState->player.x = playerPosX;
    gameState->player.y = playerPosY;
    world[playerPosY][playerPosX] = '@';
    
    initColor();

    drawMap(playerPosY, playerPosX);
    displayPlayerStatus(&user);    // Game loop
    while (gameRunning) {
        char ch;

        if (kbhit()) {
            world[playerPosY][playerPosX] = '.'; // restore last cell
            ch = getch();

            int newY = playerPosY;
            int newX = playerPosX;

            // Determine new position based on key
            switch(ch) {
                case 'w': newY--; break;
                case 'a': newX--; break;
                case 's': newY++; break;
                case 'd': newX++; break;
                case 'q': gameRunning = 0; break;  // Quit game
                case 'z': // Save game
                    save_game(gameState, "savegame.sav");
                    break;
                case 'x': // Load game
                    if (load_game(gameState, "savegame.sav")) {
                        world_to_engine(gameState);
                        playerPosX = gameState->player.x;
                        playerPosY = gameState->player.y;
                        user.health = gameState->player.health;
                        user.max_health = gameState->player.max_health;
                        user.level = gameState->player.level;
                        drawMap(playerPosY, playerPosX);
                        displayPlayerStatus(&user);
                    }
                    break;
                default: break;
            }            // Check if new position is valid
            if (newY >= 0 && newY < HEIGHT && newX >= 0 && newX < WIDTH && 
                collisionMap[newY][newX] != 1 && ch != 'q') {
                
                // Check for enemy at new position
                int enemyEncountered = 0;
                for (int i = 0; i < enemyCount; i++) {
                    if (enemyList[i]->y == newY && enemyList[i]->x == newX) {
                        // Combat - reduce enemy health, simplistic for now
                        printf("\nYou attack the %s!\n", enemyList[i]->name);
                        
                        // Update player stats in game state
                        gameState->player.health -= 2;
                        user.health = gameState->player.health;
                        
                        enemyEncountered = 1;
                        
                        // Remove the enemy (for now - could expand to health system)
                        world[enemyList[i]->y][enemyList[i]->x] = '.';
                        
                        // Remove from game state
                        AIEnemy* enemy = get_enemy_at(gameState, newX, newY);
                        if (enemy) {
                            WorldTile* tile = get_tile(gameState, newX, newY);
                            if (tile) tile->entity_id = 0;
                        }
                        
                        // Move enemies to end and decrease count
                        free(enemyList[i]);
                        for (int j = i; j < enemyCount - 1; j++) {
                            enemyList[j] = enemyList[j + 1];
                        }
                        enemyCount--;
                        break;
                    }
                }
                
                if (!enemyEncountered) {
                    playerPosY = newY;
                    playerPosX = newX;
                    
                    // Update game state
                    gameState->player.x = playerPosX;
                    gameState->player.y = playerPosY;
                }                world[playerPosY][playerPosX] = '@';
                drawMap(playerPosY, playerPosX);
                displayPlayerStatus(&user);
                
                // Advance game turn
                turn();
                processEnemyTurns();
                
                // Update game state
                update_game_state(gameState);
                
                // Check for game over after turn
                if(user.health <= 0) {
                    printf("\nYou have died! Game over.\n");
                    gameRunning = 0;
                }
            }
        }        
        
        // Check for end conditions and break out of the game loop if necessary
        if(user.health <= 0) {
            printf("\nYou have died! Game over.\n");
            gameRunning = 0;
        }
    }    // Clean up and exit
    printf("\nThanks for playing!\n");
    destroy_game_state(gameState);
    fclose(fptr);
    return 0;
}

// Display player stats
void displayPlayerStatus(Player *user) {
    printf("\nHealth: %d/%d | Level: %d\n", 
           user->health, user->max_health, user->level);
    printf("Controls: w,a,s,d to move, z to save, x to load, q to quit\n");
}

// Process enemy turns
void processEnemyTurns() {
    // For each enemy, try to move towards the player
    for (int i = 0; i < enemyCount; i++) {
        // Save original position
        int origX = enemyList[i]->x;
        int origY = enemyList[i]->y;
        
        // Simple AI - move randomly (25% chance to move)
        if (rand() % 4 == 0) {
            int direction = rand() % 4; // 0=up, 1=right, 2=down, 3=left
            int newX = origX;
            int newY = origY;
            
            switch(direction) {
                case 0: newY--; break; // Up
                case 1: newX++; break; // Right
                case 2: newY++; break; // Down
                case 3: newX--; break; // Left
            }
            
            // Check if valid move
            if (newY >= 0 && newY < HEIGHT && newX >= 0 && newX < WIDTH && 
                collisionMap[newY][newX] != 1 && 
                !(newY == playerPosY && newX == playerPosX)) { // Don't move onto player
                
                // Update world and enemy position
                world[origY][origX] = '.';
                enemyList[i]->x = newX;
                enemyList[i]->y = newY;
                world[newY][newX] = enemyList[i]->icon;
            }
        }
    }
}

// Show main menu
void showMainMenu(GameState *state) {
    // Set up colors
    initColor();
    
    // Clear screen
    system("cls");
    
    // Display menu
    printf("\033[93m =====================\n");
    printf(" =    ROGUELIKE     =\n");
    printf(" =====================\n\n");
    printf("\033[96m 1. New Game\n");
    printf(" 2. Load Game\n");
    printf(" 3. Credits\n");
    printf(" 4. Quit\n\n");
    printf("\033[97m Enter your choice: ");
    
    // Get user choice
    char choice = getch();
    
    // Process choice
    switch(choice) {
        case '1':
            // Start new game
            break;
        case '2':
            // Load saved game
            if (load_game(state, "savegame.sav")) {
                // Game loaded successfully
            }
            break;
        case '3':
            // Show credits
            system("cls");
            printf("\033[93m =====================\n");
            printf(" =     CREDITS      =\n");
            printf(" =====================\n\n");
            printf("\033[96m Programming: Aidan\n");
            printf(" AI Assistance: GitHub Copilot\n\n");
            printf("\033[97m Press any key to return to menu...");
            getch();
            showMainMenu(state);
            break;
        case '4':
            // Quit game
            exit(0);
            break;
        default:
            showMainMenu(state);
            break;
    }
}

// Load level from file
void loadLevelFromFile(GameState *state, const char *filename) {
    FILE *fptr = fopen(filename, "r");
    if(fptr == NULL) {
        printf("Error! Could not load level file: %s", filename);   
        return;             
    }
    
    // Initialize the game world
    initLevel(fptr);
    
    // Copy data to game state
    engine_to_world(state);
    
    fclose(fptr);
}

// Handle player input
void handleInput(GameState *state, Player *user, int *gameRunning) {
    char ch = getch();
    
    // Store original position for collision checking
    int newY = playerPosY;
    int newX = playerPosX;
    
    // Move current character
    world[playerPosY][playerPosX] = '.';
    
    // Process input
    switch(ch) {
        case 'w': newY--; break;
        case 'a': newX--; break;
        case 's': newY++; break;
        case 'd': newX++; break;
        case 'q': *gameRunning = 0; break;
        case 'z': 
            save_game(state, "savegame.sav");
            break;
        case 'x':
            if (load_game(state, "savegame.sav")) {
                world_to_engine(state);
                playerPosX = state->player.x;
                playerPosY = state->player.y;
                user->health = state->player.health;
                user->max_health = state->player.max_health;
            }
            break;
        case 'i': // Inventory
            // Show inventory screen (would be implemented in a full game)
            break;
        case 'c': // Character sheet
            // Show character screen (would be implemented in a full game)
            break;
        case 'm': // Map
            // Show map screen (would be implemented in a full game)
            break;
        default: break;
    }
    
    // Check if move is valid
    if (newY >= 0 && newY < HEIGHT && newX >= 0 && newX < WIDTH && 
        collisionMap[newY][newX] != 1 && ch != 'q' && ch != 'z' && ch != 'x' && 
        ch != 'i' && ch != 'c' && ch != 'm') {
        
        // Check for enemy at new position
        int enemyEncountered = 0;
        for (int i = 0; i < enemyCount; i++) {
            if (enemyList[i]->y == newY && enemyList[i]->x == newX) {
                // Combat logic would go here
                enemyEncountered = 1;
                break;
            }
        }
        
        if (!enemyEncountered) {
            playerPosY = newY;
            playerPosX = newX;
            
            // Update game state
            state->player.x = playerPosX;
            state->player.y = playerPosY;
        }
    }
    
    // Update world with new player position
    world[playerPosY][playerPosX] = '@';
}