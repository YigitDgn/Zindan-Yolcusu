#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <raylib.h>

void RestartWindow(bool fullscreen,int width,int height,const char* title);
void ToggleWindowState(bool decoreted);
void CleanWindow();

#endif

