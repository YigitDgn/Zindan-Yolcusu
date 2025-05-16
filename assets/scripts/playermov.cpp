#include <raylib.h>
#include <stdio.h>
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
#define PLAYER_JUMP_FRAMES 5
#define PLAYER_JUMP_ROWS 1
#define PLAYER_JUMP_FRAME_W 128
#define PLAYER_JUMP_FRAME_H 128

// Animasyon setleri
Texture2D default_idle;
Texture2D default_walk;
Texture2D default_run;
Texture2D sword_idle;  // Kılıçlı idle animasyonu
Texture2D sword_walk;  // Kılıçlı yürüme animasyonu
Texture2D sword_run;   // Kılıçlı koşma animasyonu
Texture2D jumpside;

// Karakterin fiziksel ve animasyonel durumu
struct Player {
    float x, y;
    float speed;
    int frame;
    float frameTimer;
    bool moving;
    bool facingRight;
    bool hasSword;  // Kılıç var mı?
    float velocityY; // Zıplama için dikey hız
    bool onGround;  // Yerde mi?
};

Player player;

extern float energy;

bool isJumping = false;

void UpdateEnergy(float deltaTime, bool isMoving, bool isRunning);

void InitPlayer(float startX, float startY) {
    // Texture'ları yükle
    default_idle = LoadTexture("assets/sprites/player/default/idle.png");
    default_walk = LoadTexture("assets/sprites/player/default/walk.png");
    default_run = LoadTexture("assets/sprites/player/default/run.png");
    jumpside = LoadTexture("assets/sprites/player/default/jumpside.png");
    sword_idle = LoadTexture("assets/sprites/player/rustedsword/idle.png");
    sword_walk = LoadTexture("assets/sprites/player/rustedsword/walk.png");
    sword_run = LoadTexture("assets/sprites/player/rustedsword/run.png");
    

    player.x = startX;
    player.y = startY;
    player.speed = 3.0f;
    player.frame = 0;
    player.frameTimer = 0.0f;
    player.moving = false;
    player.facingRight = true;
    player.hasSword = false;
    isJumping = false;
    player.velocityY = 0.0f;
    player.onGround = true;
}

void UpdatePlayer() {
    player.moving = false;
    bool running = false;
    float gravity = 1.2f; // Yerçekimi
    float jumpStrength = 24.0f; // Daha yüksek zıplama
    float groundY = GetScreenHeight() - 256 * scalefactor; // Yerin y pozisyonu (ep1.cpp ile uyumlu)

    // Zıplama başlat
    if ((IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && player.onGround) {
        isJumping = true;
        player.frame = 0;
        player.frameTimer = 0.0f;
        player.velocityY = -jumpStrength;
        player.onGround = false;
        energy -= 15;
    }
    // Zıplama sırasında fizik
    if (!player.onGround) {
        player.y += player.velocityY;
        player.velocityY += gravity;
        if (player.y >= groundY) {
            player.y = groundY;
            player.velocityY = 0.0f;
            player.onGround = true;
            isJumping = false;
        }
    }
    if (isJumping) {
        // Zıplama animasyonu frame ilerlet
        int maxFrames = PLAYER_JUMP_FRAMES;
        player.frameTimer += GetFrameTime() * PLAYER_FRAME_SPEED;
        if (player.frameTimer >= 1.0f) {
            player.frame = (player.frame + 1) % maxFrames;
            player.frameTimer = 0.0f;
        }
        // Yatay hareket zıplarken de devam edebilir
        float jumpMoveSpeed = player.speed * 1.5f;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
            player.x += jumpMoveSpeed;
            player.moving = true;
            player.facingRight = true;
        }
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
            player.x -= jumpMoveSpeed;
            player.moving = true;
            player.facingRight = false;
        }
        return;
    }
    if (energy > 0.0f) {
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
    } else {
        // Enerji yoksa idle'da kal
        player.moving = false;
        player.speed = 3.0f;
    }

    // Animasyon frame güncelle
    int maxFrames = player.moving ? (running ? PLAYER_RUN_FRAMES : PLAYER_WALK_FRAMES) : PLAYER_IDLE_FRAMES;
    player.frameTimer += GetFrameTime() * PLAYER_FRAME_SPEED;
    if (player.frameTimer >= 1.0f) {
        player.frame = (player.frame + 1) % maxFrames;
        player.frameTimer = 0.0f;
    }

    // Enerji güncelle (hareket halindeyken yenilenme olmasın)
    UpdateEnergy(GetFrameTime(), player.moving, running);
}

void DrawPlayer(float scalefactor) {
    bool running = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) && player.moving;
    // Zıplama animasyonu
    if (isJumping && player.moving) {
        int frameW = PLAYER_JUMP_FRAME_W;
        int frameH = PLAYER_JUMP_FRAME_H;
        int col = player.frame;
        Rectangle src = { (float)(col * frameW), 0, (float)frameW, (float)frameH };
        if (!player.facingRight) src.width = -src.width;
        float drawX = player.x;
        float drawY = player.y;
        float scale = scalefactor * 2.0f;
        float drawW = frameW * scale;
        float drawH = frameH * scale;
        DrawTexturePro(jumpside, src, (Rectangle){drawX, drawY, drawW, drawH}, (Vector2){0,0}, 0.0f, WHITE);
        return;
    }
    // Kılıç durumuna göre texture seç
    Texture2D* tex;
    if (player.hasSword) {
        if (player.moving) {
            if (running) {
                tex = &sword_run;  // Kılıçlı koşma animasyonu
            } else {
                tex = &sword_walk;  // Kılıçlı yürüme animasyonu
            }
        } else {
            tex = &sword_idle;  // Kılıçlı idle animasyonu
        }
    } else {
        if (player.moving) {
            if (running) {
                tex = &default_run;
            } else {
                tex = &default_walk;
            }
        } else {
            tex = &default_idle;
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
        int frameWidth = tex->width / PLAYER_IDLE_FRAMES;
        int frameHeight = tex->height;
        src = (Rectangle){ (float)(player.frame * frameWidth), 0, (float)frameWidth, (float)frameHeight };
        if (!player.facingRight) src.width = -src.width;
    }

    float drawX = player.x;
    float drawY = player.y;
    float scale = scalefactor * 2.0f; // Karakteri 2 kat büyüt
    float drawW = (player.moving ? (running ? PLAYER_RUN_FRAME_W : PLAYER_WALK_FRAME_W) : PLAYER_WALK_FRAME_W) * scale;
    float drawH = (player.moving ? (running ? PLAYER_RUN_FRAME_H : PLAYER_WALK_FRAME_H) : PLAYER_WALK_FRAME_H) * scale;
    DrawTexturePro(*tex, src, (Rectangle){drawX, drawY, drawW, drawH}, (Vector2){0,0}, 0.0f, WHITE);
}


