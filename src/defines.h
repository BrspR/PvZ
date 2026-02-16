#ifndef DEFINES_H
#define DEFINES_H

#include "raylib.h"
#include <stdbool.h>

// --- Grid & Screen (1500 x 1000) ---
#define SCREEN_WIDTH 1500
#define SCREEN_HEIGHT 1000

#define GRID_ROWS 5
#define GRID_COLS 10
#define CELL_W 135.0f
#define CELL_H 135.0f
#define GRID_START_X 60.0f
#define GRID_START_Y 160.0f

// --- Gameplay Constants ---
#define START_SUN 200
#define SKY_SUN_INTERVAL 15.0f
#define FLOWER_SUN_INTERVAL 24.0f
#define MINE_ARM_TIME 5.0f
#define CHOMPER_DIGEST_TIME 40.0f

#define FELFEL_ARM_TIME 3.0f // Takes 3 seconds to activate

#define MAX_ZOMBIES 50
#define MAX_PROJECTILES 100
#define MAX_SUNS 50

#define ZOMBIE_BASE_SPEED 0.4f
#define PEA_DAMAGE 20

// --- Level 2 Specifics ---
#define LEVEL2_TOTAL_ZOMBIES 30
#define REWARD_KILL_THRESHOLD 5
#define REWARD_SUN_AMOUNT 50
#define THINKING_ZOMBIE_SPEED_MULT 1.0f

// --- Enums ---
typedef enum GameScreen {
    MENU,

    LEVEL_SELECT,
    LEVEL_1,
    level_2,
    LEVEL_3,
    GAME_OVER,
    VICTORY,
    SHOP_SCREEN
} GameScreen;

typedef enum PlantType {
    PLANT_NONE = 0,
    PLANT_SUNFLOWER = 1,
    PLANT_PEASHOOTER = 2,
    PLANT_MINE = 3,
    PLANT_CHOMPER = 4,
    PLANT_ROSE = 5,
    PLANT_FELFEL = 6
} PlantType;

typedef enum {
    ZOMBIE_NORMAL,
    ZOMBIE_THINKING
} ZombieType;

// --- Structs ---
typedef struct {
    Rectangle rect;
    PlantType plantType;
    float hp;
    float maxHp;
    float timer;
    float lifeTimer;
    float stateTimer;
    bool isArmed;
} Cell;

typedef struct {
    Vector2 position;
    Rectangle hitBox;
    float speed;
    int hp;
    bool active;
    int row;
    bool eating;
    bool isSlowed;
    float freezeTimer;
    int rowSwitchCount;
    ZombieType type;
} Zombie;

typedef struct {
    Vector2 position;
    bool active;
    int row;
    int damage;
    bool isFrozen;
} Projectile;

typedef struct {
    bool active;
    bool triggered;
    int row;
    float x;
} Mower;

typedef struct {
    Vector2 position;
    Vector2 target;
    bool active;
    bool isFalling;
    float spawnTime;
} Sun;

#endif