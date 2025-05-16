#include "globals.h"

float scalefactor = 1.0f;
bool isfullscreen = false;
bool shouldToggleFullscreen = false;
bool shouldExit = false;
bool showSettings = false;
Scene currentScene = SCENE_MENU;
float fadeAlpha = 0.0f;
bool fading = false;
int fadeDirection = 1;
Scene nextScene = SCENE_MENU;

// Texture tanımlamaları
Texture2D menu;
Texture2D d1;
Texture2D d3;
Texture2D d4;
Texture2D idle;
Texture2D walk;
Texture2D run;
Texture2D selectionpanel;
Texture2D header;
Texture2D sword;
Texture2D healthbar;
Texture2D rustedSwordIdle;
Texture2D rustedSwordWalk;
Texture2D rustedSwordRun;
Image icon;

