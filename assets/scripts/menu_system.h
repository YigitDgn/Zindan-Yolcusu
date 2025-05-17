#pragma once

#include <raylib.h>

// Menü sistemi fonksiyonları
void DrawPauseMenu(float scalefactor);
void UpdatePauseMenu(float scalefactor);
bool ShouldReturnToMainMenu();
void ResetReturnToMainMenu();
void CheckPauseInput();

// Dışarıdan erişilebilir değişkenler
extern bool isPaused;
extern float musicVolume;
extern float sfxVolume;
extern int screenMode; 