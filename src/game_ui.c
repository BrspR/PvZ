#include "game_ui.h"
#include <stdio.h>

// Textures
Texture2D bgMenu, bgLevel1, bgLevel3;
Texture2D tMower, tZombie, tThinkingZombie;
Texture2D sunTex;
Texture2D plantsTex[8]; // Array holds all plant textures
Texture2D imgLevel1, imgLevel2, imgLevel3;
Texture2D peaTex;

// Audio
Music musicTheme;
Music musicStart;
Sound soundRoar;
Sound soundWin;
Sound soundLose;

void LoadGameAssets() {
    // 1. Visuals
    bgMenu = LoadTexture("assets/bg.jpg");
    bgLevel1 = LoadTexture("assets/chaman.jpg");
    bgLevel3 = LoadTexture("assets/level3bg.jpg");

    tMower = LoadTexture("assets/mover2.png");
    tZombie = LoadTexture("assets/zombie.png");
    sunTex = LoadTexture("assets/sun.png");
    peaTex = LoadTexture("assets/pea.png");

    imgLevel1 = LoadTexture("assets/level1.png");
    imgLevel2 = LoadTexture("assets/level2.png");
    imgLevel3 = LoadTexture("assets/level3.png");

    if (FileExists("assets/braindeadzombie.png")) {
        tThinkingZombie = LoadTexture("assets/braindeadzombie.png");
    } else if (FileExists("assets/braindead.png")) {
        tThinkingZombie = LoadTexture("assets/braindead.png");
    } else {
        tThinkingZombie = LoadTexture("assets/zombie.png");
    }

    plantsTex[1] = LoadTexture("assets/sunflower.png");
    plantsTex[2] = LoadTexture("assets/peashooter.png");
    plantsTex[3] = LoadTexture("assets/mine.png");
    plantsTex[4] = LoadTexture("assets/chumper.png");
    plantsTex[5] = LoadTexture("assets/rose.png");

    // --- NEW: Load Felfel Texture ---
    if (FileExists("assets/felfel.png")) {
        plantsTex[6] = LoadTexture("assets/felfel.png");
    }

    // 2. Audio
    if (FileExists("assets/themesong.ogg")) musicTheme = LoadMusicStream("assets/themesong.ogg");
    else musicTheme = LoadMusicStream("assets/themesong.m4a");

    musicStart = LoadMusicStream("assets/startmusic.ogg");
    soundRoar = LoadSound("assets/roar.ogg");
    soundLose = LoadSound("assets/lose.ogg");

    if(FileExists("assets/win.mp3")) soundWin = LoadSound("assets/win.mp3");
    else soundWin = LoadSound("assets/win.ogg");

    musicTheme.looping = true;
    musicStart.looping = false;
}

void UnloadGameAssets() {
    UnloadTexture(bgMenu); UnloadTexture(bgLevel1); UnloadTexture(bgLevel3);
    UnloadTexture(tMower); UnloadTexture(tZombie); UnloadTexture(tThinkingZombie);
    UnloadTexture(sunTex); UnloadTexture(peaTex);
    UnloadTexture(imgLevel1); UnloadTexture(imgLevel2); UnloadTexture(imgLevel3);
    for(int i=1; i<8; i++) UnloadTexture(plantsTex[i]);

    UnloadMusicStream(musicTheme); UnloadMusicStream(musicStart);
    UnloadSound(soundRoar); UnloadSound(soundWin); UnloadSound(soundLose);
}

void DrawMenu(int menuSelection) {
    DrawTexturePro(bgMenu, (Rectangle){0,0,bgMenu.width,bgMenu.height}, (Rectangle){0,0,SCREEN_WIDTH,SCREEN_HEIGHT}, (Vector2){0,0}, 0.0f, WHITE);

    const char* options[] = {"PLAY", "SHOP", "EXIT"};
    int exitY = SCREEN_HEIGHT - 350;
    int spacing = 220;
    int yPositions[3];
    yPositions[0] = exitY - (2 * spacing);
    yPositions[1] = exitY - spacing;
    yPositions[2] = exitY;
    for(int i=0; i<3; i++) {
        Color c = (i == menuSelection) ? YELLOW : WHITE;
        DrawText(options[i], SCREEN_WIDTH/2 - 60, yPositions[i], 50, c);
        if(i == menuSelection) DrawText(">", SCREEN_WIDTH/2 - 100, yPositions[i], 50, YELLOW);
    }
}

