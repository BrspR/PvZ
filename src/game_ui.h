#ifndef GAME_UI_H
#define GAME_UI_H

#include "defines.h"

// --- Visual Assets (Global Textures) ---
extern Texture2D bgMenu, bgLevel1, bgLevel3; // Backgrounds
extern Texture2D tMower, tZombie, tThinkingZombie; // Entities
extern Texture2D sunTex; // Sun
extern Texture2D plantsTex[8]; // Plant Array
extern Texture2D imgLevel1, imgLevel2, imgLevel3; // Level Select Buttons
extern Texture2D peaTex; // Projectile Texture

// --- Audio Assets ---
extern Music musicTheme;  // Looping background music
extern Music musicStart;  // Menu music (plays once)
extern Sound soundRoar;   // Level start sound
extern Sound soundWin;    // Victory sound
extern Sound soundLose;   // Defeat sound

// --- Function Prototypes ---
void LoadGameAssets();
void UnloadGameAssets();
void DrawMenu(int menuSelection);
void DrawLevelSelect();
void DrawGameOverlay(int sunCount, GameScreen screen, int killed, Rectangle cards[], int costs[], float cooldowns[], float maxCooldowns[], int selected);
void DrawVictoryOrGameOver(GameScreen screen);

#endif