#include "gamestate.h"
#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>  // For va_list

// Initialization functions

/**
 * Creates a new game state with default values
 */
GameState* create_game_state() {
    GameState* state = (GameState*)malloc(sizeof(GameState));
    if (!state) return NULL;
    
    // Initialize defaults
    memset(state, 0, sizeof(GameState));
    state->player.health = 20;
    state->player.max_health = 20;
    state->player.level = 1;
    state->player.x = 3;
    state->player.y = 3;
    state->real_start_time = time(NULL);
    state->debug_mode = 0;
    state->paused = 0;
    state->is_loaded = 0;
    
    return state;
}

/**
 * Initialize the world with given dimensions and seed
 */
void init_world(GameState* state, int width, int height, int seed) {
    if (!state) return;
    
    // Set world properties
    state->world.seed = seed;
    state->world.chunk_width = width;
    state->world.chunk_height = height;
    state->world.current_chunk_x = 0;
    state->world.current_chunk_y = 0;
    state->world.turn_counter = 0;
    state->world.world_time = time(NULL);
    strcpy(state->world.name, "Default World");
    
    // Create initial chunk
    state->world.chunk_count = 1;
    state->world.chunks = (WorldChunk**)malloc(sizeof(WorldChunk*));
    state->world.chunks[0] = (WorldChunk*)malloc(sizeof(WorldChunk));
    
    WorldChunk* chunk = state->world.chunks[0];
    chunk->x = 0;
    chunk->y = 0;
    chunk->width = width;
    chunk->height = height;
    chunk->active = 1;
    chunk->last_updated = time(NULL);
    
    // Initialize tiles
    chunk->tiles = (WorldTile**)malloc(height * sizeof(WorldTile*));
    for (int y = 0; y < height; y++) {
        chunk->tiles[y] = (WorldTile*)malloc(width * sizeof(WorldTile));
        for (int x = 0; x < width; x++) {
            chunk->tiles[y][x].type = TILE_FLOOR;
            chunk->tiles[y][x].display_char = '.';
            chunk->tiles[y][x].walkable = 1;
            chunk->tiles[y][x].transparent = 1;
            chunk->tiles[y][x].entity_id = 0;
            chunk->tiles[y][x].item_id = 0;
        }
    }
}

/**
 * Load a specific chunk from disk or generate it
 */
void load_chunk(GameState* state, int chunk_x, int chunk_y) {
    if (!state) return;
    
    // Check if chunk already exists
    for (int i = 0; i < state->world.chunk_count; i++) {
        if (state->world.chunks[i]->x == chunk_x && state->world.chunks[i]->y == chunk_y) {
            state->world.chunks[i]->active = 1;
            state->world.current_chunk_x = chunk_x;
            state->world.current_chunk_y = chunk_y;
            return;
        }
    }
    
    // Create new chunk
    state->world.chunk_count++;
    state->world.chunks = (WorldChunk**)realloc(state->world.chunks, 
                                               state->world.chunk_count * sizeof(WorldChunk*));
    
    int index = state->world.chunk_count - 1;
    state->world.chunks[index] = (WorldChunk*)malloc(sizeof(WorldChunk));
    
    WorldChunk* chunk = state->world.chunks[index];
    chunk->x = chunk_x;
    chunk->y = chunk_y;
    chunk->width = state->world.chunk_width;
    chunk->height = state->world.chunk_height;
    chunk->active = 1;
    chunk->last_updated = time(NULL);
    
    // Initialize tiles with procedural generation
    chunk->tiles = (WorldTile**)malloc(chunk->height * sizeof(WorldTile*));
    for (int y = 0; y < chunk->height; y++) {
        chunk->tiles[y] = (WorldTile*)malloc(chunk->width * sizeof(WorldTile));
        for (int x = 0; x < chunk->width; x++) {
            // Basic procedural generation using seed
            int value = (x * 7 + y * 13 + state->world.seed + chunk_x * 31 + chunk_y * 47) % 100;
            
            if (value < 70) {
                chunk->tiles[y][x].type = TILE_FLOOR;
                chunk->tiles[y][x].display_char = '.';
                chunk->tiles[y][x].walkable = 1;
                chunk->tiles[y][x].transparent = 1;
            } else {
                chunk->tiles[y][x].type = TILE_WALL;
                chunk->tiles[y][x].display_char = 'w';
                chunk->tiles[y][x].walkable = 0;
                chunk->tiles[y][x].transparent = 0;
            }
            
            chunk->tiles[y][x].entity_id = 0;
            chunk->tiles[y][x].item_id = 0;
        }
    }
    
    // Set as current chunk
    state->world.current_chunk_x = chunk_x;
    state->world.current_chunk_y = chunk_y;
}

