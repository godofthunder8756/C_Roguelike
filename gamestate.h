#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Include enemy.h first to avoid redefinition issues
#include "enemy.h"

// Forward declarations
struct WorldTile;
struct WorldChunk;
struct World;
struct GameState;

// Define item type here to avoid circular dependencies
typedef struct GameItem {
    char name[32];      // Item name
    char icon;          // Display character
    int value;          // Value (for selling/buying)
    double weight;      // Weight
    int type;           // Type of item
    int properties[10]; // Various properties based on type
} GameItem;

// Tile types
typedef enum {
    TILE_EMPTY = 0,
    TILE_FLOOR = 1,
    TILE_WALL = 2,
    TILE_DOOR = 3,
    TILE_WATER = 4,
    TILE_LAVA = 5
} TileType;

// Represents a single tile in the world
typedef struct WorldTile {
    TileType type;          // Type of tile
    char display_char;      // Character to display
    int walkable;           // Whether the tile can be walked on
    int transparent;        // Whether the tile allows light to pass through
    int entity_id;          // ID of entity on this tile (0 = no entity)
    int item_id;            // ID of item on this tile (0 = no item)
} WorldTile;

// A chunk of the world (for larger worlds)
typedef struct WorldChunk {
    int x, y;               // Chunk coordinates
    WorldTile** tiles;      // 2D array of tiles
    int width, height;      // Dimensions of this chunk
    int active;             // Whether this chunk is currently active
    time_t last_updated;    // When this chunk was last updated
} WorldChunk;

// Represents a complete world
typedef struct World {
    char name[64];          // World name
    WorldChunk** chunks;    // Array of chunks
    int chunk_count;        // Number of chunks
    int chunk_width;        // Width of a chunk
    int chunk_height;       // Height of a chunk
    int current_chunk_x;    // Current active chunk x
    int current_chunk_y;    // Current active chunk y
    int seed;               // World seed for procedural generation
    time_t world_time;      // In-game time
    int turn_counter;       // Number of turns passed
} World;

// Extended player structure with more RPG attributes
typedef struct Player {
    int id;                 // Unique ID
    char name[32];          // Player name
    int x, y;               // Position
    int chunk_x, chunk_y;   // Chunk position
    int health;             // Current health
    int max_health;         // Maximum health
    int strength;           // Affects damage
    int dexterity;          // Affects hit chance and dodge
    int intelligence;       // Affects magic and interactions
    int level;              // Current level
    int xp;                 // Experience points
    int inventory_id;       // ID of player's inventory
    int equipment_slots[10];// Equipped items (IDs)
    int faction_relations[10]; // Relations with different factions
    int status_effects[10]; // Active status effects
} Player;

// Extended enemy with AI attributes
typedef struct AIEnemy {
    enemy base;             // Base enemy structure
    int id;                 // Unique ID
    int faction_id;         // Which faction this enemy belongs to
    int ai_state;           // Current AI state (patrolling, hunting, fleeing, etc.)
    int ai_target_id;       // ID of current target
    int detection_radius;   // How far this enemy can see
    int memory_count;       // Number of things this enemy remembers
    struct {
        int entity_id;      // What was seen
        int x, y;           // Where it was seen
        time_t time_seen;   // When it was seen
    } memories[10];         // Memory of things the enemy has seen
    int path[64][2];        // Current path being followed
    int path_length;        // Length of current path
    int path_index;         // Current position in path
    int behavior_flags;     // Behavior flags (aggressive, timid, etc.)
    int last_action_time;   // When the enemy last took an action
} AIEnemy;

// Game state structure that holds everything
typedef struct GameState {    Player player;          // The player
    World world;            // The world
    int enemy_count;        // Number of enemies
    AIEnemy* enemies;       // Dynamic array of enemies
    int item_count;         // Number of items in the world
    GameItem* items;        // Dynamic array of items
    int active_effects;     // Global effects currently active
    char save_file[256];    // Path to save file
    int is_loaded;          // Whether game state is loaded
    time_t real_start_time; // When the game was started
    int paused;             // Whether the game is paused
    int debug_mode;         // Whether debug mode is enabled
    // Additional fields can be added for future expansion
} GameState;

// Function prototypes

// Initialization functions
GameState* create_game_state();
void init_world(GameState* state, int width, int height, int seed);
void load_chunk(GameState* state, int chunk_x, int chunk_y);
void unload_chunk(GameState* state, int chunk_x, int chunk_y);

// Game state management
int save_game(GameState* state, const char* filename);
int load_game(GameState* state, const char* filename);
void update_game_state(GameState* state);
void destroy_game_state(GameState* state);
void world_to_engine(GameState* state);
void engine_to_world(GameState* state);

// World interaction
void set_tile(GameState* state, int x, int y, TileType type);
WorldTile* get_tile(GameState* state, int x, int y);
int is_walkable(GameState* state, int x, int y);
WorldChunk* get_chunk_at(GameState* state, int chunk_x, int chunk_y);
int get_chunk_index(GameState* state, int chunk_x, int chunk_y);

// Entity management
void move_entity(GameState* state, int entity_id, int new_x, int new_y);
int add_enemy(GameState* state, AIEnemy enemy);
void remove_enemy(GameState* state, int enemy_id);
AIEnemy* get_enemy(GameState* state, int enemy_id);
AIEnemy* get_enemy_at(GameState* state, int x, int y);

// Item management
int add_item(GameState* state, GameItem new_item, int x, int y);
GameItem* get_item(GameState* state, int item_id);
void remove_item(GameState* state, int item_id);

// AI and simulation
void process_enemy_ai(GameState* state, AIEnemy* enemy);
void update_faction_relations(GameState* state);
int can_detect_player(GameState* state, AIEnemy* enemy);
void update_enemy_memory(GameState* state, AIEnemy* enemy, int entity_id, int x, int y);
void calculate_path(GameState* state, AIEnemy* enemy, int target_x, int target_y);
void simulate_world_chunk(GameState* state, WorldChunk* chunk);

// Utility functions
void log_game_event(GameState* state, const char* format, ...);
int get_distance(int x1, int y1, int x2, int y2);
int get_line_of_sight(GameState* state, int x1, int y1, int x2, int y2);
int roll_dice(int num_dice, int num_sides);
int chance(float probability);

#endif /* GAMESTATE_H */
