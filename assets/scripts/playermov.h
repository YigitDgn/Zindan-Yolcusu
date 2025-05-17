#ifndef PLAYERMOV_H
#define PLAYERMOV_H

#include <raylib.h>

struct Player {
    float x, y;
    float speed;
    int frame;
    float frameTimer;
    bool moving;
    bool facingRight;
    bool hasSword;  // Kılıç var mı?
};

void InitPlayer(float startX, float startY);
void UpdatePlayer();
void DrawPlayer(float scalefactor);

extern struct Player player;
extern double lastDamageTime;

#endif 