/**
 * Unload a chunk to save memory (doesn't delete it)
 */
void unload_chunk(GameState* state, int chunk_x, int chunk_y) {
    if (!state) return;
    
    for (int i = 0; i < state->world.chunk_count; i++) {
        if (state->world.chunks[i]->x == chunk_x && state->world.chunks[i]->y == chunk_y) {
            state->world.chunks[i]->active = 0;
            state->world.chunks[i]->last_updated = time(NULL);
            return;
        }
    }
}

// Game state management

/**
 * Save the game to a file
 */
int save_game(GameState* state, const char* filename) {
    if (!state || !filename) return 0;
    
    FILE* file = fopen(filename, "wb");
    if (!file) return 0;
    
    // Write header
    fprintf(file, "ROGUELIKE_SAVE_v1\n");
    
    // Write player data
    fprintf(file, "PLAYER\n");
    fprintf(file, "%d %d %d %d %d %d %s\n", 
            state->player.x, state->player.y,
            state->player.health, state->player.max_health,
            state->player.strength, state->player.level,
            state->player.name);
    
    // Write world data
    fprintf(file, "WORLD\n");    fprintf(file, "%s %d %d %d %d %lld\n",
            state->world.name, state->world.chunk_count,
            state->world.chunk_width, state->world.chunk_height,
            state->world.seed, state->world.world_time);
    
    // Write chunk data
    fprintf(file, "CHUNKS\n");
    for (int i = 0; i < state->world.chunk_count; i++) {
        WorldChunk* chunk = state->world.chunks[i];        fprintf(file, "CHUNK %d %d %d %d %d %lld\n",
                chunk->x, chunk->y, chunk->width, chunk->height,
                chunk->active, chunk->last_updated);
        
        // Write all tiles in chunk
        for (int y = 0; y < chunk->height; y++) {
            for (int x = 0; x < chunk->width; x++) {
                WorldTile* tile = &chunk->tiles[y][x];
                fprintf(file, "%d %c %d %d %d %d\n",
                        tile->type, tile->display_char,
                        tile->walkable, tile->transparent,
                        tile->entity_id, tile->item_id);
            }
        }
    }
    
    // Write enemy data
    fprintf(file, "ENEMIES %d\n", state->enemy_count);
    for (int i = 0; i < state->enemy_count; i++) {
        AIEnemy* enemy = &state->enemies[i];
        fprintf(file, "%d %d %d %d %d %d %d %d\n",
                enemy->id, enemy->base.x, enemy->base.y,
                enemy->base.health, enemy->faction_id,
                enemy->ai_state, enemy->detection_radius,
                enemy->behavior_flags);
    }
    
    // Write item data
    fprintf(file, "ITEMS %d\n", state->item_count);
    for (int i = 0; i < state->item_count; i++) {
        GameItem* item = &state->items[i];
        fprintf(file, "%s %c %d %f %d\n",
                item->name, item->icon,
                item->value, item->weight, item->type);
    }
    
    // Write end marker
    fprintf(file, "END\n");
    
    fclose(file);
    printf("Game saved to %s\n", filename);
    return 1;
}

/**
 * Load the game from a file
 */
