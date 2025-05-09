#ifndef MENU_H
#define MENU_H

#include <raylib.h>

struct menuState {
    bool showSettings;
    int language;
    float musicVolume;
    float sfxVolume;
};

void InitializeMenu();
void UpdateMenu(float currentWidth,float currentHeight);


#endif
