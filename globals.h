#ifndef GLOBALS_H
#define GLOBALS_H

#include <raylib.h>

#define FLAG_FULLSCREEN 0x00000001
//Ekran boyutları
#define FULLSCREEN_WIDTH 1920
#define FULLSCREEN_HEIGHT 1080
#define WINDOWED_WIDTH 1280
#define WINDOWED_HEIGHT 720

//Pencere başlığı
#define TITLE "Zindan Yolcusu"


extern float scalefactor;
extern bool isfullscreen;
extern bool shouldToggleFullscreen;
extern Texture2D menu,d1,idle,walk;
extern Texture2D selectionpanel;
extern Texture2D header;
extern bool shouldExit;
extern bool showSettings;
extern Image icon;
extern Texture2D run;

enum Scene { SCENE_MENU, SCENE_EP1 };
extern Scene currentScene;

extern float fadeAlpha;
extern bool fading;
extern int fadeDirection; // 1: kararma, -1: açılma
extern Scene nextScene;

#endif
