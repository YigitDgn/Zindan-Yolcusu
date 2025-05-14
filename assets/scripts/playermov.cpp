#include <raylib.h>
#include "../../globals.h"

// Karakterin sprite sheet bilgileri
#define PLAYER_IDLE_FRAMES 5
#define PLAYER_WALK_FRAMES 9
#define PLAYER_WALK_COLS 3
#define PLAYER_WALK_ROWS 3
#define PLAYER_WALK_FRAME_W 128
#define PLAYER_WALK_FRAME_H 128
#define PLAYER_FRAME_SPEED 8 // Animasyon hızı
#define PLAYER_RUN_FRAMES 8 // 9. frame boş, sadece 8 frame var
#define PLAYER_RUN_COLS 3
#define PLAYER_RUN_ROWS 3
#define PLAYER_RUN_FRAME_W 128
#define PLAYER_RUN_FRAME_H 128

// Karakterin fiziksel ve animasyonel durumu
struct Player {
    float x, y;
    float speed;
    int frame;
    float frameTimer;
    bool moving;
    bool facingRight;
};

Player player;

void InitPlayer(float startX, float startY) {
    player.x = startX;
    player.y = startY;
    player.speed = 3.0f;
    player.frame = 0;
    player.frameTimer = 0.0f;
    player.moving = false;
    player.facingRight = true;
}

void UpdatePlayer() {
    player.moving = false;
    bool running = false;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        player.x += player.speed;
        player.moving = true;
        player.facingRight = true;
    }
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        player.x -= player.speed;
        player.moving = true;
        player.facingRight = false;
    }
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        running = true;
        player.speed = 6.0f;
    } else {
        player.speed = 3.0f;
    }
    // Animasyon frame güncelle
    int maxFrames = player.moving ? (running ? PLAYER_RUN_FRAMES : PLAYER_WALK_FRAMES) : PLAYER_IDLE_FRAMES;
    player.frameTimer += GetFrameTime() * PLAYER_FRAME_SPEED;
    if (player.frameTimer >= 1.0f) {
        player.frame = (player.frame + 1) % maxFrames;
        player.frameTimer = 0.0f;
    }
}

void DrawPlayer(float scalefactor) {
    bool running = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) && player.moving;
    Texture2D* tex = &idle;
    int maxFrames = PLAYER_IDLE_FRAMES;
    if (player.moving) {
        if (running) {
            tex = &run;
            maxFrames = PLAYER_RUN_FRAMES;
        } else {
            tex = &walk;
            maxFrames = PLAYER_WALK_FRAMES;
        }
    }
    Rectangle src;
    if (player.moving && running) {
        int col = player.frame % PLAYER_RUN_COLS;
        int row = player.frame / PLAYER_RUN_COLS;
        src = (Rectangle){ (float)(col * PLAYER_RUN_FRAME_W), (float)(row * PLAYER_RUN_FRAME_H), (float)PLAYER_RUN_FRAME_W, (float)PLAYER_RUN_FRAME_H };
        if (!player.facingRight) src.width = -src.width;
    } else if (player.moving) {
        int col = player.frame % PLAYER_WALK_COLS;
        int row = player.frame / PLAYER_WALK_COLS;
        src = (Rectangle){ (float)(col * PLAYER_WALK_FRAME_W), (float)(row * PLAYER_WALK_FRAME_H), (float)PLAYER_WALK_FRAME_W, (float)PLAYER_WALK_FRAME_H };
        if (!player.facingRight) src.width = -src.width;
    } else {
        int frameWidth = tex->width / maxFrames;
        int frameHeight = tex->height;
        src = (Rectangle){ (float)(player.frame * frameWidth), 0, (float)frameWidth, (float)frameHeight };
        if (!player.facingRight) src.width = -src.width;
    }
    float drawX = player.x;
    float drawY = player.y;
    float scale = scalefactor * 2.0f; // Karakteri 2 kat büyüt
    float drawW = (player.moving ? (running ? PLAYER_RUN_FRAME_W : PLAYER_WALK_FRAME_W) : tex->width / maxFrames) * scale;
    float drawH = (player.moving ? (running ? PLAYER_RUN_FRAME_H : PLAYER_WALK_FRAME_H) : tex->height) * scale;
    DrawTexturePro(*tex, src, (Rectangle){drawX, drawY, drawW, drawH}, (Vector2){0,0}, 0.0f, WHITE);
}
