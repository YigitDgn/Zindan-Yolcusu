#include <raylib.h>
#include "globals.h"
#include "window_manager.h"



int main() {
    //Önce pencere modunu ayarlayalım
    InitWindow(WINDOWED_WIDTH,WINDOWED_HEIGHT,TITLE);
    SetTargetFPS(60);
    

    isfullscreen= true;
    scalefactor= 1.0f;
    if(isfullscreen) {
        RestartWindow(isfullscreen,FULLSCREEN_WIDTH,FULLSCREEN_HEIGHT,TITLE);
    } else {
        RestartWindow(isfullscreen,WINDOWED_WIDTH,WINDOWED_HEIGHT,TITLE);
    }


    while(!WindowShouldClose()){
        float currentWidth = GetScreenWidth();
        float currentHeight = GetScreenHeight();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(menu,(Rectangle){0,0,(float)menu.width,(float)menu.height},
        (Rectangle){0,0,currentWidth,currentHeight},
        (Vector2){0,0},0.0f,WHITE);
        EndDrawing();
    }

    CleanWindow();

}
