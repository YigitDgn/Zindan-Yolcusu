#include <raylib.h>
#include <stdio.h>
#include <math.h>
#include "../../globals.h"
#include "enemy.h" // Zombie yapısı için
#include "inventory.h"
#include "debug.h" // Debug fonksiyonları için
#include "playermov.h"
#include "health_system.h"
#include "camera.h"
#include "ep1.h"

#ifndef ITEM_HEALTHELIXIR
#define ITEM_HEALTHELIXIR 3
#endif

// Debug değişkenleri
extern bool infiniteMode;

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

// EP1_ZOMBIE_COUNT sabitini tanımla
#define EP1_ZOMBIE_COUNT 49

// Zombies dizisini extern olarak tanımla
extern Zombie zombies[EP1_ZOMBIE_COUNT];

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
Texture2D ironsword_idle;
Texture2D ironsword_walk;
Texture2D ironsword_run;
Texture2D ironsword_attack;
bool ironsword_loaded = false;

// Player yapısının tanımını kaldırıyorum çünkü playermov.h'da tanımlı

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

    if (!ironsword_loaded) {
        ironsword_idle = LoadTexture("assets/sprites/player/ironsword/idle.png");
        ironsword_walk = LoadTexture("assets/sprites/player/ironsword/walk.png");
        ironsword_run = LoadTexture("assets/sprites/player/ironsword/run.png");
        ironsword_attack = LoadTexture("assets/sprites/player/ironsword/attack.png");
        ironsword_loaded = true;
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
    player.swordType = ITEM_RUSTEDSWORD;
}

void UnloadPlayer() {
    if (player_attack_sword_loaded) {
        UnloadTexture(player_attack_sword);
        player_attack_sword_loaded = false;
    }
    if (ironsword_loaded) {
        UnloadTexture(ironsword_idle);
        UnloadTexture(ironsword_walk);
        UnloadTexture(ironsword_run);
        UnloadTexture(ironsword_attack);
        ironsword_loaded = false;
    }
}