int load_game(GameState* state, const char* filename) {
    if (!state || !filename) return 0;
    
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Could not open save file: %s\n", filename);
        return 0;
    }
    
    char buffer[256];
    
    // Read and check header
    if (!fgets(buffer, sizeof(buffer), file) || 
        strncmp(buffer, "ROGUELIKE_SAVE", 14) != 0) {
        printf("Invalid save file format\n");
        fclose(file);
        return 0;
    }
    
    // Clean up existing state
    destroy_game_state(state);
    state = create_game_state();
    
    // Read sections
    while (fgets(buffer, sizeof(buffer), file)) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline
        
        if (strcmp(buffer, "PLAYER") == 0) {
            // Read player data
            if (fscanf(file, "%d %d %d %d %d %d %s\n",
                      &state->player.x, &state->player.y,
                      &state->player.health, &state->player.max_health,
                      &state->player.strength, &state->player.level,
                      state->player.name) != 7) {
                printf("Error reading player data\n");
                fclose(file);
                return 0;
            }
        }        else if (strcmp(buffer, "WORLD") == 0) {
            // Read world data
            if (fscanf(file, "%s %d %d %d %d %lld\n",
                      state->world.name, &state->world.chunk_count,
                      &state->world.chunk_width, &state->world.chunk_height,
                      &state->world.seed, &state->world.world_time) != 6) {
                printf("Error reading world data\n");
                fclose(file);
                return 0;
            }
            
            // Allocate chunk array
            state->world.chunks = (WorldChunk**)malloc(state->world.chunk_count * sizeof(WorldChunk*));
        }
        else if (strcmp(buffer, "CHUNKS") == 0) {
            // Read chunk data
            for (int i = 0; i < state->world.chunk_count; i++) {
                // Read chunk header
                int chunk_x, chunk_y, width, height, active;
                time_t last_updated;
                  if (fscanf(file, "CHUNK %d %d %d %d %d %lld\n",
                          &chunk_x, &chunk_y, &width, &height,
                          &active, &last_updated) != 6) {
                    printf("Error reading chunk header\n");
                    fclose(file);
                    return 0;
                }
                
                // Create chunk
                state->world.chunks[i] = (WorldChunk*)malloc(sizeof(WorldChunk));
                WorldChunk* chunk = state->world.chunks[i];
                chunk->x = chunk_x;
                chunk->y = chunk_y;
                chunk->width = width;
                chunk->height = height;
                chunk->active = active;
                chunk->last_updated = last_updated;
                
                // Allocate tiles
                chunk->tiles = (WorldTile**)malloc(height * sizeof(WorldTile*));
                for (int y = 0; y < height; y++) {
                    chunk->tiles[y] = (WorldTile*)malloc(width * sizeof(WorldTile));
                    
                    // Read tile data for this row
                    for (int x = 0; x < width; x++) {
                        WorldTile* tile = &chunk->tiles[y][x];
                        int type, walkable, transparent, entity_id, item_id;
                        char display_char;
                        
                        if (fscanf(file, "%d %c %d %d %d %d\n",
                                  &type, &display_char,
                                  &walkable, &transparent,
                                  &entity_id, &item_id) != 6) {
                            printf("Error reading tile data\n");
                            fclose(file);
                            return 0;
                        }
                        
                        tile->type = (TileType)type;
                        tile->display_char = display_char;
                        tile->walkable = walkable;
                        tile->transparent = transparent;
                        tile->entity_id = entity_id;
                        tile->item_id = item_id;
                    }
                }
            }
        }
        else if (strncmp(buffer, "ENEMIES", 7) == 0) {
            // Read enemy count
            if (sscanf(buffer, "ENEMIES %d", &state->enemy_count) != 1) {
                printf("Error reading enemy count\n");
                fclose(file);
                return 0;
            }
            
            // Allocate enemies array
            state->enemies = (AIEnemy*)malloc(state->enemy_count * sizeof(AIEnemy));
            
            // Read each enemy
            for (int i = 0; i < state->enemy_count; i++) {
                AIEnemy* enemy = &state->enemies[i];
                
                if (fscanf(file, "%d %d %d %d %d %d %d %d\n",
                          &enemy->id, &enemy->base.x, &enemy->base.y,
                          &enemy->base.health, &enemy->faction_id,
                          &enemy->ai_state, &enemy->detection_radius,
                          &enemy->behavior_flags) != 8) {
                    printf("Error reading enemy data\n");
                    fclose(file);
                    return 0;
                }
                
                // Set icon and name based on faction/type
                enemy->base.icon = 'G'; // Default goblin for now
                enemy->base.name = "Goblin"; // Default
            }
        }
        else if (strncmp(buffer, "ITEMS", 5) == 0) {
            // Read item count
            if (sscanf(buffer, "ITEMS %d", &state->item_count) != 1) {
                printf("Error reading item count\n");
                fclose(file);
                return 0;
            }
            
            // Allocate items array
            state->items = (GameItem*)malloc(state->item_count * sizeof(GameItem));
            
            // Read each item
            for (int i = 0; i < state->item_count; i++) {
                GameItem* item = &state->items[i];
                  if (fscanf(file, "%s %c %d %lf %d\n",
                          item->name, &item->icon,
                          &item->value, &item->weight, &item->type) != 5) {
                    printf("Error reading item data\n");
                    fclose(file);
                    return 0;
                }
            }
        }
        else if (strcmp(buffer, "END") == 0) {
            break;
        }
    }
    
    fclose(file);
    state->is_loaded = 1;
    strcpy(state->save_file, filename);
    printf("Game loaded from %s\n", filename);
    return 1;
}

/**
 * Update game state (called once per turn)
 */
