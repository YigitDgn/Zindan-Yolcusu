#include <vector>
#include "resource_manager.h"
#include "globals.h"

void LoadResources(){
    menu = LoadTexture("assets/sprites/env/menu.png");
    d1 = LoadTexture("assets/sprites/env/d1.png");
    idle = LoadTexture("assets/sprites/player/idle.png");
    walk = LoadTexture("assets/sprites/player/walk.png");
    run = LoadTexture("assets/sprites/player/run.png");
    selectionpanel = LoadTexture("assets/sprites/env/selectionpanel.png");
    header = LoadTexture("assets/sprites/env/header.png");
}

void UnloadResources(){
    UnloadTexture(menu);
    UnloadTexture(d1);
    UnloadTexture(idle);
    UnloadTexture(walk);
    UnloadTexture(run);
    UnloadTexture(selectionpanel);
    UnloadTexture(header);
}




