#ifndef MENU_H
#define MENU_H

#include <raylib.h>

extern bool shouldToggleFullscreen;
extern struct menuState m;

struct menuState {
    bool showSettings;
    int language;
    float musicVolume;
    float sfxVolume;
    int screenMode;  // 0: Pencere modu, 1: Tam ekran
    bool showPlayPanel; // Oyna paneli açık mı
    bool playPanelJustOpened; // Oyna paneli yeni mi açıldı
};

void InitializeMenu();
void UpdateMenu(float currentWidth,float currentHeight);
void DrawMenu(float currentWidth, float currentHeight);


#endif