void update_game_state(GameState* state) {
    if (!state) return;
    
    // Update world time and turn counter
    state->world.world_time++;
    state->world.turn_counter++;
    
    // Update active chunks
    for (int i = 0; i < state->world.chunk_count; i++) {
        if (state->world.chunks[i]->active) {
            simulate_world_chunk(state, state->world.chunks[i]);
        }
    }
    
    // Process AI for all enemies
    for (int i = 0; i < state->enemy_count; i++) {
        process_enemy_ai(state, &state->enemies[i]);
    }
    
    // Update faction relations periodically
    if (state->world.turn_counter % 10 == 0) {
        update_faction_relations(state);
    }
}

/**
 * Free all memory used by game state
 */
void destroy_game_state(GameState* state) {
    if (!state) return;
    
    // Free chunks and tiles
    for (int i = 0; i < state->world.chunk_count; i++) {
        WorldChunk* chunk = state->world.chunks[i];
        
        for (int y = 0; y < chunk->height; y++) {
            free(chunk->tiles[y]);
        }
        
        free(chunk->tiles);
        free(chunk);
    }
    
    free(state->world.chunks);
    
    // Free enemies
    if (state->enemies) {
        free(state->enemies);
    }
    
    // Free items
    if (state->items) {
        free(state->items);
    }
    
    // Reset state to default values
    memset(state, 0, sizeof(GameState));
}

/**
 * Convert the world representation to the engine representation
 */
void world_to_engine(GameState* state) {
    if (!state) return;
    
    // Get active chunk
    WorldChunk* chunk = get_chunk_at(state, 
                                     state->world.current_chunk_x, 
                                     state->world.current_chunk_y);
    if (!chunk) return;
    
    // Copy tile data to engine world representation
    for (int y = 0; y < chunk->height && y < HEIGHT; y++) {
        for (int x = 0; x < chunk->width && x < WIDTH; x++) {
            WorldTile* tile = &chunk->tiles[y][x];
            
            // Set display character based on tile type
            world[y][x] = tile->display_char;
            
            // Set collision map
            collisionMap[y][x] = tile->walkable ? 0 : 1;
            
            // Add entities if present
            if (tile->entity_id > 0) {
                AIEnemy* enemy = get_enemy(state, tile->entity_id);
                if (enemy) {
                    world[y][x] = enemy->base.icon;
                }
            }
            
            // Player position is special (handled separately)
            if (state->player.x == x && state->player.y == y) {
                world[y][x] = '@';
            }
        }
    }
    
    // Update enemy list for engine
    enemyCount = 0;
    for (int i = 0; i < state->enemy_count; i++) {
        AIEnemy* ai_enemy = &state->enemies[i];
        
        // Only add enemies in current chunk
        if (ai_enemy->base.x >= 0 && ai_enemy->base.x < WIDTH &&
            ai_enemy->base.y >= 0 && ai_enemy->base.y < HEIGHT) {
            
            // Create engine enemy
            enemy* new_enemy = (enemy*)malloc(sizeof(enemy));
            new_enemy->x = ai_enemy->base.x;
            new_enemy->y = ai_enemy->base.y;
            new_enemy->icon = ai_enemy->base.icon;
            new_enemy->health = ai_enemy->base.health;
            new_enemy->name = ai_enemy->base.name;
            
            // Add to engine list
            enemyList[enemyCount++] = new_enemy;
        }
    }
}

/**
 * Convert the engine representation to the world representation
 */
