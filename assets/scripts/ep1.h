#pragma once
#include <raylib.h>

void InitEp1Scene();
void UnloadEp1Scene();
void DrawEp1Scene(float currentWidth, float currentHeight, float scalefactor);
void DrawHealthBar(float x, float y, float scalefactor, bool isMoving);
void UpdateEnergy(float deltaTime, bool isMoving, bool isRunning);
void InitInventory();
void UnloadInventory();
void DrawInventory(float scalefactor);
void UpdateEp1Scene(float deltaTime);
void DrawPauseMenu(float scalefactor);
void UpdatePauseMenu();
void StartNewGame(); 