#ifndef LEVEL3_H
#define LEVEL3_H

#include "defines.h"
#include "braindeadzombie.h"
#include "levels.h"

// --- Level 3 Globals ---
int l3_normalZombiesSpawned = 0;
int l3_thinkingZombiesSpawned = 0;
int l3_killedCounter = 0;
int l3_rewardCounter = 0;
float l3_waveTimer = 0.0f;

void InitLevel3() {
    sunCount = 1400; // Start high
    l3_normalZombiesSpawned = 0;
    l3_thinkingZombiesSpawned = 0;
    l3_killedCounter = 0;
    l3_rewardCounter = 0;
    l3_waveTimer = -3.0f;
    currentPeaFireRate = 1.8f;

    // Reset Grid
    for (int r=0; r<GRID_ROWS; r++) {
        for (int c=0; c<GRID_COLS; c++) {
            grid[r][c].rect = (Rectangle){GRID_START_X + c * CELL_W, GRID_START_Y + r * CELL_H, CELL_W, CELL_H};
            grid[r][c].plantType = PLANT_NONE;
            grid[r][c].hp = 0;
            grid[r][c].timer = 0;
            grid[r][c].lifeTimer = 0;
            grid[r][c].isArmed = false;
        }
    }

    for (int i = 0; i < GRID_ROWS; i++) {
        mowers[i].active = true;
        mowers[i].triggered = false;
        mowers[i].x = GRID_START_X + 10;
        mowers[i].row = i;
    }

    for(int i=0; i<MAX_ZOMBIES; i++) zombies[i].active = false;
    for(int i=0; i<MAX_PROJECTILES; i++) projectiles[i].active = false;
    for(int i=0; i<MAX_SUNS; i++) suns[i].active = false;
}

void SpawnL3NormalZombie() {
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
        l3_normalZombiesSpawned++;
    }
}

void SpawnL3ThinkingZombie() {
    int bestRow = PickBestStartRow();

    int id = -1;
    for(int i=0; i<MAX_ZOMBIES; i++) if(!zombies[i].active) { id=i; break; }

    if (id != -1) {
        zombies[id].active = true;
        zombies[id].type = ZOMBIE_THINKING;
        zombies[id].row = bestRow;

        float spawnX = GRID_START_X + (GRID_COLS * CELL_W);
        zombies[id].position = (Vector2){spawnX, grid[bestRow][0].rect.y + 10};
        zombies[id].speed = ZOMBIE_BASE_SPEED;
        zombies[id].hp = 150;
        zombies[id].hitBox = (Rectangle){zombies[id].position.x + 20, zombies[id].position.y, 60, 90};
        zombies[id].eating = false;
        zombies[id].freezeTimer = 0;
        zombies[id].rowSwitchCount = 0;

        l3_thinkingZombiesSpawned++;
    }
}