void engine_to_world(GameState* state) {
    if (!state) return;
    
    // Get or create active chunk
    WorldChunk* chunk = get_chunk_at(state, 
                                     state->world.current_chunk_x, 
                                     state->world.current_chunk_y);
    if (!chunk) {
        load_chunk(state, state->world.current_chunk_x, state->world.current_chunk_y);
        chunk = get_chunk_at(state, state->world.current_chunk_x, state->world.current_chunk_y);
    }
    
    // Copy engine world to game state
    for (int y = 0; y < HEIGHT && y < chunk->height; y++) {
        for (int x = 0; x < WIDTH && x < chunk->width; x++) {
            WorldTile* tile = &chunk->tiles[y][x];
            
            // Set tile type based on character
            switch (world[y][x]) {
                case 'w':
                    tile->type = TILE_WALL;
                    tile->display_char = 'w';
                    tile->walkable = 0;
                    tile->transparent = 0;
                    break;
                    
                case '.':
                    tile->type = TILE_FLOOR;
                    tile->display_char = '.';
                    tile->walkable = 1;
                    tile->transparent = 1;
                    break;
                    
                case '@':
                    // Player position - floor tile underneath
                    tile->type = TILE_FLOOR;
                    tile->display_char = '.';
                    tile->walkable = 1;
                    tile->transparent = 1;
                    
                    // Update player position
                    state->player.x = x;
                    state->player.y = y;
                    break;
                    
                default:
                    // Check if it's an enemy
                    for (int i = 0; i < enemyCount; i++) {
                        if (enemyList[i]->x == x && enemyList[i]->y == y) {
                            // It's an enemy - save entity ID
                            tile->type = TILE_FLOOR; // Floor under enemy
                            tile->display_char = '.';
                            tile->walkable = 1;
                            tile->transparent = 1;
                            
                            // Create AI enemy if needed
                            if (state->enemy_count == 0) {
                                state->enemies = (AIEnemy*)malloc(sizeof(AIEnemy));
                            } else {
                                state->enemies = (AIEnemy*)realloc(state->enemies, 
                                                         (state->enemy_count + 1) * sizeof(AIEnemy));
                            }
                            
                            // Initialize enemy
                            AIEnemy* ai_enemy = &state->enemies[state->enemy_count];
                            ai_enemy->id = state->enemy_count + 1; // 1-based IDs
                            ai_enemy->base.x = x;
                            ai_enemy->base.y = y;
                            ai_enemy->base.health = 10;
                            ai_enemy->base.icon = enemyList[i]->icon;
                            ai_enemy->base.name = enemyList[i]->name;
                            ai_enemy->faction_id = 1; // Default faction
                            ai_enemy->ai_state = 0; // Idle
                            ai_enemy->detection_radius = 5;
                            ai_enemy->memory_count = 0;
                            ai_enemy->behavior_flags = 0;
                            
                            // Link enemy to tile
                            tile->entity_id = ai_enemy->id;
                            
                            state->enemy_count++;
                            break;
                        }
                    }
                    break;
            }
        }
    }
}

// World interaction

/**
 * Set a tile type at a specific position
 */
void set_tile(GameState* state, int x, int y, TileType type) {
    if (!state) return;
    
    WorldTile* tile = get_tile(state, x, y);
    if (!tile) return;
    
    tile->type = type;
    
    // Update display character and properties based on type
    switch (type) {
        case TILE_EMPTY:
            tile->display_char = ' ';
            tile->walkable = 0;
            tile->transparent = 1;
            break;
            
        case TILE_FLOOR:
            tile->display_char = '.';
            tile->walkable = 1;
            tile->transparent = 1;
            break;
            
        case TILE_WALL:
            tile->display_char = 'w';
            tile->walkable = 0;
            tile->transparent = 0;
            break;
            
        case TILE_DOOR:
            tile->display_char = '+';
            tile->walkable = 1;
            tile->transparent = 0;
            break;
            
        case TILE_WATER:
            tile->display_char = '~';
            tile->walkable = 0;
            tile->transparent = 1;
            break;
            
        case TILE_LAVA:
            tile->display_char = '^';
            tile->walkable = 0;
            tile->transparent = 1;
            break;
    }
}

/**
 * Get a tile at a specific position
 */
WorldTile* get_tile(GameState* state, int x, int y) {
    if (!state) return NULL;
    
    // Find chunk containing this position
    int chunk_x = state->world.current_chunk_x;
    int chunk_y = state->world.current_chunk_y;
    
    WorldChunk* chunk = get_chunk_at(state, chunk_x, chunk_y);
    if (!chunk) return NULL;
    
    // Check bounds
    if (x < 0 || y < 0 || x >= chunk->width || y >= chunk->height)
        return NULL;
    
    return &chunk->tiles[y][x];
}

/**
 * Check if a position is walkable
 */
int is_walkable(GameState* state, int x, int y) {
    WorldTile* tile = get_tile(state, x, y);
    if (!tile) return 0;
    
    return tile->walkable && tile->entity_id == 0;
}

/**
 * Get a chunk at specific coordinates
 */
WorldChunk* get_chunk_at(GameState* state, int chunk_x, int chunk_y) {
    if (!state) return NULL;
    
    for (int i = 0; i < state->world.chunk_count; i++) {
        if (state->world.chunks[i]->x == chunk_x && 
            state->world.chunks[i]->y == chunk_y) {
            return state->world.chunks[i];
        }
    }
    
    return NULL;
}

/**
 * Get index of a chunk
 */
int get_chunk_index(GameState* state, int chunk_x, int chunk_y) {
    if (!state) return -1;
    
    for (int i = 0; i < state->world.chunk_count; i++) {
        if (state->world.chunks[i]->x == chunk_x && 
            state->world.chunks[i]->y == chunk_y) {
            return i;
        }
    }
    
    return -1;
}

// Entity management

/**
 * Move an entity to a new position
 */
