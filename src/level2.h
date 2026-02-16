#ifndef LEVEL2_H
#define LEVEL2_H

// These are your "Toolboxes". They contain variables and helper functions we need.
#include "defines.h"           // Contains grid size, zombie types, and struct definitions.
#include "braindeadzombie.h"   // Contains the "Math Brain" for the smart zombies.
#include "levels.h"            // Contains shared variables like 'grid' and 'zombies'.

// --- GLOBAL VARIABLES FOR LEVEL 2 ---
// These keep track of the game state specifically for this level.

// How many smart zombies have we spawned so far? (Max is 30)
int thinkingZombiesSpawnedCount = 0;

// How many have we killed? (Used for winning the level)
int thinkingZombiesKilledCounter = 0;

// A timer to count seconds until the next zombie spawns.
float l2_spawnTimer = 0.0f;

// How much health the next zombie will have (changes with difficulty).
float currentZombieHP = 120.0f;

// -------------------------------------------------------------------------
// FUNCTION: InitLevel2
// Purpose: Resets the entire game board to be ready for Level 2.
// It cleans up old zombies, resets plants, and sets the starting money.
// -------------------------------------------------------------------------
void InitLevel2() {
    // 1. Set Starting Money (Sun)
    sunCount = 200;

    // 2. Reset counters to 0
    thinkingZombiesSpawnedCount = 0;
    thinkingZombiesKilledCounter = 0;
    l2_spawnTimer = 0.0f;
    currentZombieHP = 120.0f;

    // 3. Set Base Difficulty Variables (Normal Mode)
    currentSkyInterval = 11.0f;     // Sun falls every 11 seconds
    currentFlowerInterval = 11.0f;  // Flowers produce sun every 11 seconds
    currentZombieSpawnRate = 8.0f;  // Zombies spawn every 8 seconds
    currentPeaFireRate = 1.3f;      // Peashooters shoot every 1.3 seconds

    // 4. Clean the Grid (Remove all old plants from Level 1)
    for (int r = 0; r < GRID_ROWS; r++) {     // Loop through rows (0 to 4)
        for (int c = 0; c < GRID_COLS; c++) { // Loop through columns (0 to 9)
            // Calculate where this square is on the screen (Pixels)
            grid[r][c].rect = (Rectangle){
                GRID_START_X + c * CELL_W,  // X Position
                GRID_START_Y + r * CELL_H,  // Y Position
                CELL_W, CELL_H              // Width & Height
            };

            // Empty the square
            grid[r][c].plantType = PLANT_NONE;
            grid[r][c].hp = 0;
            grid[r][c].timer = 0;
            grid[r][c].lifeTimer = 0;
            grid[r][c].isArmed = false;
        }
    }

    // 5. DISABLE MOWERS (Level 2 Rule: "No Gas")
    for(int i = 0; i < GRID_ROWS; i++) {
        mowers[i].active = false; // Turn them off
        mowers[i].x = -500;       // Move them way off screen just in case
    }

    // 6. Clear all entities (Delete old zombies and bullets)
    for(int i=0; i<MAX_ZOMBIES; i++) zombies[i].active = false;
    for(int i=0; i<MAX_PROJECTILES; i++) projectiles[i].active = false;
    for(int i=0; i<MAX_SUNS; i++) suns[i].active = false;
}

// -------------------------------------------------------------------------
// FUNCTION: SpawnThinkingZombie
// Purpose: Finds a dead zombie slot, brings it to life, and gives it a brain.
// -------------------------------------------------------------------------
void SpawnThinkingZombie() {
    // 1. Update the "Math Brain" with the latest plant positions.
    UpdateAllRowStats(grid, mowers);

    // 2. Ask the "Math Brain" which row is the safest/easiest.
    int bestRow = PickBestStartRow();

    // 3. Stop if we have already spawned enough zombies for this level.
    if (thinkingZombiesSpawnedCount >= LEVEL2_TOTAL_ZOMBIES) return;

    // 4. Find an empty slot in our zombie array.
    int id = -1;
    for(int i = 0; i < MAX_ZOMBIES; i++) {
        if(!zombies[i].active) { // Found a dead zombie slot!
            id = i;
            break;
        }
    }

    // 5. If we found a slot, Initialize the zombie!
    if (id != -1) {
        zombies[id].active = true;         // It's alive!
        zombies[id].type = ZOMBIE_THINKING;// Set type to Smart Zombie bssssssssss
        zombies[id].row = bestRow;         // Put it in the smartest row

        // Calculate spawn position (Just off the right side of the grid)
        float spawnX = GRID_START_X + (GRID_COLS * CELL_W);
        zombies[id].position = (Vector2){spawnX, grid[bestRow][0].rect.y + 10};

        // Set stats
        zombies[id].speed = ZOMBIE_BASE_SPEED;
        zombies[id].hp = currentZombieHP; // Uses dynamic HP (Harder if you have lots of sun)

        // Set collision box
        zombies[id].hitBox = (Rectangle){
            zombies[id].position.x + 20,
            zombies[id].position.y,
            60, 90
        };

        zombies[id].eating = false;
        zombies[id].freezeTimer = 0; // Timer used for thinking
        zombies[id].rowSwitchCount = 0; // Starts with 0 switches

        thinkingZombiesSpawnedCount++; // Count this spawn
    }
}