void UpdatePlayer() {
    // Debug konsolu açıksa oyun kontrollerini devre dışı bırak
    extern bool debugConsoleOpen;
    extern int selectedEquipSlot;
    extern Inventory playerInventory;
    extern bool isGameOver;  // Game over durumunu kontrol et
    
    // Game over durumunda kontrolleri devre dışı bırak
    if (isGameOver) return;
    
    player.swordType = playerInventory.equip[selectedEquipSlot].type;
    player.hasSword = (player.swordType == ITEM_RUSTEDSWORD || player.swordType == ITEM_IRONSWORD);
    if (debugConsoleOpen) return;

    player.moving = false;
    bool running = false;
    float gravity = 0.5f; // Yerçekimini azalttım
    float jumpStrength = 20.0f; // Zıplama kuvveti
    float groundOffset = 80 * scalefactor; // Zemini biraz yukarı al
    float groundY = GetScreenHeight() - groundOffset; // Yerin y pozisyonu

    // Oyuncu hitbox'ı için hesaplamalar
    float playerSpriteH = 128 * scalefactor * 2.0f;
    float playerHitboxH = playerSpriteH * 0.9f;
    float playerHitboxY = (playerSpriteH - playerHitboxH) / 2.0f;
    float playerGroundY = groundY - (playerHitboxY + playerHitboxH);

    // Saldırı kontrolü
    if (IsKeyPressed(KEY_SPACE) && !player.isAttackAnimPlaying) {
        printf("Saldırı başlatıldı!\n");
        player.isAttackAnimPlaying = true;
        player.isAttacking = true;
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
        running = false;
        player.speed = 2.0f;
    }

    // Eğer saldırı animasyonu oynuyorsa koşma tuşlarını dikkate alma
    if (player.isAttackAnimPlaying) {
        running = false;
        player.speed = 2.0f;
    }

    // Zıplama başlat
    if ((IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && player.onGround) {
        isJumping = true;
        player.frame = 0;
        player.frameTimer = 0.0f;
        player.velocityY = -jumpStrength;
        player.onGround = false;
        if (!infiniteMode) energy -= 15;
    }

    // Zıplama sırasında fizik
    if (!player.onGround) {
        // Yer çekimini uygula
        player.velocityY += gravity;
        
        // Yeni pozisyonu hesapla
        float newY = player.y + player.velocityY;
        
        // Yere çarpma kontrolü
        if (newY >= playerGroundY) {
            player.y = playerGroundY;
            player.velocityY = 0;
            player.onGround = true;
            isJumping = false;
        } else {
            player.y = newY;
        }
    } else {
        // Yerdeyken pozisyonu sabitle
        player.y = playerGroundY;
        player.velocityY = 0;
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
    extern Texture2D d3_texture;
    extern Texture2D d4_texture;
    float mapWidth = (d3_texture.width + 49 * d4_texture.width) * scalefactor;
    float playerMaxX = mapWidth - 128 * scalefactor * 2.0f;

    if (energy > 0.0f || infiniteMode) {
        float moveSpeed = player.speed * GetFrameTime() * 60.0f;
        bool moved = false;
        if ((IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) && player.x < playerMaxX) {
            player.x += moveSpeed;
            player.moving = true;
            player.facingRight = true;
            moved = true;
        }
        if ((IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) && player.x > 0) {
            player.x -= moveSpeed;
            player.moving = true;
            player.facingRight = false;
            moved = true;
        }
        if (!moved) {
            player.moving = false;
        }
        if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) && !infiniteMode) {
            running = true;
            player.speed = 4.0f;
        } else if (!infiniteMode) {
            player.speed = 2.0f;
        }
    } else {
        // Enerji yoksa idle'da kal
        player.moving = false;
        if (!infiniteMode) player.speed = 2.0f;
    }

    // Sınırları tekrar uygula (taşma olmasın)
    if (player.x < 0) player.x = 0;
    if (player.x > playerMaxX) player.x = playerMaxX;

    // Animasyon frame güncelle
    int maxFrames = player.moving ? (running ? PLAYER_RUN_FRAMES : PLAYER_WALK_FRAMES) : PLAYER_IDLE_FRAMES;
    player.frameTimer += GetFrameTime() * PLAYER_FRAME_SPEED;
    if (player.frameTimer >= 1.0f) {
        player.frame = (player.frame + 1) % maxFrames;
        player.frameTimer = 0.0f;
    }

    // Enerji güncelle (hareket halindeyken yenilenme olmasın)
    UpdateEnergy(GetFrameTime(), player.moving, running);

    // Eliksir kullanımı (F tuşu)
    if (IsKeyPressed(KEY_F) && !player.isHealing) {
        extern Inventory playerInventory;
        extern int selectedEquipSlot;
        // Seçili slotta sağlık iksiri varsa kullan
        if (playerInventory.equip[selectedEquipSlot].type == ITEM_HEALTHELIXIR) {
            playerInventory.equip[selectedEquipSlot].type = ITEM_NONE;
            player.isHealing = true;
            player.healStartTime = GetTime();
            player.healStartHealth = playerHealth;
            player.healTargetHealth = 100.0f;
        }
    }
    // Eliksir ile canı yavaş yavaş doldurma
    if (player.isHealing) {
        double elapsed = GetTime() - player.healStartTime;
        float healAmount = (player.healTargetHealth - player.healStartHealth) * (elapsed / 2.0f); // 2 saniyede dolsun
        playerHealth = player.healStartHealth + healAmount;
        if (playerHealth >= player.healTargetHealth) {
            playerHealth = player.healTargetHealth;
            player.isHealing = false;
        }
    }
}