void move_entity(GameState* state, int entity_id, int new_x, int new_y) {
    if (!state || entity_id <= 0) return;
    
    // Handle player
    if (entity_id == 0) {
        // Clear old tile
        WorldTile* old_tile = get_tile(state, state->player.x, state->player.y);
        if (old_tile) old_tile->entity_id = 0;
        
        // Update position
        state->player.x = new_x;
        state->player.y = new_y;
        
        // Update new tile
        WorldTile* new_tile = get_tile(state, new_x, new_y);
        if (new_tile) new_tile->entity_id = 0; // Player is special
        
        return;
    }
    
    // Find entity
    AIEnemy* enemy = NULL;
    for (int i = 0; i < state->enemy_count; i++) {
        if (state->enemies[i].id == entity_id) {
            enemy = &state->enemies[i];
            break;
        }
    }
    
    if (!enemy) return;
    
    // Clear old position
    WorldTile* old_tile = get_tile(state, enemy->base.x, enemy->base.y);
    if (old_tile) old_tile->entity_id = 0;
    
    // Update position
    enemy->base.x = new_x;
    enemy->base.y = new_y;
    
    // Update new tile
    WorldTile* new_tile = get_tile(state, new_x, new_y);
    if (new_tile) new_tile->entity_id = entity_id;
}

/**
 * Add a new enemy to the game
 */
int add_enemy(GameState* state, AIEnemy enemy) {
    if (!state) return 0;
    
    // Resize enemy array
    if (state->enemy_count == 0) {
        state->enemies = (AIEnemy*)malloc(sizeof(AIEnemy));
    } else {
        state->enemies = (AIEnemy*)realloc(state->enemies, 
                                  (state->enemy_count + 1) * sizeof(AIEnemy));
    }
    
    // Set ID
    enemy.id = state->enemy_count + 1;
    
    // Add enemy
    state->enemies[state->enemy_count] = enemy;
    state->enemy_count++;
    
    // Update tile
    WorldTile* tile = get_tile(state, enemy.base.x, enemy.base.y);
    if (tile) tile->entity_id = enemy.id;
    
    return enemy.id;
}

/**
 * Remove an enemy from the game
 */
