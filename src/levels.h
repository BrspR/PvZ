#ifndef LEVELS_H
#define LEVELS_H

#include "defines.h"

// Global Variables Reference
extern Cell grid[GRID_ROWS][GRID_COLS];
extern Zombie zombies[MAX_ZOMBIES];
extern Projectile projectiles[MAX_PROJECTILES];
extern Mower mowers[GRID_ROWS];
extern Sun suns[MAX_SUNS];
extern int sunCount;

// Difficulty Vars
float currentSkyInterval = 6.0f;
float currentFlowerInterval = 7.0f;
float currentZombieSpawnRate = 12.0f;
float currentPeaFireRate = 1.5f;

extern float zombieSpawnTimer;
extern float sunSpawnTimer;

// Helper: Spawn Sun from Sunflower
void SpawnSunAt(float x, float y) {
    for(int i=0; i<MAX_SUNS; i++) {
        if(!suns[i].active) {
            suns[i].active = true;
            suns[i].position = (Vector2){x, y};
            suns[i].target = (Vector2){x, y};
            suns[i].isFalling = false;
            suns[i].spawnTime = GetTime();
            break;
        }
    }
}

// Helper: Spawn Sun from Sky
void SpawnSkySun() {
    for(int i=0; i<MAX_SUNS; i++) {
        if(!suns[i].active) {
            suns[i].active = true;
            // Random X position above screen
            suns[i].position = (Vector2){(float)GetRandomValue(GRID_START_X, GRID_START_X + GRID_COLS*CELL_W), -50};
            suns[i].target = (Vector2){suns[i].position.x, (float)GetRandomValue(200, 600)};
            suns[i].isFalling = true;
            suns[i].spawnTime = GetTime();
            break;
        }
    }
}

// Initialize Level 1
void InitLevel() {
    sunCount = 200;
    zombieSpawnTimer = -2.0f; // Delay first zombie
    sunSpawnTimer = 0.0f;

    // L1 Settings
    currentSkyInterval = 6.0f;
    currentFlowerInterval = 7.0f;
    currentZombieSpawnRate = 6.0f;
    currentPeaFireRate = 1.5f;

    // Reset Grid
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            grid[r][c].rect = (Rectangle){GRID_START_X + c * CELL_W, GRID_START_Y + r * CELL_H, CELL_W, CELL_H};
            grid[r][c].plantType = PLANT_NONE;
            grid[r][c].hp = 0;
            grid[r][c].timer = 0;
            grid[r][c].lifeTimer = 0;
            grid[r][c].isArmed = false;
        }
    }

    // Reset Mowers
    for (int i = 0; i < GRID_ROWS; i++) {
        mowers[i].active = true;
        mowers[i].triggered = false;
        mowers[i].x = GRID_START_X + 10;
        mowers[i].row = i;
    }

    // Clear Arrays
    for(int i=0; i<MAX_ZOMBIES; i++) zombies[i].active = false;
    for(int i=0; i<MAX_PROJECTILES; i++) projectiles[i].active = false;
    for(int i=0; i<MAX_SUNS; i++) suns[i].active = false;
}

