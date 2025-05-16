#include <vector>
#include "resource_manager.h"
#include "globals.h"

void LoadResources(){
    menu = LoadTexture("assets/sprites/env/menu.png");
    d1 = LoadTexture("assets/sprites/env/d1.png");
    d3 = LoadTexture("assets/sprites/env/d3.png");
    d4 = LoadTexture("assets/sprites/env/d4.png");
    idle = LoadTexture("assets/sprites/player/idle.png");
    walk = LoadTexture("assets/sprites/player/walk.png");
    run = LoadTexture("assets/sprites/player/run.png");
    selectionpanel = LoadTexture("assets/sprites/env/selectionpanel.png");
    header = LoadTexture("assets/sprites/env/header.png");
    sword = LoadTexture("assets/sprites/player/rustedsword.png");
    healthbar = LoadTexture("assets/sprites/env/healthbar.png");
    rustedSwordIdle = LoadTexture("assets/sprites/player/rustedsword/idle.png");
    rustedSwordWalk = LoadTexture("assets/sprites/player/rustedsword/walk.png");
    rustedSwordRun = LoadTexture("assets/sprites/player/rustedsword/run.png");
}

void UnloadResources(){
    UnloadTexture(menu);
    UnloadTexture(d1);
    UnloadTexture(d3);
    UnloadTexture(d4);
    UnloadTexture(idle);
    UnloadTexture(walk);
    UnloadTexture(run);
    UnloadTexture(selectionpanel);
    UnloadTexture(header);
    UnloadTexture(sword);
    UnloadTexture(healthbar);
    UnloadTexture(rustedSwordIdle);
    UnloadTexture(rustedSwordWalk);
    UnloadTexture(rustedSwordRun);
}




