#ifndef DEBUG_H
#define DEBUG_H

#include <raylib.h>
#include "enemy.h"

// Debug konsolu fonksiyonları
void UpdateDebugConsole();
void DrawDebugConsole();
void DrawHitboxes(Zombie* zombies, int zombieCount);
void DrawRegionNumbers(); // Bölge numaralarını çizme fonksiyonu

#endif // DEBUG_H 