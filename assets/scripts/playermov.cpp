#include <raylib.h>
#include <stdio.h>
#include "../../globals.h"
#include "enemy.h" // Zombie yapısı için

// Karakterin sprite sheet bilgileri
#define PLAYER_IDLE_FRAMES 5
#define PLAYER_WALK_FRAMES 9
#define PLAYER_WALK_COLS 3
#define PLAYER_WALK_ROWS 3
#define PLAYER_WALK_FRAME_W 128
#define PLAYER_WALK_FRAME_H 128
#define PLAYER_FRAME_SPEED 4 // Animasyon hızı
#define PLAYER_RUN_FRAMES 8 // 9. frame boş, sadece 8 frame var
#define PLAYER_RUN_COLS 3
#define PLAYER_RUN_ROWS 3
#define PLAYER_RUN_FRAME_W 128
#define PLAYER_RUN_FRAME_H 128
#define PLAYER_JUMP_FRAMES 5
#define PLAYER_JUMP_ROWS 1
#define PLAYER_JUMP_FRAME_W 128
#define PLAYER_JUMP_FRAME_H 128
#define PLAYER_ATTACK_FRAMES 4 // Saldırı animasyonu için frame sayısı
#define PLAYER_ATTACK_FRAME_W 128
#define PLAYER_ATTACK_FRAME_H 128

// Animasyon setleri
Texture2D default_idle;
Texture2D default_walk;
Texture2D default_run;
Texture2D default_attack; // Kılıçsız saldırı animasyonu
Texture2D sword_idle;  // Kılıçlı idle animasyonu
Texture2D sword_walk;  // Kılıçlı yürüme animasyonu
Texture2D sword_run;   // Kılıçlı koşma animasyonu
Texture2D jumpside;
Texture2D player_attack_sword;
bool player_attack_sword_loaded = false;

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
    float attackAreaStartX; // Saldırı alanı başlangıç X
    float attackAreaStartY; // Saldırı alanı başlangıç Y
    bool isAttacking; // Saldırıyor mu?
    bool isAttackAnimPlaying; // Saldırı animasyonu oynuyor mu?
    int attackAnimFrame; // Saldırı animasyonu frame'i
    double attackAnimLastTime; // Son saldırı frame zamanı
    bool attackAnimTriggered; // Saldırı tetiklendi mi?
    bool hasDealtDamage; // Hasar verildi mi?
    float attackAnimStartX; // Saldırı başlangıç X
    float attackAnimTargetX; // Saldırı hedef X
    float attackAnimTimer; // Saldırı animasyonu zamanlayıcısı
};

Player player;

extern float energy;

bool isJumping = false;

double lastDamageTime = -1.0;

void UpdateEnergy(float deltaTime, bool isMoving, bool isRunning);

void InitPlayer(float startX, float startY) {
    // Texture'ları yükle
    default_idle = LoadTexture("assets/sprites/player/default/idle.png");
    default_walk = LoadTexture("assets/sprites/player/default/walk.png");
    default_run = LoadTexture("assets/sprites/player/default/run.png");
    default_attack = LoadTexture("assets/sprites/player/default/attack.png");
    jumpside = LoadTexture("assets/sprites/player/default/jumpside.png");
    sword_idle = LoadTexture("assets/sprites/player/rustedsword/idle.png");
    sword_walk = LoadTexture("assets/sprites/player/rustedsword/walk.png");
    sword_run = LoadTexture("assets/sprites/player/rustedsword/run.png");
    
    if (!player_attack_sword_loaded) {
        player_attack_sword = LoadTexture("assets/sprites/player/rustedsword/attack.png");
        player_attack_sword_loaded = true;
    }

    player.x = startX;
    player.y = startY;
    player.speed = 2.0f;
    player.frame = 0;
    player.frameTimer = 0.0f;
    player.moving = false;
    player.facingRight = true;
    player.hasSword = false;
    isJumping = false;
    player.velocityY = 0.0f;
    player.onGround = true;
    player.isAttacking = false;
    player.isAttackAnimPlaying = false;
    player.attackAnimFrame = 0;
    player.attackAnimLastTime = 0.0;
    player.attackAnimTriggered = false;
    player.hasDealtDamage = false;
    player.attackAnimStartX = 0.0f;
    player.attackAnimTargetX = 0.0f;
    player.attackAnimTimer = 0.0f;
}

void UnloadPlayer() {
    if (player_attack_sword_loaded) {
        UnloadTexture(player_attack_sword);
        player_attack_sword_loaded = false;
    }
}