void DrawLevelSelect() {
    ClearBackground((Color){20, 25, 30, 255});
    DrawText("SELECT LEVEL", SCREEN_WIDTH * 0.6f, 50, 60, GOLD);
    float leftMargin = 50;
    float boxWidth = SCREEN_WIDTH * 0.45f;
    float boxHeight = (SCREEN_HEIGHT - 150) / 3.5f;
    float textX = leftMargin + boxWidth + 50;

    Rectangle r1 = {leftMargin, 120, boxWidth, boxHeight};
    if (imgLevel1.id > 0) DrawTexturePro(imgLevel1, (Rectangle){0,0,imgLevel1.width,imgLevel1.height}, r1, (Vector2){0,0}, 0, WHITE);
    DrawRectangleLinesEx(r1, 4, WHITE);
    DrawText("LEVEL 1: Normal", textX, 120 + 40, 40, WHITE);
    DrawText("Standard Day Mode", textX, 120 + 90, 25, LIGHTGRAY);

    float y2 = 120 + boxHeight + 30;
    Rectangle r2 = {leftMargin, y2, boxWidth, boxHeight};
    if (imgLevel2.id > 0) DrawTexturePro(imgLevel2, (Rectangle){0,0,imgLevel2.width,imgLevel2.height}, r2, (Vector2){0,0}, 0, WHITE);
    DrawRectangleLinesEx(r2, 4, SKYBLUE);
    DrawText("LEVEL 2: No Gas", textX, y2 + 40, 40, SKYBLUE);
    DrawText("Braindead Zombies", textX, y2 + 90, 25, LIGHTGRAY);

    float y3 = y2 + boxHeight + 30;
    Rectangle r3 = {leftMargin, y3, boxWidth, boxHeight};
    if (imgLevel3.id > 0) DrawTexturePro(imgLevel3, (Rectangle){0,0,imgLevel3.width,imgLevel3.height}, r3, (Vector2){0,0}, 0, WHITE);
    DrawRectangleLinesEx(r3, 4, PURPLE);
    DrawText("LEVEL 3: Dark Night", textX, y3 + 40, 40, PURPLE);
    DrawText("No Sunflowers!", textX, y3 + 90, 25, LIGHTGRAY);
    DrawText("Press ESC to return", SCREEN_WIDTH - 300, SCREEN_HEIGHT - 50, 20, GRAY);
}

void DrawGameOverlay(int sunCount, GameScreen screen, int killed, Rectangle cards[], int costs[], float cooldowns[], float maxCooldowns[], int selected) {
    DrawRectangle(0, 0, SCREEN_WIDTH, 140, Fade(BROWN, 0.9f));
    DrawCircle(50, 70, 30, YELLOW);
    DrawText(TextFormat("%d", sunCount), 90, 50, 50, YELLOW);

    if (screen == level_2) {
        DrawText("LEVEL 2", SCREEN_WIDTH - 200, 30, 30, SKYBLUE);
        DrawText(TextFormat("Kills: %d", killed), SCREEN_WIDTH - 200, 70, 30, WHITE);
    }
    if (screen == 4) {
        DrawText("LEVEL 3: NIGHT", SCREEN_WIDTH - 250, 30, 30, PURPLE);
        DrawText("No Sun", SCREEN_WIDTH - 250, 70, 30, RED);
    }

    for (int i = 1; i <= 6; i++) {
        if (screen == 4 && i == 1) continue;


        //if (screen == 4 && i == 6) continue;

        Color btnColor = (sunCount >= costs[i] && cooldowns[i] <= 0) ? WHITE : GRAY;
        if (selected == i) btnColor = GREEN;

        DrawRectangleRec(cards[i], Fade(LIGHTGRAY, 0.5f));

        if (plantsTex[i].id > 0) {
            Rectangle dest = cards[i];
            if (i == 5) { // Rose Scaling
                dest.width *= 1.5f;
                dest.x -= (dest.width - cards[i].width) / 2.0f;
            }
            DrawTexturePro(plantsTex[i], (Rectangle){0,0,plantsTex[i].width,plantsTex[i].height},
                           dest, (Vector2){0,0}, 0.0f, btnColor);
        }

        if(cooldowns[i] > 0) {
            float ratio = cooldowns[i] / maxCooldowns[i];
            DrawRectangleRec((Rectangle){cards[i].x, cards[i].y + (1.0f-ratio)*cards[i].height, cards[i].width, ratio*cards[i].height}, Fade(BLACK, 0.7f));
        }
        DrawText(TextFormat("%d", costs[i]), (int)cards[i].x + 20, (int)cards[i].y + 85, 20, BLACK);
    }
}

void DrawVictoryOrGameOver(GameScreen screen) {
    if (screen == VICTORY) {
        ClearBackground(GREEN);
        DrawText("VICTORY!", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 50, 80, WHITE);
    } else {
        ClearBackground(BLACK);
        DrawText("GAME OVER", SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 - 50, 80, RED);
    }
    DrawText("Press ENTER to Menu", SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 + 80, 20, WHITE);
}