void remove_enemy(GameState* state, int enemy_id) {
    if (!state || enemy_id <= 0) return;
    
    int index = -1;
    for (int i = 0; i < state->enemy_count; i++) {
        if (state->enemies[i].id == enemy_id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) return;
    
    // Clear tile
    WorldTile* tile = get_tile(state, state->enemies[index].base.x, 
                               state->enemies[index].base.y);
    if (tile) tile->entity_id = 0;
    
    // Remove enemy by shifting array
    for (int i = index; i < state->enemy_count - 1; i++) {
        state->enemies[i] = state->enemies[i + 1];
    }
    
    state->enemy_count--;
    
    // Resize array if needed
    if (state->enemy_count > 0) {
        state->enemies = (AIEnemy*)realloc(state->enemies, 
                                  state->enemy_count * sizeof(AIEnemy));
    } else {
        free(state->enemies);
        state->enemies = NULL;
    }
}

/**
 * Get an enemy by ID
 */
AIEnemy* get_enemy(GameState* state, int enemy_id) {
    if (!state || enemy_id <= 0) return NULL;
    
    for (int i = 0; i < state->enemy_count; i++) {
        if (state->enemies[i].id == enemy_id) {
            return &state->enemies[i];
        }
    }
    
    return NULL;
}

/**
 * Get an enemy at specific coordinates
 */
AIEnemy* get_enemy_at(GameState* state, int x, int y) {
    if (!state) return NULL;
    
    WorldTile* tile = get_tile(state, x, y);
    if (!tile || tile->entity_id <= 0) return NULL;
    
    return get_enemy(state, tile->entity_id);
}

// Item management

/**
 * Add a new item to the game
 */
int add_item(GameState* state, GameItem new_item, int x, int y) {
    if (!state) return 0;
    
    // Resize item array
    if (state->item_count == 0) {
        state->items = (GameItem*)malloc(sizeof(GameItem));
    } else {
        state->items = (GameItem*)realloc(state->items, 
                               (state->item_count + 1) * sizeof(GameItem));
    }
    
    // Add item
    state->items[state->item_count] = new_item;
    int item_id = state->item_count + 1;
    state->item_count++;
    
    // Update tile if position is valid
    if (x >= 0 && y >= 0) {
        WorldTile* tile = get_tile(state, x, y);
        if (tile) tile->item_id = item_id;
    }
    
    return item_id;
}

/**
 * Get an item by ID
 */
GameItem* get_item(GameState* state, int item_id) {
    if (!state || item_id <= 0 || item_id > state->item_count) return NULL;
    
    return &state->items[item_id - 1];
}

/**
 * Remove an item from the game
 */
void remove_item(GameState* state, int item_id) {
    if (!state || item_id <= 0 || item_id > state->item_count) return;
    
    // Clear all tile references to this item
    for (int i = 0; i < state->world.chunk_count; i++) {
        WorldChunk* chunk = state->world.chunks[i];
        
        for (int y = 0; y < chunk->height; y++) {
            for (int x = 0; x < chunk->width; x++) {
                if (chunk->tiles[y][x].item_id == item_id) {
                    chunk->tiles[y][x].item_id = 0;
                }
            }
        }
    }
    
    // Remove item by shifting array
    for (int i = item_id - 1; i < state->item_count - 1; i++) {
        state->items[i] = state->items[i + 1];
    }
    
    state->item_count--;
    
    // Update references to other items (decrease ID by 1 for items after the removed one)
    for (int i = 0; i < state->world.chunk_count; i++) {
        WorldChunk* chunk = state->world.chunks[i];
        
        for (int y = 0; y < chunk->height; y++) {
            for (int x = 0; x < chunk->width; x++) {
                if (chunk->tiles[y][x].item_id > item_id) {
                    chunk->tiles[y][x].item_id--;
                }
            }
        }
    }
    
    // Resize array if needed
    if (state->item_count > 0) {
        state->items = (GameItem*)realloc(state->items, 
                               state->item_count * sizeof(GameItem));
    } else {
        free(state->items);
        state->items = NULL;
    }
}

// AI and simulation

/**
 * Process AI for a specific enemy
 */
void process_enemy_ai(GameState* state, AIEnemy* enemy) {
    if (!state || !enemy) return;
    
    // Check if player is visible
    int can_see_player = can_detect_player(state, enemy);
    
    // Update AI state based on what enemy knows
    switch (enemy->ai_state) {
        case 0: // Idle
            if (can_see_player) {
                // Player spotted! Change to chase state
                enemy->ai_state = 2;
                enemy->ai_target_id = 0; // Player ID
                update_enemy_memory(state, enemy, 0, state->player.x, state->player.y);
            } else if (rand() % 4 == 0) {
                // Random chance to start patrolling
                enemy->ai_state = 1;
            }
            break;
            
        case 1: // Patrol
            if (can_see_player) {
                // Player spotted! Change to chase state
                enemy->ai_state = 2;
                enemy->ai_target_id = 0;
                update_enemy_memory(state, enemy, 0, state->player.x, state->player.y);
            } else {
                // Move randomly
                int dirs[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}}; // up, right, down, left
                int dir = rand() % 4;
                int new_x = enemy->base.x + dirs[dir][0];
                int new_y = enemy->base.y + dirs[dir][1];
                
                if (is_walkable(state, new_x, new_y)) {
                    move_entity(state, enemy->id, new_x, new_y);
                }
            }
            break;
            
        case 2: // Chase player
            if (can_see_player) {
                // Update memory of player position
                update_enemy_memory(state, enemy, 0, state->player.x, state->player.y);
                
                // Calculate path to player
                calculate_path(state, enemy, state->player.x, state->player.y);
            }
            
            // Move along path if we have one
            if (enemy->path_length > 0 && enemy->path_index < enemy->path_length) {
                int next_x = enemy->path[enemy->path_index][0];
                int next_y = enemy->path[enemy->path_index][1];
                
                if (is_walkable(state, next_x, next_y)) {
                    move_entity(state, enemy->id, next_x, next_y);
                }
                
                enemy->path_index++;
            } else {
                // No path or reached end of path
                // If we can't see player, go to last known position
                if (!can_see_player && enemy->memory_count > 0) {
                    int newest_memory = 0;
                    time_t newest_time = 0;
                    
                    // Find newest memory of player
                    for (int i = 0; i < enemy->memory_count; i++) {
                        if (enemy->memories[i].entity_id == 0 && 
                            enemy->memories[i].time_seen > newest_time) {
                            newest_memory = i;
                            newest_time = enemy->memories[i].time_seen;
                        }
                    }
                    
                    // Go to last known position
                    calculate_path(state, enemy, 
                                 enemy->memories[newest_memory].x,
                                 enemy->memories[newest_memory].y);
                } else if (!can_see_player) {
                    // Lost track of player, go back to idle
                    enemy->ai_state = 0;
                }
            }
            break;
    }
}

/**
 * Update faction relations
 */
