#include "window_manager.h"
#include "globals.h"
#include "resource_manager.h"
#include "menu.h"

void RestartWindow(bool fullscreen,int width,int height,const char* title) {

    UnloadResources();
   SetConfigFlags(0);

   if(fullscreen) {
        SetConfigFlags(FLAG_WINDOW_UNDECORATED);
        SetConfigFlags(FLAG_FULLSCREEN);
   }

   InitWindow(width,height,title);
   
   SetWindowState(FLAG_WINDOW_ALWAYS_RUN);

   if(!fullscreen) {
    ClearWindowState(FLAG_WINDOW_UNDECORATED);
   }

   SetExitKey(0);

   LoadResources();
}

void ToggleWindowState(bool decoreted){
    if(decoreted) {
        ClearWindowState(FLAG_WINDOW_UNDECORATED); 
    }
    else {
        SetWindowState(FLAG_WINDOW_UNDECORATED);    
    }
}

void CleanWindow(){
    UnloadResources();
    CloseWindow();
}

