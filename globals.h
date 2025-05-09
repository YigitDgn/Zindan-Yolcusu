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
extern Texture2D menu,d1,idle,walk;


#endif