// -------------------------------------------------------------------------
// FUNCTION: UpdateLevel2Logic
// Purpose: The MAIN LOOP for Level 2. This runs 60 times every second.
// It handles difficulty, spawning, movement, eating, and collisions.
// -------------------------------------------------------------------------
void UpdateLevel2Logic(GameScreen *currentScreen, float dt) {
    // 1. Update the "Math Brain" cache (Optimized: once per frame)
    UpdateAllRowStats(grid, mowers);

    // --- DYNAMIC DIFFICULTY SYSTEM ---
    // If you have too much money, the game gets harder!

    float dynamicSpeedMult = 1.0f;          // Multiplier for zombie speed
    int currentReward = REWARD_SUN_AMOUNT;  // How much sun you get for kills

    if (sunCount > 1000) {
        // EXTREME MODE: You are rich, so we punish you.
        currentReward = 0;              // No reward
        currentZombieSpawnRate = 2.7f;  // Spawn super fast (every 2.7s)
        dynamicSpeedMult = 3.0f;        // Zombies run 3x faster
        currentZombieHP = 220.0f;       // Zombies are tanks
    }
    else if (sunCount > 400 ) {
        // HARD MODE: Getting tougher.
        currentReward = 0;
        currentZombieSpawnRate = 4.0f;
        dynamicSpeedMult = 1.5f;
        currentZombieHP = 150.0f;
    }
    else {
        // NORMAL MODE: Standard settings.
        currentReward = REWARD_SUN_AMOUNT;
        currentZombieSpawnRate = 8.0f;
        dynamicSpeedMult = 1.0f;
        currentZombieHP = 120.0f;
    }

    // --- SUN SPAWNING LOGIC ---
    sunSpawnTimer += dt; // Add time passed (dt) to timer
    if (sunSpawnTimer >= currentSkyInterval) {
        sunSpawnTimer = 0;
        SpawnSkySun(); // Drop a sun from the sky
    }

    // Move existing suns down (Gravity)
    for(int i=0; i<MAX_SUNS; i++) {
        if(suns[i].active) {
            // If it hasn't reached the ground yet, move Y down
            if (suns[i].isFalling && suns[i].position.y < suns[i].target.y) {
                suns[i].position.y += 100 * dt;
            }
            // Despawn sun if it sits there for too long (20 seconds)
            if (GetTime() - suns[i].spawnTime > 20.0) {
                suns[i].active = false;
            }
        }
    }

    // --- ZOMBIE SPAWN LOGIC ---
    l2_spawnTimer += dt;
    // If timer passed the rate AND we haven't spawned all zombies yet...
    if (l2_spawnTimer > currentZombieSpawnRate && thinkingZombiesSpawnedCount < LEVEL2_TOTAL_ZOMBIES) {
        l2_spawnTimer = 0;
        SpawnThinkingZombie();
    }

    // --- VICTORY CHECK ---
    // If we spawned everyone...
    if (thinkingZombiesSpawnedCount >= LEVEL2_TOTAL_ZOMBIES) {
         bool allDead = true;
         // Check if ANY zombie is still alive
         for(int i=0; i<MAX_ZOMBIES; i++) if(zombies[i].active) allDead = false;

         // If none are alive, YOU WIN!
         if(allDead) *currentScreen = VICTORY;
    }

    // --- PLANT LOGIC LOOP ---
    // We loop through every square on the grid to update plants.

    // First: Check if a Rose or Chomper exists to apply buffs/debuffs
    float rowSpeedMultipliers[GRID_ROWS]; // Stores slow-down factor for each row
    bool rowHasRose[GRID_ROWS];           // Stores if a row has a Rose

    // Initialize defaults (Normal speed, No rose)
    for(int i=0; i<GRID_ROWS; i++) {
        rowSpeedMultipliers[i] = 1.0f;
        rowHasRose[i] = false;
    }

    // Scan Grid
    for(int r=0; r<GRID_ROWS; r++) {
        for(int c=0; c<GRID_COLS; c++) {
            if(grid[r][c].plantType == PLANT_ROSE) rowHasRose[r] = true;

            // If Chomper is ALIVE (timer < 20s), it slows the row
            if(grid[r][c].plantType == PLANT_CHOMPER && grid[r][c].lifeTimer < 20.0f) {
                rowSpeedMultipliers[r] = 0.333f; // Set speed to 33%
            }
        }
    }

    // Main Plant Update Loop
    for (int r=0; r<GRID_ROWS; r++) {
        // Optimization: Only check collisions if a zombie is actually in this row
        bool zInRow = false;
        for(int z=0; z<MAX_ZOMBIES; z++) if(zombies[z].active && zombies[z].row == r && zombies[z].position.x > 0) zInRow = true;

        for (int c=0; c<GRID_COLS; c++) {
            if (grid[r][c].plantType == PLANT_NONE) continue; // Skip empty cells

            grid[r][c].lifeTimer += dt; // Age the plant

            // ROSE: Heals plants
            if (rowHasRose[r] && grid[r][c].hp < grid[r][c].maxHp) {
                grid[r][c].hp += (grid[r][c].maxHp * 0.10f * dt); // Heal 10% per sec
                if (grid[r][c].hp > grid[r][c].maxHp) grid[r][c].hp = grid[r][c].maxHp;
            }

            // SUNFLOWER: Makes money
            if (grid[r][c].plantType == PLANT_SUNFLOWER) {
                grid[r][c].timer += dt;
                if (grid[r][c].timer >= currentFlowerInterval) {
                    grid[r][c].timer = 0;
                    SpawnSunAt(grid[r][c].rect.x, grid[r][c].rect.y);
                }
            }
            // PEASHOOTER: Shoots peas
            else if (grid[r][c].plantType == PLANT_PEASHOOTER) {
                grid[r][c].timer += dt;

                // ROSE BUFF: Fire faster if rose is present
                float fireDelay = currentPeaFireRate;
                if (rowHasRose[r]) fireDelay /= 1.2f;

                // Fire ONLY if ready AND zombie is in row
                if (zInRow && grid[r][c].timer >= fireDelay) {
                    grid[r][c].timer = 0;
                    // Find inactive projectile to use
                    for(int p=0; p<MAX_PROJECTILES; p++) {
                        if(!projectiles[p].active) {
                            projectiles[p].active = true;
                            projectiles[p].row = r;
                            // Shoot from tip of the plant
                            projectiles[p].position = (Vector2){grid[r][c].rect.x+90, grid[r][c].rect.y+35};
                            projectiles[p].damage = PEA_DAMAGE;
                            break;
                        }
                    }
                }
            }
            else if (grid[r][c].plantType == PLANT_FELFEL) {
                if (grid[r][c].lifeTimer >= FELFEL_ARM_TIME + 1.2f)  { // 3 Seconds
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
            // POTATO MINE: Explodes
            else if (grid[r][c].plantType == PLANT_MINE) {
                // Arm itself after 5 seconds
                if (grid[r][c].lifeTimer >= MINE_ARM_TIME) grid[r][c].isArmed = true;

                if (grid[r][c].isArmed) {
                    // Create a "Trigger Zone" 50px in front
                    Rectangle triggerRect = grid[r][c].rect;
                    triggerRect.width += 50;

                    // Check if any zombie stepped on it
                    for (int z=0; z<MAX_ZOMBIES; z++) {
                        if (zombies[z].active && zombies[z].row == r && CheckCollisionRecs(triggerRect, zombies[z].hitBox)) {
                            // KILL COMMAND: Set HP to negative (-200)
                            zombies[z].hp = -200;
                            // Remove Mine
                            grid[r][c].plantType = PLANT_NONE;
                            break;
                        }
                    }
                }
            }
            // ROSE: Dies of old age
            else if (grid[r][c].plantType == PLANT_ROSE) {
                if (grid[r][c].lifeTimer >= 10.0f) grid[r][c].plantType = PLANT_NONE;
            }
            // CHOMPER: Dies of old age
            else if (grid[r][c].plantType == PLANT_CHOMPER) {
                if (grid[r][c].lifeTimer >= 20.0f) grid[r][c].plantType = PLANT_NONE;
            }
        }
    }

    // --- PROJECTILE MOVEMENT & COLLISION ---
    for (int i=0; i<MAX_PROJECTILES; i++) {
        if (!projectiles[i].active) continue;

        // Move bullet right
        projectiles[i].position.x += 300 * dt;

        // Remove if off-screen
        if (projectiles[i].position.x > SCREEN_WIDTH) projectiles[i].active = false;

        // Check collision with zombies
        for(int z=0; z<MAX_ZOMBIES; z++) {
            if (zombies[z].active && zombies[z].row == projectiles[i].row && CheckCollisionCircleRec(projectiles[i].position, 10, zombies[z].hitBox)) {
                // Hit!
                projectiles[i].active = false; // Remove bullet
                zombies[z].hp -= projectiles[i].damage; // Hurt zombie

                // Zombie Died?
                if (zombies[z].hp <= 0 && zombies[z].hp > -100) {
                    zombies[z].active = false;
                    // Give reward if it was a Thinking Zombie
                    if (zombies[z].type == ZOMBIE_THINKING) {
                        thinkingZombiesKilledCounter++;
                        if (thinkingZombiesKilledCounter >= REWARD_KILL_THRESHOLD) {
                            sunCount += currentReward;
                            thinkingZombiesKilledCounter = 0;
                        }
                    }
                }
                break;
            }
        }
    }

    // --- ZOMBIE MOVEMENT & LOGIC ---
    for (int i=0; i<MAX_ZOMBIES; i++) {
        if (!zombies[i].active) continue;

        // Check for Mine Kill Code (-200)
        if (zombies[i].hp <= -200) { zombies[i].active = false; continue; }
        if (zombies[i].hp <= 0) { zombies[i].active = false; continue; }

        // Calculate Speed (Base * Row Slow * Difficulty Multiplier)
        float currentSpeed = zombies[i].speed * rowSpeedMultipliers[zombies[i].row] * dynamicSpeedMult;

        // --- THINKING ZOMBIE AI ---
        if (zombies[i].type == ZOMBIE_THINKING) {
            zombies[i].freezeTimer += dt;

            // Check every 1 second AND if switched less than 3 times
            if (zombies[i].freezeTimer > 1.0f && zombies[i].rowSwitchCount < 2) {
                zombies[i].freezeTimer = 0;
                int bestRow = PickBestStartRow(); // Ask brain for best row

                if (zombies[i].row != bestRow) {
                    zombies[i].row = bestRow;
                    zombies[i].rowSwitchCount++; // Increment switch count
                }
            }

            // Smoothly slide Y position to the new row
            float targetY = grid[zombies[i].row][0].rect.y + 10;
            if (fabs(zombies[i].position.y - targetY) > 2.0f) {
                if (zombies[i].position.y < targetY) zombies[i].position.y += 100 * dt;
                else zombies[i].position.y -= 100 * dt;
            }
        }

        // Move Left (if not eating)
        if (!zombies[i].eating) {
            zombies[i].position.x -= currentSpeed * 60.0f * dt;
        }

        // Update Hitbox Position
        zombies[i].hitBox.x = zombies[i].position.x + 20;
        zombies[i].hitBox.y = zombies[i].position.y;

        // --- EATING LOGIC ---
        zombies[i].eating = false;
        // Calculate which column the zombie is standing on
        int col = (int)((zombies[i].hitBox.x + 30 - GRID_START_X)/CELL_W);

        // If there is a plant here...
        if (col >= 0 && col < GRID_COLS && grid[zombies[i].row][col].plantType != PLANT_NONE) {

            // SPECIAL CASE: If it's a Mine AND it is Armed, STOP.
            // Don't eat it. Wait for it to explode you.
            if (grid[zombies[i].row][col].plantType == PLANT_MINE && grid[zombies[i].row][col].isArmed) {
                zombies[i].eating = false;
            }
            else {
                // Otherwise, nom nom nom.
                zombies[i].eating = true;
                // Damage = 20 HP per second (Takes 5s to eat 100HP plant)
                grid[zombies[i].row][col].hp -= 20.0f * dt;

                // Plant dead?
                if (grid[zombies[i].row][col].hp <= 0) grid[zombies[i].row][col].plantType = PLANT_NONE;
            }
        }

        // Game Over Check (Reached the house)
        if (zombies[i].position.x < -50) *currentScreen = GAME_OVER;
    }
}
#endif