void update_faction_relations(GameState* state) {
    // Placeholder for now - would implement faction dynamics here
}

/**
 * Check if an enemy can detect the player
 */
int can_detect_player(GameState* state, AIEnemy* enemy) {
    if (!state || !enemy) return 0;
    
    // Calculate distance to player
    int dx = state->player.x - enemy->base.x;
    int dy = state->player.y - enemy->base.y;
    int distance = (int)sqrt(dx*dx + dy*dy);
    
    // Check if within detection radius
    if (distance > enemy->detection_radius) return 0;
    
    // Check line of sight
    return get_line_of_sight(state, enemy->base.x, enemy->base.y, 
                           state->player.x, state->player.y);
}

/**
 * Update enemy's memory of an entity
 */
void update_enemy_memory(GameState* state, AIEnemy* enemy, int entity_id, int x, int y) {
    if (!state || !enemy) return;
    
    // Look for existing memory of this entity
    for (int i = 0; i < enemy->memory_count; i++) {
        if (enemy->memories[i].entity_id == entity_id) {
            // Update existing memory
            enemy->memories[i].x = x;
            enemy->memories[i].y = y;
            enemy->memories[i].time_seen = state->world.world_time;
            return;
        }
    }
    
    // Add new memory if we have space
    if (enemy->memory_count < 10) {
        enemy->memories[enemy->memory_count].entity_id = entity_id;
        enemy->memories[enemy->memory_count].x = x;
        enemy->memories[enemy->memory_count].y = y;
        enemy->memories[enemy->memory_count].time_seen = state->world.world_time;
        enemy->memory_count++;
    } else {
        // Replace oldest memory
        int oldest = 0;
        time_t oldest_time = state->world.world_time;
        
        for (int i = 0; i < enemy->memory_count; i++) {
            if (enemy->memories[i].time_seen < oldest_time) {
                oldest = i;
                oldest_time = enemy->memories[i].time_seen;
            }
        }
        
        enemy->memories[oldest].entity_id = entity_id;
        enemy->memories[oldest].x = x;
        enemy->memories[oldest].y = y;
        enemy->memories[oldest].time_seen = state->world.world_time;
    }
}

/**
 * Calculate a path for an enemy to a target
 */
void calculate_path(GameState* state, AIEnemy* enemy, int target_x, int target_y) {
    if (!state || !enemy) return;
    
    // Very simple direct path for now - this would be replaced with proper A* pathfinding
    enemy->path_length = 0;
    enemy->path_index = 0;
    
    // Add current position as first point
    enemy->path[enemy->path_length][0] = enemy->base.x;
    enemy->path[enemy->path_length][1] = enemy->base.y;
    enemy->path_length++;
    
    // Add target as final point
    enemy->path[enemy->path_length][0] = target_x;
    enemy->path[enemy->path_length][1] = target_y;
    enemy->path_length++;
}

/**
 * Simulate world changes in a chunk
 */
void simulate_world_chunk(GameState* state, WorldChunk* chunk) {
    if (!state || !chunk) return;
    
    // Update chunk's last_updated time
    chunk->last_updated = time(NULL);
    
    // Process events based on world state, e.g.,
    // - Growth of plants
    // - Water flow
    // - Temperature changes
    // - Weather effects
    // - Trap activations
    // - Light sources
}

// Utility functions

/**
 * Log a game event
 */
void log_game_event(GameState* state, const char* format, ...) {
    // Placeholder for logging system
    if (state->debug_mode) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf("\n");
    }
}

/**
 * Calculate distance between two points
 */
int get_distance(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    return (int)sqrt(dx*dx + dy*dy);
}

/**
 * Check if there's a line of sight between two points
 */
int get_line_of_sight(GameState* state, int x1, int y1, int x2, int y2) {
    if (!state) return 0;
    
    // Using Bresenham's line algorithm to check for obstacles
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    
    while (x1 != x2 || y1 != y2) {
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
        
        // Skip the endpoints
        if ((x1 == x2 && y1 == y2) || (x1 == x2 && y1 == y2))
            continue;
        
        // Check if this tile blocks line of sight
        WorldTile* tile = get_tile(state, x1, y1);
        if (tile && !tile->transparent)
            return 0;
    }
    
    return 1;
}

/**
 * Roll dice with a specific number of dice and sides
 */
int roll_dice(int num_dice, int num_sides) {
    int result = 0;
    for (int i = 0; i < num_dice; i++) {
        result += (rand() % num_sides) + 1;
    }
    return result;
}

/**
 * Check if an event with a specific probability happens
 */
int chance(float probability) {
    return (rand() / (float)RAND_MAX) < probability;
}