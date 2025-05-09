#include "menu.h"
#include "globals.h"

static menuState m {
    .showSettings = false,
    .language = 0,
    .musicVolume = 1.0f,
    .sfxVolume = 1.0f
};

static bool shouldExit = false;

void InitializeMenu() {
    m.showSettings = false;
    m.language = 0;
    m.musicVolume = 0.7f;
    m.sfxVolume = 1.0f; 
    shouldExit = false;
}

void UpdateMenu(float currentWidth,float currentHeight) {
    float settingScale = currentWidth / 1920.0f;
}
    
    
    
