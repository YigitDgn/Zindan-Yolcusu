#include "globals.h"

Texture2D menu,d1,idle,walk;
Texture2D selectionpanel;
Texture2D header;
Texture2D run;

float scalefactor = 1.0f;
bool isfullscreen = false;
bool shouldToggleFullscreen = false;
bool shouldExit = false;
bool showSettings = false;
Image icon = LoadImage("main.png");

Scene currentScene = SCENE_MENU;

float fadeAlpha = 0.0f;
bool fading = false;
int fadeDirection = 1;
Scene nextScene = SCENE_MENU;

