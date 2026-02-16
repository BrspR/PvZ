#include "raylib.h"
#include <stdio.h>
#include "defines.h"
#include "game_ui.h"
#include "levels.h"
#include "level2.h"
#include "level3.h"

// --- GAME STATE ---
Cell grid[GRID_ROWS][GRID_COLS];
Zombie zombies[MAX_ZOMBIES];
Projectile projectiles[MAX_PROJECTILES];
Mower mowers[GRID_ROWS];
Sun suns[MAX_SUNS];
int sunCount = 500;
float sunSpawnTimer = 0.0f;

// --- GLOBALS ---
int selectedCard = -1;
float zombieSpawnTimer = 0.0f;
float gameOverTimer = 0.0f;

int selectedPlantType = 0;
float cooldowns[8] = {0};
// 1:Sun, 2:Pea, 3:Mine, 4:Chomper, 5:Rose, 6:Felfel
float maxCooldowns[8] = {0, 5.0f, 5.0f, 15.0f, 7.5f, 15.0f, 20.0f, 0};
// Costs: Felfel = 75
int plantCosts[8] = {0, 50, 100, 75, 150, 125, 75, 0};
Rectangle cardRects[7]; // Increased array size

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PvZ - Final Project");
    InitAudioDevice();
    SetTargetFPS(60);

    LoadGameAssets();

    GameScreen currentScreen = MENU;
    GameScreen prevScreen = -1;
    InitLevel();

    int menuSel = 0;
    // Starting X=150 to fit 6 cards comfortably
    for(int i=1; i<=6; i++) cardRects[i] = (Rectangle){150 + i*110, 20, 100, 110};

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        Vector2 mouse = GetMousePosition();

        UpdateMusicStream(musicTheme);
        UpdateMusicStream(musicStart);

        if (currentScreen == MENU) {
            if (prevScreen != MENU) { StopMusicStream(musicTheme); PlayMusicStream(musicStart); }
        }
        else if (currentScreen == LEVEL_1 || currentScreen == level_2 || currentScreen == LEVEL_3) {
            if (prevScreen == MENU || prevScreen == LEVEL_SELECT || prevScreen == VICTORY || prevScreen == GAME_OVER) {
                StopMusicStream(musicStart); PlaySound(soundRoar); PlayMusicStream(musicTheme);
            }
        }
        else if (currentScreen == VICTORY) { if (prevScreen != VICTORY) { StopMusicStream(musicTheme); PlaySound(soundWin); } }
        else if (currentScreen == GAME_OVER) { if (prevScreen != GAME_OVER) { StopMusicStream(musicTheme); PlaySound(soundLose); } }

        prevScreen = currentScreen;
        if (currentScreen == MENU) {
            if (IsKeyPressed(KEY_DOWN)) menuSel = (menuSel + 1) % 3;
            if (IsKeyPressed(KEY_UP)) menuSel = (menuSel - 1 + 3) % 3;
            if (IsKeyPressed(KEY_ENTER)) {
                if (menuSel == 0) currentScreen = LEVEL_SELECT;
                if (menuSel == 1) currentScreen = SHOP_SCREEN; // Enter Shop
                if (menuSel == 2) break;
            }
        }
        else if (currentScreen == SHOP_SCREEN) {
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = MENU;

            Rectangle backBtn = {50, 50, 150, 60};
            if (CheckCollisionPointRec(mouse, backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentScreen = MENU;
            }
        }


        else if (currentScreen == LEVEL_SELECT) {
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = MENU;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                float leftMargin = 50;
                float boxWidth = SCREEN_WIDTH * 0.45f;
                float boxHeight = (SCREEN_HEIGHT - 150) / 3.5f;

                if (mouse.x > leftMargin && mouse.x < leftMargin+boxWidth && mouse.y > 120 && mouse.y < 120+boxHeight) {
                    InitLevel();
                    maxCooldowns[1]=2.5f; maxCooldowns[2]=7.5f; maxCooldowns[3]=15.0f; maxCooldowns[4]=3.5f; maxCooldowns[5]=15.0f; maxCooldowns[6]=20.0f;
                    currentScreen = LEVEL_1;
                }
                float y2 = 120 + boxHeight + 30;
                if (mouse.x > leftMargin && mouse.x < leftMargin+boxWidth && mouse.y > y2 && mouse.y < y2+boxHeight) {
                    InitLevel2();
                    maxCooldowns[1]=3.5f; maxCooldowns[2]=12.0f; maxCooldowns[3]=22.5f; maxCooldowns[4]=5.0f; maxCooldowns[5]=15.0f; maxCooldowns[6]=20.0f;
                    currentScreen = level_2;
                }
                float y3 = y2 + boxHeight + 30;
                if (mouse.x > leftMargin && mouse.x < leftMargin+boxWidth && mouse.y > y3 && mouse.y < y3+boxHeight) {
                    InitLevel3();
                    // Felfel not in L3, so index 6 irrelevant here
                    maxCooldowns[2]=5.0f; maxCooldowns[3]=15.0f; maxCooldowns[4]=7.5f; maxCooldowns[5]=15.0f;maxCooldowns[6]=20.0f;
                    currentScreen = LEVEL_3;
                }
            }
        }
        else if (currentScreen == LEVEL_1 || currentScreen == level_2 || currentScreen == LEVEL_3) {
            for(int i=1; i<=6; i++) if(cooldowns[i] > 0) cooldowns[i] -= dt;

            if (currentScreen == LEVEL_1) UpdateGameLogic(&currentScreen);
            else if (currentScreen == level_2) UpdateLevel2Logic(&currentScreen, dt);
            else if (currentScreen == LEVEL_3) UpdateLevel3Logic(&currentScreen, dt);

            if (currentScreen != LEVEL_3 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                for(int i=0; i<MAX_SUNS; i++) {
                    if (suns[i].active) {
                        Rectangle sunHitbox = {suns[i].position.x - 25, suns[i].position.y - 25, 100, 100};
                        if (CheckCollisionPointRec(mouse, sunHitbox)) {
                            suns[i].active = false;
                            sunCount += 50;
                        }
                    }
                }
            }

            for(int i=1; i<=6; i++) {
                if (currentScreen == LEVEL_3 && i == 1)  continue;

                if (CheckCollisionPointRec(mouse, cardRects[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (sunCount >= plantCosts[i] && cooldowns[i] <= 0) {
                        selectedCard = i;
                        selectedPlantType = i;
                    }
                }
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && selectedCard != -1) {
                if (mouse.y > GRID_START_Y) {
                    int c = (int)((mouse.x - GRID_START_X) / CELL_W);
                    int r = (int)((mouse.y - GRID_START_Y) / CELL_H);

                    bool canPlant = false;
                    if (currentScreen == LEVEL_1 && c >= 1) canPlant = true;
                    if (currentScreen == LEVEL_3 && c >= 1) canPlant = true;
                    if (currentScreen == level_2 && c >= 1) canPlant = true;

                    if (canPlant && c < GRID_COLS && r >= 0 && r < GRID_ROWS && grid[r][c].plantType == PLANT_NONE) {
                        grid[r][c].plantType = selectedPlantType;

                        if (selectedPlantType == PLANT_MINE) grid[r][c].hp = 300;

                        else if (selectedPlantType == PLANT_FELFEL) grid[r][c].hp = 1000;
                        else grid[r][c].hp = 100;

                        grid[r][c].maxHp = grid[r][c].hp;
                        grid[r][c].isArmed = false;
                        grid[r][c].timer = 0;
                        grid[r][c].lifeTimer = 0;
                        grid[r][c].stateTimer = 0;

                        sunCount -= plantCosts[selectedPlantType];
                        cooldowns[selectedCard] = maxCooldowns[selectedCard];
                        selectedCard = -1;
                    }
                }
            }
            if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) selectedCard = -1;
        }
        else if (currentScreen == VICTORY || currentScreen == GAME_OVER) {
            if (IsKeyPressed(KEY_ENTER)) currentScreen = MENU;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (currentScreen == MENU) {
            DrawMenu(menuSel);
        }
        else if (currentScreen == SHOP_SCREEN) {
            ClearBackground((Color){40, 40, 50, 255}); // Dark Grey

            DrawText("SHOP IS CLOSED!", SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 50, 60, RED);
            DrawText("Come back later...", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 30, 30, LIGHTGRAY);

            Rectangle backBtn = {50, 50, 150, 60};
            DrawRectangleRec(backBtn, MAROON);
            DrawRectangleLinesEx(backBtn, 3, RED);
            DrawText("BACK", 85, 65, 30, WHITE);
        }
        else if (currentScreen == LEVEL_SELECT) {
            DrawLevelSelect();
        }
        else if (currentScreen == LEVEL_1 || currentScreen == level_2 || currentScreen == LEVEL_3) {
            DrawTexturePro(bgLevel1, (Rectangle){0,0,bgLevel1.width,bgLevel1.height}, (Rectangle){0,0,SCREEN_WIDTH,SCREEN_HEIGHT}, (Vector2){0,0}, 0, WHITE);

            for(int r=0; r<GRID_ROWS; r++) {
                for(int c=0; c<GRID_COLS; c++) {
                    DrawRectangleLinesEx(grid[r][c].rect, 1.0f, Fade(BLACK, 0.3f));
                    if (currentScreen == level_2 && c == 0) DrawRectangleRec(grid[r][c].rect, Fade(GRAY, 0.5f));
                }
            }

            for(int i=0; i<GRID_ROWS; i++) {
                if(mowers[i].active)
                    DrawTexturePro(tMower, (Rectangle){0,0,tMower.width,tMower.height}, (Rectangle){mowers[i].x, grid[i][0].rect.y+25, 80, 70}, (Vector2){0,0}, 0, WHITE);
            }

            for(int r=0; r<GRID_ROWS; r++) {
                for(int c=0; c<GRID_COLS; c++) {
                    if(grid[r][c].plantType != PLANT_NONE) {
                        int type = grid[r][c].plantType;

                        float scaleX = 1.0f;
                        float scaleY = 1.0f;

                        if (type == PLANT_PEASHOOTER) { scaleX = 0.7f; scaleY = 0.7f; }
                        else if (type == PLANT_ROSE) { scaleX = 1.2f; scaleY = 1.0f; }
                        else if (type == PLANT_MINE) { scaleX = 0.7f; scaleY = 0.7f; }
                        else if (type == PLANT_FELFEL) { scaleX = 0.8f; scaleY = 0.8f; } // Scale Felfel

                        float w = plantsTex[type].width;
                        float h = plantsTex[type].height;

                        Rectangle dest = {
                            grid[r][c].rect.x + (grid[r][c].rect.width - grid[r][c].rect.width*scaleX)/2,
                            grid[r][c].rect.y + (grid[r][c].rect.height - grid[r][c].rect.height*scaleY)/2,
                            grid[r][c].rect.width * scaleX,
                            grid[r][c].rect.height * scaleY
                        };

                        if (type == PLANT_MINE && grid[r][c].lifeTimer < MINE_ARM_TIME) {
                             DrawTexturePro(plantsTex[type], (Rectangle){0,0,w,h}, dest, (Vector2){0,-20}, 0, Fade(WHITE, 0.6f));
                        }
                        // --- NEW: FELFEL DRAWING (Fade in while arming) ---
                        else if (type == PLANT_FELFEL && grid[r][c].lifeTimer < FELFEL_ARM_TIME) {
                             DrawTexturePro(plantsTex[type], (Rectangle){0,0,w,h}, dest, (Vector2){0,0}, 0, Fade(WHITE, 0.5f));
                        }
                        else {
                             DrawTexturePro(plantsTex[type], (Rectangle){0,0,w,h}, dest, (Vector2){0,0}, 0, WHITE);
                        }
                    }
                }
            }

            for(int i=0; i<MAX_ZOMBIES; i++) {
                if(zombies[i].active) {
                    Texture2D zTex = (zombies[i].type == ZOMBIE_THINKING) ? tThinkingZombie : tZombie;

                    float zScale = 1.2f;
                    if (zombies[i].type == ZOMBIE_NORMAL) zScale = 1.0f;
                    if (zombies[i].type == ZOMBIE_THINKING) zScale = 1.2f;

                    DrawTexturePro(zTex, (Rectangle){0,0,zTex.width, zTex.height},
                        (Rectangle){zombies[i].position.x, zombies[i].position.y - (110*zScale - 110)/2, 80 * zScale, 110 * zScale},
                        (Vector2){0,0}, 0, WHITE);
                }
            }

            for(int i=0; i<MAX_PROJECTILES; i++) {
                if(projectiles[i].active) {

                        DrawCircleV(projectiles[i].position, 10, GREEN);

                }
            }

            if (currentScreen != LEVEL_3) {
                for(int i=0; i<MAX_SUNS; i++) {
                    if(suns[i].active) {
                        if (sunTex.id > 0) {
                            DrawTexturePro(sunTex, (Rectangle){0,0,sunTex.width,sunTex.height},
                                (Rectangle){suns[i].position.x, suns[i].position.y, 75, 75}, (Vector2){0,0}, 0, WHITE);
                        } else {
                            DrawCircle(suns[i].position.x + 25, suns[i].position.y + 25, 25, ORANGE);
                        }
                    }
                }
            }


            DrawGameOverlay(sunCount, currentScreen, thinkingZombiesKilledCounter, cardRects, plantCosts, cooldowns, maxCooldowns, selectedCard);
        }
        else {
            DrawVictoryOrGameOver(currentScreen);
        }

        EndDrawing();
    }

    UnloadGameAssets();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}