void UpdatePlayer() {
    player.moving = false;
    bool running = false;
    float gravity = 0.5f; // Yerçekimini azalttım
    float jumpStrength = 20.0f; // Zıplama kuvveti
    float groundY = GetScreenHeight() - 256 * scalefactor; // Yerin y pozisyonu (ep1.cpp ile uyumlu)

    // Saldırı kontrolü
    if (IsKeyPressed(KEY_SPACE) && !player.isAttackAnimPlaying) {
        player.isAttackAnimPlaying = true;
        player.attackAnimTimer = 0.0f;
        player.attackAnimFrame = 0;
        player.attackAnimLastTime = GetTime();
        player.attackAnimTriggered = true;
        player.hasDealtDamage = false;
        player.attackAnimStartX = player.x;
        if (player.facingRight) {
            player.attackAnimTargetX = player.x + 50.0f;
        } else {
            player.attackAnimTargetX = player.x - 50.0f;
        }
    }

    // Saldırı animasyonu
    if (player.isAttackAnimPlaying) {
        player.isAttacking = true;
        double currentTime = GetTime();
        float frameProgress = (float)((currentTime - player.attackAnimLastTime) / 0.125); // 0.5/4 = 0.125
        float t = (player.attackAnimFrame + frameProgress) / 4.0f;
        if (t > 1.0f) t = 1.0f;
        player.x = player.attackAnimStartX + (player.attackAnimTargetX - player.attackAnimStartX) * t;
        player.moving = true;

        // Son frame'de ve animasyon bitmeden, sadece bir kez hasar ver
        if (player.attackAnimFrame == 3 && !player.hasDealtDamage) {
            float attackAreaW = 128 * scalefactor * 0.5f;
            float attackAreaH = 128 * scalefactor * 0.9f;
            float attackAreaX = player.facingRight ? player.x + 128 * scalefactor : player.x - attackAreaW;
            float attackAreaY = player.y;
            Rectangle attackAreaRect = {attackAreaX, attackAreaY, attackAreaW, attackAreaH};
            
            // Zombi hitbox'ı için extern değişken kullan
            extern Zombie zombie;
            Rectangle zombieHitboxRect = {zombie.x, zombie.y, 128 * scalefactor * 0.8f, 128 * scalefactor * 0.9f};
            
            if (CheckCollisionRecs(attackAreaRect, zombieHitboxRect)) {
                // Zombiye hasar ver
                float damage = player.hasSword ? 5.0f : 2.0f;
                zombie.health -= damage;
                if (zombie.health < 0) zombie.health = 0;
                player.hasDealtDamage = true;
                zombie.lastDamageTime = GetTime(); // Zombi hasar alma zamanını kaydet
            }
        }

        if (currentTime - player.attackAnimLastTime >= 0.125) { // 0.5/4 = 0.125
            player.attackAnimFrame++;
            player.attackAnimLastTime = currentTime;
        }

        if (player.attackAnimFrame >= 4) {
            player.isAttackAnimPlaying = false;
            player.isAttacking = false;
            player.attackAnimFrame = 0;
            player.attackAnimTriggered = false;
        }
        return;
    }

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
        
        // Yere çarpma kontrolü
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
        float jumpMoveSpeed = player.speed * 1.2f * GetFrameTime() * 60.0f; // Zıplarken hareket hızını azalttım
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
        float moveSpeed = player.speed * GetFrameTime() * 60.0f;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
            player.x += moveSpeed;
            player.moving = true;
            player.facingRight = true;
        }
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
            player.x -= moveSpeed;
            player.moving = true;
            player.facingRight = false;
        }
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
            running = true;
            player.speed = 4.0f;
        } else {
            player.speed = 2.0f;
        }
    } else {
        // Enerji yoksa idle'da kal
        player.moving = false;
        player.speed = 2.0f;
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
    
    // Saldırı animasyonu
    Texture2D* attackTex = &default_attack;
    if (player.hasSword && player_attack_sword_loaded) {
        attackTex = &player_attack_sword;
    }
    if (player.isAttacking) {
        int frameToDraw = player.attackAnimFrame;
        int frameCount = 4; // attack.png 4 frame
        int frameWidth = attackTex->width / frameCount;
        int frameHeight = attackTex->height;
        Rectangle src = {
            (float)(frameToDraw * frameWidth),
            0,
            (float)frameWidth,
            (float)frameHeight
        };
        if (!player.facingRight) src.width = -src.width;
        float drawX = player.x;
        float drawY = player.y;
        float scale = scalefactor * 2.0f;
        float drawW = frameWidth * scale;
        float drawH = frameHeight * scale;
        DrawTexturePro(*attackTex, src, (Rectangle){drawX, drawY, drawW, drawH}, (Vector2){0,0}, 0.0f, WHITE);
        return;
    }

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
        Color tint = WHITE;
        if (GetTime() - lastDamageTime < 0.3) tint = RED;
        DrawTexturePro(jumpside, src, (Rectangle){drawX, drawY, drawW, drawH}, (Vector2){0,0}, 0.0f, tint);
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
    Color tint = WHITE;
    if (GetTime() - lastDamageTime < 0.3) tint = RED;
    DrawTexturePro(*tex, src, (Rectangle){drawX, drawY, drawW, drawH}, (Vector2){0,0}, 0.0f, tint);
}


