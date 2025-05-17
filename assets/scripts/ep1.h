#ifndef EP1_H
#define EP1_H

#include <raylib.h>

// Texture değişkenlerini dışa aktar
extern Texture2D d3_texture;
extern Texture2D d4_texture;
extern bool d3_loaded;
extern bool d4_loaded;

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

#endif // EP1_H 