void DrawPlayer(float scalefactor) {
    bool running = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) && player.moving;
    // Saldırı animasyonu
    if (player.isAttacking) {
        int frameCount = 4; // attack.png 4 frame
        int frameToDraw = player.attackAnimFrame;
        Texture2D* attackTex = &default_attack;
        if (player.hasSword) {
            if (player.swordType == ITEM_IRONSWORD && ironsword_loaded) {
                attackTex = &ironsword_attack;
            } else if (player.swordType == ITEM_RUSTEDSWORD && player_attack_sword_loaded) {
                attackTex = &player_attack_sword;
            }
        }
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

        // Saldırı alanı hesaplama
        float playerHitboxW = drawW * 0.6f;
        float playerHitboxH = drawH * 0.9f;
        float playerHitboxX = drawX + (drawW - playerHitboxW) / 2;
        float playerHitboxY = drawY + (drawH - playerHitboxH) / 2;
        float attackAreaW = playerHitboxW * 0.5f;
        float attackAreaH = playerHitboxH;
        float attackAreaX, attackAreaY;
        if (player.facingRight) {
            attackAreaX = playerHitboxX + playerHitboxW;
        } else {
            attackAreaX = playerHitboxX - attackAreaW;
        }
        attackAreaY = playerHitboxY;
        Rectangle attackArea = {attackAreaX, attackAreaY, attackAreaW, attackAreaH};

        // Saldırı alanı ile zombi hitbox'ı çakışma kontrolü
        if (player.attackAnimFrame == 3 && !player.hasDealtDamage) {
            for (int i = 0; i < EP1_ZOMBIE_COUNT; i++) {
                float zombieSpriteW = 128 * scalefactor * 1.8f;
                float zombieSpriteH = 128 * scalefactor * 1.8f;
                float zombieHitboxW = zombieSpriteW * 0.8f;
                float zombieHitboxH = zombieSpriteH * 0.9f;
                float zombieHitboxX = zombies[i].x + (zombieSpriteW - zombieHitboxW) / 2;
                float zombieHitboxY = zombies[i].y + (zombieSpriteH - zombieHitboxH) / 2;
                Rectangle zombieHitbox = {zombieHitboxX, zombieHitboxY, zombieHitboxW, zombieHitboxH};

                if (CheckCollisionRecs(attackArea, zombieHitbox)) {
                    float damage = 2.0f; // Temel hasar (kılıçsız)
                    if (player.hasSword) {
                        if (player.swordType == ITEM_IRONSWORD) {
                            damage = 10.0f;
                        } else if (player.swordType == ITEM_RUSTEDSWORD) {
                            damage = 5.0f;
                        }
                    }
                    zombies[i].health -= damage;
                    if (zombies[i].health < 0) zombies[i].health = 0;
                    zombies[i].lastDamageTime = GetTime();
                    player.hasDealtDamage = true;
                    printf("Zombiye %f hasar verildi! Kalan can: %f\n", damage, zombies[i].health);
                    break; // Aynı anda sadece bir zombiye hasar verilsin
                }
            }
        }

        static double lastAnimUpdate = 0.0;
        double currentTime = GetTime();
        if (currentTime - lastAnimUpdate >= 0.1875) {
            player.attackAnimFrame++;
            lastAnimUpdate = currentTime;
        }
        if (player.attackAnimFrame >= 4) {
            printf("Saldırı animasyonu bitti.\n");
            player.isAttackAnimPlaying = false;
            player.isAttacking = false;
            player.attackAnimFrame = 0;
            player.attackAnimTriggered = false;
        }
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
        if (player.swordType == ITEM_IRONSWORD && ironsword_loaded) {
            if (player.moving) {
                if (running) {
                    tex = &ironsword_run;
                } else {
                    tex = &ironsword_walk;
                }
            } else {
                tex = &ironsword_idle;
            }
        } else if (player.swordType == ITEM_RUSTEDSWORD && rustedSwordLoaded) {
            if (player.moving) {
                if (running) {
                    tex = &sword_run;
                } else {
                    tex = &sword_walk;
                }
            } else {
                tex = &sword_idle;
            }
        } else {
            tex = &default_idle;
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