// MAIN UPDATE LOOP (Logic for Level 1)
void UpdateGameLogic(GameScreen *currentScreen) {
    float dt = GetFrameTime();

    // 1. Sun Spawning
    sunSpawnTimer += dt;
    if (sunSpawnTimer >= currentSkyInterval) {
        sunSpawnTimer = 0;
        SpawnSkySun();
    }

    // Update active suns
    for(int i=0; i<MAX_SUNS; i++) {
        if(suns[i].active) {
            if(suns[i].isFalling && suns[i].position.y < suns[i].target.y) suns[i].position.y += 100 * dt;
            if(GetTime() - suns[i].spawnTime > 15.0) suns[i].active = false;
        }
    }

    // 2. Zombie Spawning (Normal Zombies Only)
    zombieSpawnTimer += dt;
    if (zombieSpawnTimer > currentZombieSpawnRate) {
        zombieSpawnTimer = 0;
        int id = -1;
        for(int i=0; i<MAX_ZOMBIES; i++) if(!zombies[i].active) { id=i; break; }

        if (id != -1) {
            zombies[id].active = true;
            zombies[id].type = ZOMBIE_NORMAL;
            zombies[id].row = GetRandomValue(0, GRID_ROWS-1);
            zombies[id].position = (Vector2){SCREEN_WIDTH + 20, grid[zombies[id].row][0].rect.y + 10};
            zombies[id].speed = ZOMBIE_BASE_SPEED;
            zombies[id].hp = 100;
            zombies[id].hitBox = (Rectangle){zombies[id].position.x + 20, zombies[id].position.y, 60, 90};
            zombies[id].eating = false;
        }
    }

    // 3. Row Buffs (Check for Rose/Chomper)
    float rowSpeedMultipliers[GRID_ROWS];
    bool rowHasRose[GRID_ROWS];

    for(int i=0; i<GRID_ROWS; i++) {
        rowSpeedMultipliers[i] = 1.0f;
        rowHasRose[i] = false;
    }

    for(int r=0; r<GRID_ROWS; r++) {
        for(int c=0; c<GRID_COLS; c++) {
            if(grid[r][c].plantType == PLANT_ROSE) rowHasRose[r] = true;
            if(grid[r][c].plantType == PLANT_CHOMPER && grid[r][c].lifeTimer < 20.0f) {
                rowSpeedMultipliers[r] = 0.333f; // Slow down row if Chomper digesting
            }
        }
    }

    // 4. Plant Logic
    for (int r = 0; r < GRID_ROWS; r++) {
        bool zInRow = false;
        for(int z=0; z<MAX_ZOMBIES; z++)
            if(zombies[z].active && zombies[z].row == r && zombies[z].position.x > 0) zInRow = true;

        for (int c = 0; c < GRID_COLS; c++) {
            if (grid[r][c].plantType == PLANT_NONE) continue;

            grid[r][c].lifeTimer += dt;

            // ROSE HEALING: Heals plants in the same row
            if (rowHasRose[r] && grid[r][c].hp < grid[r][c].maxHp) {
                grid[r][c].hp += (grid[r][c].maxHp * 0.10f * dt);
                if (grid[r][c].hp > grid[r][c].maxHp) grid[r][c].hp = grid[r][c].maxHp;
            }

            if (grid[r][c].plantType == PLANT_SUNFLOWER) {
                grid[r][c].timer += dt;
                if (grid[r][c].timer >= currentFlowerInterval) {
                    grid[r][c].timer = 0;
                    SpawnSunAt(grid[r][c].rect.x, grid[r][c].rect.y);
                }
            }
            else if (grid[r][c].plantType == PLANT_PEASHOOTER) {
                grid[r][c].timer += dt;

                // ROSE BUFF: Fire 1.2x faster
                float fireDelay = currentPeaFireRate;
                if (rowHasRose[r]) fireDelay /= 1.2f;

                if (zInRow && grid[r][c].timer >= fireDelay) {
                    grid[r][c].timer = 0;
                    for(int p=0; p<MAX_PROJECTILES; p++) {
                        if(!projectiles[p].active) {
                            projectiles[p].active = true;
                            projectiles[p].row = r;
                            // Shoot from exact end of barrel
                            projectiles[p].position = (Vector2){grid[r][c].rect.x + 90, grid[r][c].rect.y + 35};
                            projectiles[p].damage = PEA_DAMAGE;
                            break;
                        }
                    }
                }
            }
            // --- MINE LOGIC ---
            else if (grid[r][c].plantType == PLANT_MINE) {
                if (grid[r][c].lifeTimer >= MINE_ARM_TIME) grid[r][c].isArmed = true;

                if (grid[r][c].isArmed) {
                    // Trigger Zone: 50px in front of mine
                    Rectangle triggerRect = grid[r][c].rect;
                    triggerRect.width += 50;

                    for (int z=0; z<MAX_ZOMBIES; z++) {
                        if (zombies[z].active && zombies[z].row == r &&
                            CheckCollisionRecs(triggerRect, zombies[z].hitBox)) {

                            zombies[z].hp = -200; // INSTAKILL SIGNAL
                            grid[r][c].plantType = PLANT_NONE; // Mine disappears
                            break;
                        }
                    }
                }
            }
            else if (grid[r][c].plantType == PLANT_FELFEL) {
                if (grid[r][c].lifeTimer >= FELFEL_ARM_TIME +1.2f) { // 3 Seconds
                    // KILL ALL ZOMBIES IN ROW
                    for(int z=0; z<MAX_ZOMBIES; z++) {
                        if(zombies[z].active && zombies[z].row == r) {
                            zombies[z].hp = -200; // Instakill
                        }
                    }
                    // Remove Plant
                    grid[r][c].plantType = PLANT_NONE;
                }
            }
            // Rose Dies after 10s
            else if (grid[r][c].plantType == PLANT_ROSE) {
                if (grid[r][c].lifeTimer >= 10.0f) grid[r][c].plantType = PLANT_NONE;
            }
            else if (grid[r][c].plantType == PLANT_CHOMPER) {
                if (grid[r][c].lifeTimer >= 20.0f) grid[r][c].plantType = PLANT_NONE;
            }
        }
    }

    // 5. Projectiles
    for (int i=0; i<MAX_PROJECTILES; i++) {
        if (!projectiles[i].active) continue;
        projectiles[i].position.x += 300 * dt;
        if (projectiles[i].position.x > SCREEN_WIDTH) projectiles[i].active = false;

        for(int z=0; z<MAX_ZOMBIES; z++) {
            if (zombies[z].active && zombies[z].row == projectiles[i].row &&
                CheckCollisionCircleRec(projectiles[i].position, 10, zombies[z].hitBox)) {

                projectiles[i].active = false;
                zombies[z].hp -= projectiles[i].damage;
                if (zombies[z].hp <= 0 && zombies[z].hp > -100) zombies[z].active = false;
                break;
            }
        }
    }

    // 6. Zombie Logic
    for (int i=0; i<MAX_ZOMBIES; i++) {
        if (!zombies[i].active) continue;

        // Check for Mine Kill (-200 HP)
        if (zombies[i].hp <= -200) {
            zombies[i].active = false;
            continue;
        }
        if (zombies[i].hp <= 0) {
            zombies[i].active = false;
            continue;
        }

        float currentSpeed = zombies[i].speed * rowSpeedMultipliers[zombies[i].row] *3.0f;

        if (!zombies[i].eating) zombies[i].position.x -= currentSpeed * 60.0f * dt;
        zombies[i].hitBox.x = zombies[i].position.x + 20;

        zombies[i].eating = false;
        int col = (int)((zombies[i].hitBox.x + 30 - GRID_START_X)/CELL_W);

        // Eating Logic
        if (col >= 0 && col < GRID_COLS && grid[zombies[i].row][col].plantType != PLANT_NONE) {
            // FIX: If Mine is armed, DON'T eat it (Wait to trigger explosion)
            if (grid[zombies[i].row][col].plantType == PLANT_MINE && grid[zombies[i].row][col].isArmed) {
                zombies[i].eating = false;
            }
            else {
                zombies[i].eating = true;
                grid[zombies[i].row][col].hp -= 20.0f * dt; // Eat plant
                if (grid[zombies[i].row][col].hp <= 0) grid[zombies[i].row][col].plantType = PLANT_NONE;
            }
        }

        if (mowers[zombies[i].row].active && zombies[i].position.x < mowers[zombies[i].row].x + 40) {
            mowers[zombies[i].row].triggered = true;
        }

        if (zombies[i].position.x < -50) *currentScreen = GAME_OVER;
    }

    // 7. Mowers Logic
    for (int i=0; i<GRID_ROWS; i++) {
        if (mowers[i].triggered) {
            mowers[i].x += 400 * dt;
            for(int z=0; z<MAX_ZOMBIES; z++) {
                if(zombies[z].active && zombies[z].row == i &&
                   zombies[z].position.x < mowers[i].x + 50 && zombies[z].position.x > mowers[i].x - 50) {
                    zombies[z].active = false;
                }
            }
            if (mowers[i].x > SCREEN_WIDTH) mowers[i].active = false;
        }
    }
}

#endif