void UpdateLevel3Logic(GameScreen *currentScreen, float dt) {
    UpdateAllRowStats(grid, mowers);

    // --- DIFFICULTY RULE: Sun < 700 ---
    float dynamicSpeedMult = 1.0f;

    if (sunCount < 700) {
        // DOUBLE EVERYTHING (Hard Mode)
        currentZombieSpawnRate = 4.0f;  // Base 8.0 / 2
        dynamicSpeedMult = 2.0f;        // 2x Speed
    } else {
        // NORMAL
        currentZombieSpawnRate = 7.0f;
        dynamicSpeedMult = 1.5f;
    }

    l3_waveTimer += dt;
    if (l3_waveTimer > currentZombieSpawnRate) { // Dynamic Rate
        l3_waveTimer = 0;
        if (l3_normalZombiesSpawned < 15) SpawnL3NormalZombie();
        if (l3_thinkingZombiesSpawned < 15) SpawnL3ThinkingZombie();
    }

    if (l3_normalZombiesSpawned >= 15 && l3_thinkingZombiesSpawned >= 15) {
         bool allDead = true;
         for(int i=0; i<MAX_ZOMBIES; i++) if(zombies[i].active) allDead = false;
         if(allDead) *currentScreen = VICTORY;
    }

    float rowSpeedMultipliers[GRID_ROWS];
    bool rowHasRose[GRID_ROWS];
    for(int i=0; i<GRID_ROWS; i++) { rowSpeedMultipliers[i] = 1.0f; rowHasRose[i] = false; }

    for(int r=0; r<GRID_ROWS; r++) {
        for(int c=0; c<GRID_COLS; c++) {
            if(grid[r][c].plantType == PLANT_ROSE) rowHasRose[r] = true;
            if(grid[r][c].plantType == PLANT_CHOMPER && grid[r][c].lifeTimer < 20.0f) rowSpeedMultipliers[r] = 0.333f;
        }
    }

    // Plants Logic
    for (int r=0; r<GRID_ROWS; r++) {
        bool zInRow = false;
        for(int z=0; z<MAX_ZOMBIES; z++) if(zombies[z].active && zombies[z].row == r && zombies[z].position.x > 0) zInRow = true;

        for (int c=0; c<GRID_COLS; c++) {
            if (grid[r][c].plantType == PLANT_NONE) continue;
            grid[r][c].lifeTimer += dt;

            // Rose Healing
            if (rowHasRose[r] && grid[r][c].hp < grid[r][c].maxHp) {
                grid[r][c].hp += (grid[r][c].maxHp * 0.10f * dt);
                if (grid[r][c].hp > grid[r][c].maxHp) grid[r][c].hp = grid[r][c].maxHp;
            }

            if (grid[r][c].plantType == PLANT_PEASHOOTER) {
                grid[r][c].timer += dt;
                float fireDelay = currentPeaFireRate;
                if (rowHasRose[r]) fireDelay /= 1.2f;

                if (zInRow && grid[r][c].timer >= fireDelay) {
                    grid[r][c].timer = 0;
                    for(int p=0; p<MAX_PROJECTILES; p++) {
                        if(!projectiles[p].active) {
                            projectiles[p].active = true; projectiles[p].row = r;
                            projectiles[p].position = (Vector2){grid[r][c].rect.x+90, grid[r][c].rect.y+35};
                            projectiles[p].damage = PEA_DAMAGE; break;
                        }
                    }
                }
            }
            else if (grid[r][c].plantType == PLANT_MINE) {
                if (grid[r][c].lifeTimer >= MINE_ARM_TIME) grid[r][c].isArmed = true;
                if (grid[r][c].isArmed) {
                    Rectangle triggerRect = grid[r][c].rect;
                    triggerRect.width += 50;
                    for (int z=0; z<MAX_ZOMBIES; z++) {
                        if (zombies[z].active && zombies[z].row == r && CheckCollisionRecs(triggerRect, zombies[z].hitBox)) {
                            zombies[z].hp = -200;
                            grid[r][c].plantType = PLANT_NONE;
                            break;
                        }
                    }
                }
            }
            else if (grid[r][c].plantType == PLANT_FELFEL) {
                if (grid[r][c].lifeTimer >= FELFEL_ARM_TIME) { // 3 Seconds
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
            else if (grid[r][c].plantType == PLANT_ROSE) {
                if (grid[r][c].lifeTimer >= 10.0f) grid[r][c].plantType = PLANT_NONE;
            }
            else if (grid[r][c].plantType == PLANT_CHOMPER) {
                if (grid[r][c].lifeTimer >= 20.0f) grid[r][c].plantType = PLANT_NONE;
            }
        }
    }

    // Projectiles & Rewards
    for (int i=0; i<MAX_PROJECTILES; i++) {
        if (!projectiles[i].active) continue;
        projectiles[i].position.x += 300 * dt;
        if (projectiles[i].position.x > SCREEN_WIDTH) projectiles[i].active = false;

        for(int z=0; z<MAX_ZOMBIES; z++) {
            if (zombies[z].active && zombies[z].row == projectiles[i].row && CheckCollisionCircleRec(projectiles[i].position, 10, zombies[z].hitBox)) {
                projectiles[i].active = false;
                zombies[z].hp -= projectiles[i].damage;
                if (zombies[z].hp <= 0 && zombies[z].hp > -100) {
                    zombies[z].active = false;

                    // REWARD LOGIC: 200 Sun / 5 Thinking Kills
                    if (zombies[z].type == ZOMBIE_THINKING) {
                        l3_rewardCounter++;
                        if (l3_rewardCounter >= 5) {
                            sunCount += 200;
                            l3_rewardCounter = 0;
                        }
                    }
                }
                break;
            }
        }
    }

    // Zombies Logic
    for (int i=0; i<MAX_ZOMBIES; i++) {
        if (!zombies[i].active) continue;

        if (zombies[i].hp <= -200) { zombies[i].active = false; continue; }
        if (zombies[i].hp <= 0) { zombies[i].active = false; continue; }

        float currentSpeed = zombies[i].speed * rowSpeedMultipliers[zombies[i].row] * dynamicSpeedMult;

        // Thinking Logic
        if (zombies[i].type == ZOMBIE_THINKING) {
            zombies[i].freezeTimer += dt;
            if (zombies[i].freezeTimer > 1.0f && zombies[i].rowSwitchCount < 2) {
                zombies[i].freezeTimer = 0;
                int bestRow = PickBestStartRow();
                if (zombies[i].row != bestRow) {
                    zombies[i].row = bestRow;
                    zombies[i].rowSwitchCount++;
                }
            }
            float targetY = grid[zombies[i].row][0].rect.y + 10;
            if (fabs(zombies[i].position.y - targetY) > 2.0f) {
                if (zombies[i].position.y < targetY) zombies[i].position.y += 100 * dt;
                else zombies[i].position.y -= 100 * dt;
            }
        }

        if (!zombies[i].eating) {
            zombies[i].position.x -= currentSpeed * 60.0f * dt;
        }
        zombies[i].hitBox.x = zombies[i].position.x + 20;
        zombies[i].hitBox.y = zombies[i].position.y;

        zombies[i].eating = false;
        int col = (int)((zombies[i].hitBox.x + 30 - GRID_START_X)/CELL_W);
        if (col >= 0 && col < GRID_COLS && grid[zombies[i].row][col].plantType != PLANT_NONE) {
            if (grid[zombies[i].row][col].plantType == PLANT_MINE && grid[zombies[i].row][col].isArmed) {
                zombies[i].eating = false;
            } else {
                zombies[i].eating = true;
                grid[zombies[i].row][col].hp -= 20.0f * dt;
                if (grid[zombies[i].row][col].hp <= 0) grid[zombies[i].row][col].plantType = PLANT_NONE;
            }
        }

        if (mowers[zombies[i].row].active && zombies[i].position.x < mowers[zombies[i].row].x + 40) {
            mowers[zombies[i].row].triggered = true;
        }
        if (zombies[i].position.x < -50) *currentScreen = GAME_OVER;
    }

    // Mowers Logic
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