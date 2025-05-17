#ifndef HEALTH_SYSTEM_H
#define HEALTH_SYSTEM_H

#include <raylib.h>

// Dışarıdan erişilebilir değişkenler
extern float playerHealth;
extern float playerMaxHealth;
extern float energy;
extern float maxEnergy;
extern float mana;
extern float maxMana;
extern float totalSeconds;
extern double gameStartTime;

// Fonksiyonlar
void DrawHealthBar(float x, float y, float scalefactor, bool isMoving);
void UpdateEnergy(float deltaTime, bool isMoving, bool isRunning);

#endif // HEALTH_SYSTEM_H 