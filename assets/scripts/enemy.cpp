#include <raylib.h>
#include <stdio.h>
#include <math.h>
#include "../../globals.h"
#include "enemy.h"
#include "playermov.h"
#include "health_system.h"
#include "inventory.h"

extern float playerHealth;

// Zombi sprite sheet bilgileri
#define ZOMBIE_IDLE_FRAMES 4
#define ZOMBIE_WALK_FRAMES 4
#define ZOMBIE_ATTACK_FRAMES 4
#define ZOMBIE_FRAME_W 128
#define ZOMBIE_FRAME_H 128
#define ZOMBIE_FRAME_SPEED 2

// Zombi texture'ları
Texture2D zombie_idle;
Texture2D zombie_walk;
Texture2D zombie_attack;

// Zombi yapısı
Zombie zombie;

// Yere düşen item değişkeni
DroppedItem droppedItem = {0};

void InitZombie(float startX, float startY) {
    // Texture'ları yükle
    zombie_idle = LoadTexture("assets/sprites/enemy/zombie/zombieidle.png");
    zombie_walk = LoadTexture("assets/sprites/enemy/zombie/zombiewalk.png");
    zombie_attack = LoadTexture("assets/sprites/enemy/zombie/zombieattack.png");

    zombie.x = startX;
    zombie.y = startY;
    zombie.speed = 1.2f;
    zombie.frame = 0;
    zombie.frameTimer = 0.0f;
    zombie.moving = false;
    zombie.facingRight = true;
    zombie.isAttacking = false;
    zombie.attackCooldown = 0.0f;
    zombie.active = false; // Başlangıçta pasif
    zombie.attackAnimTimer = 0.0f;
    zombie.attackAnimFrame = 0;
    zombie.isAttackAnimPlaying = false;
    zombie.attackAnimLastTime = GetTime();
    zombie.attackAnimTriggered = false;
    zombie.hasDealtDamage = false;
    zombie.isDead = false; // Başlangıçta ölü değil
    zombie.hasDroppedItem = false; // Başlangıçta item düşürmedi
    zombie.willDropRustedSword = (zombie.level == 1); // Level 1 zombiler %100 rusted sword düşürür

    // Level sistemi başlatma
    zombie.level = 1;
    zombie.maxHealth = 20.0f;
    zombie.health = zombie.maxHealth;
    zombie.baseDamage = 10.0f;
    zombie.expValue = 10.0f;

    // Level bazlı özellikleri güncelle
    UpdateZombieStats();
}

// Level bazlı özellikleri güncelleme fonksiyonu
void UpdateZombieStats() {
    // Level sınırı kontrolü
    if (zombie.level > 5) zombie.level = 5;
    
    // Level bazlı can artışı (her level %50 daha fazla can)
    zombie.maxHealth = 20.0f * (1.0f + (zombie.level - 1) * 0.5f);
    zombie.health = zombie.maxHealth;
    
    // Level bazlı hasar artışı (her level %20 daha fazla hasar)
    zombie.baseDamage = 10.0f * (1.0f + (zombie.level - 1) * 0.2f);
    
    // Level bazlı deneyim değeri artışı (her level %30 daha fazla deneyim)
    zombie.expValue = 10.0f * (1.0f + (zombie.level - 1) * 0.3f);
}

void UpdateZombie() {
    // Zombi ölüyse güncelleme yapma
    if (zombie.isDead) return;

    // Can kontrolü
    if (zombie.health <= 0) {
        zombie.health = 0;
        zombie.isDead = true;
        zombie.active = false;
        zombie.isAttacking = false;
        zombie.isAttackAnimPlaying = false;
        
        // Item düşürme kontrolü
        if (!zombie.hasDroppedItem) {
            DropItem();
            zombie.hasDroppedItem = true;
        }
        return;
    }

    // Oyuncu hitbox'ı
    float playerW = 128 * scalefactor * 0.6f * 2.0f;
    float playerH = 128 * scalefactor * 0.9f * 2.0f;
    float playerX = player.x + (128 * scalefactor * 2.0f - playerW) / 2;
    float playerY = player.y + (128 * scalefactor * 2.0f - playerH) / 2;
    Rectangle playerHitbox = {playerX, playerY, playerW, playerH};

    // Zombi hitbox'ı
    float zombieW = 128 * scalefactor * 0.8f * 1.8f;
    float zombieH = 128 * scalefactor * 0.9f * 1.8f;
    float zombieX = zombie.x + (128 * scalefactor * 1.8f - zombieW) / 2;
    float zombieY = zombie.y + (128 * scalefactor * 1.8f - zombieH) / 2;

    // Attack area
    float attackAreaW = zombieW * 0.5f;
    float attackAreaH = zombieH;
    float attackAreaX, attackAreaY;
    if (zombie.facingRight) {
        attackAreaX = zombieX + zombieW;
    } else {
        attackAreaX = zombieX - attackAreaW;
    }
    attackAreaY = zombieY;
    Rectangle attackArea = {attackAreaX, attackAreaY, attackAreaW, attackAreaH};

    // Attack area çakışma kontrolü (sadece bir kere tetiklenir)
    if (!zombie.attackAnimTriggered && CheckCollisionRecs(playerHitbox, attackArea)) {
        zombie.isAttackAnimPlaying = true;
        zombie.attackAnimTimer = 0.0f;
        zombie.attackAnimFrame = 0;
        zombie.attackAnimLastTime = GetTime();
        zombie.attackAnimTriggered = true;
        float attackAreaWorldX, attackAreaWorldY;
        if (zombie.facingRight) {
            attackAreaWorldX = zombieX + zombieW;
        } else {
            attackAreaWorldX = zombieX - attackAreaW;
        }
        attackAreaWorldY = zombieY;
        zombie.attackAreaStartX = attackAreaWorldX;
        zombie.attackAreaStartY = attackAreaWorldY;
        float zombieSpriteW = 128 * scalefactor * 1.8f;
        zombie.attackAnimStartX = zombie.x;
        if (zombie.facingRight) {
            zombie.attackAnimTargetX = attackAreaWorldX + attackAreaW - zombieSpriteW;
        } else {
            zombie.attackAnimTargetX = attackAreaWorldX;
        }
        zombie.hasDealtDamage = false;
    }

    if (zombie.isAttackAnimPlaying) {
        zombie.isAttacking = true;
        double currentTime = GetTime();
        float frameProgress = (float)((currentTime - zombie.attackAnimLastTime) / 0.4375);
        float t = (zombie.attackAnimFrame + frameProgress) / 4.0f;
        if (t > 1.0f) t = 1.0f;
        zombie.x = zombie.attackAnimStartX + (zombie.attackAnimTargetX - zombie.attackAnimStartX) * t;
        zombie.moving = true;
        // Son frame'de ve animasyon bitmeden, sadece bir kez can azalt
        if (zombie.attackAnimFrame == 3 && !zombie.hasDealtDamage) {
            Rectangle attackAreaRect = {zombie.attackAreaStartX, zombie.attackAreaStartY, attackAreaW, attackAreaH};
            Rectangle zombieHitboxRect = {zombieX, zombieY, zombieW, zombieH};
            if (CheckCollisionRecs(playerHitbox, attackAreaRect) || CheckCollisionRecs(playerHitbox, zombieHitboxRect)) {
                // Level bazlı hasar hesaplama
                float damage = zombie.baseDamage * (1.0f + (zombie.level - 1) * 0.2f); // Her level %20 daha fazla hasar
                playerHealth -= damage;
                if (playerHealth < 0) playerHealth = 0;
                lastDamageTime = GetTime();
                zombie.hasDealtDamage = true;
            }
        }
        if (currentTime - zombie.attackAnimLastTime >= 0.4375) {
            zombie.attackAnimFrame++;
            zombie.attackAnimLastTime = currentTime;
        }
        if (zombie.attackAnimFrame >= 4) {
            zombie.isAttackAnimPlaying = false;
            zombie.isAttacking = false;
            zombie.attackAnimFrame = 0;
            zombie.attackAnimTriggered = false;
        }
        return;
    }

    // Animasyon bittiyse normal takip mantığı
    // Zombi aktifleşme alanı
    float activationW = zombieW * 10.0f;
    float activationH = zombieH;
    float activationX = zombieX + (zombieW - activationW) / 2;
    float activationY = zombieY;
    Rectangle activationArea = {activationX, activationY, activationW, activationH};

    if (!zombie.active && CheckCollisionRecs(playerHitbox, activationArea)) {
        zombie.active = true;
    }
    if (zombie.active) {
        if (player.x > zombie.x) {
            zombie.x += zombie.speed;
            zombie.facingRight = true;
        } else if (player.x < zombie.x) {
            zombie.x -= zombie.speed;
            zombie.facingRight = false;
        }
        zombie.moving = true;
    } else {
        zombie.moving = false;
    }

    // Animasyon frame güncelle
    int maxFrames = zombie.isAttacking ? ZOMBIE_ATTACK_FRAMES : ZOMBIE_IDLE_FRAMES;
    
    zombie.frameTimer += GetFrameTime() * ZOMBIE_FRAME_SPEED;
    if (zombie.frameTimer >= 1.0f) {
        zombie.frame = (zombie.frame + 1) % maxFrames;
        zombie.frameTimer = 0.0f;
    }

    // Saldırı cooldown güncelleme
    if (zombie.attackCooldown > 0) {
        zombie.attackCooldown -= GetFrameTime();
    }
}

void DrawZombie(float scalefactor) {
    // Zombi ölüyse çizme
    if (zombie.isDead) return;

    // Zombi durumuna göre texture seç
    Texture2D* tex;
    int frameToDraw = 0;
    if (zombie.isAttacking) {
        tex = &zombie_attack;
        frameToDraw = zombie.attackAnimFrame;
    } else if (zombie.moving) {
        tex = &zombie_walk;
        frameToDraw = zombie.frame;
    } else {
        tex = &zombie_idle;
        frameToDraw = zombie.frame;
    }

    // Frame hesaplama
    int frameCount = (zombie.isAttacking) ? ZOMBIE_ATTACK_FRAMES :
                     (zombie.moving ? ZOMBIE_WALK_FRAMES : ZOMBIE_IDLE_FRAMES);
    int frameWidth = tex->width / frameCount;
    int frameHeight = tex->height;

    Rectangle src = {
        (float)(frameToDraw * frameWidth),
        0,
        (float)frameWidth,
        (float)frameHeight
    };

    if (!zombie.facingRight) src.width = -src.width;

    float drawX = zombie.x;
    float drawY = zombie.y;
    float scale = scalefactor * 1.8f;
    float drawW = frameWidth * scale;
    float drawH = frameHeight * scale;

    Color tint = WHITE;
    if (GetTime() - zombie.lastDamageTime < 0.3) tint = RED; // Hasar alındığında kırmızı ol

    DrawTexturePro(*tex, src, (Rectangle){drawX, drawY, drawW, drawH}, (Vector2){0,0}, 0.0f, tint);

    // Can barı çiz
    float healthBarWidth = drawW * 0.8f; // Zombi genişliğinin %80'i
    float healthBarHeight = 10.0f * scalefactor;
    float healthBarX = drawX + (drawW - healthBarWidth) / 2;
    float healthBarY = drawY - healthBarHeight - 5.0f * scalefactor;

    // Can barı arka planı (kırmızı)
    DrawRectangle(healthBarX, healthBarY, healthBarWidth, healthBarHeight, RED);
    
    // Mevcut can (yeşil)
    float currentHealthWidth = healthBarWidth * (zombie.health / zombie.maxHealth);
    DrawRectangle(healthBarX, healthBarY, currentHealthWidth, healthBarHeight, GREEN);
    
    // Can barı çerçevesi (beyaz)
    DrawRectangleLinesEx(
        (Rectangle){healthBarX, healthBarY, healthBarWidth, healthBarHeight},
        2,
        WHITE
    );

    // Level göstergesi
    char levelText[32];
    sprintf(levelText, "Lv.%d", zombie.level);
    int levelTextWidth = MeasureText(levelText, 20);
    DrawText(levelText, healthBarX + (healthBarWidth - levelTextWidth) / 2, healthBarY - 25, 20, YELLOW);
}

// Item düşürme fonksiyonu
void DropItem() {
    if (zombie.willDropRustedSword) {
        // Rusted sword'u yere düşür
        droppedItem.x = zombie.x + 32; // Zombinin ortasına yakın bir yere
        droppedItem.y = zombie.y + 64;
        droppedItem.active = true;
        droppedItem.type = 1; // 1: rusted sword
    }
}

void UpdateDroppedItem() {
    if (!droppedItem.active) return;
    // Oyuncu itemin yakınında mı?
    float playerCenterX = player.x + 128 * scalefactor;
    float playerCenterY = player.y + 128 * scalefactor;
    float itemCenterX = droppedItem.x + 16;
    float itemCenterY = droppedItem.y + 16;
    float dx = playerCenterX - itemCenterX;
    float dy = playerCenterY - itemCenterY;
    float distance = sqrtf(dx*dx + dy*dy);
    if (distance < 64) { // Yakınlık mesafesi
        if (IsKeyPressed(KEY_E)) {
            // Envanterde ilk boş slota ekle
            for (int i = 0; i < INV_BAG_SIZE; i++) {
                if (playerInventory.bag[i].type == ITEM_NONE) {
                    playerInventory.bag[i].type = ITEM_RUSTEDSWORD;
                    droppedItem.active = false;
                    droppedItem.type = 0;
                    break;
                }
            }
        }
    }
}

void DrawDroppedItem(float scalefactor) {
    if (!droppedItem.active) return;
    if (droppedItem.type == 1 && rustedSwordLoaded) {
        DrawTextureEx(rustedSwordTexture, (Vector2){droppedItem.x, droppedItem.y}, 0, 1.0f * scalefactor, WHITE);
        // "E ile al" yazısı
        float playerCenterX = player.x + 128 * scalefactor;
        float playerCenterY = player.y + 128 * scalefactor;
        float itemCenterX = droppedItem.x + 16;
        float itemCenterY = droppedItem.y + 16;
        float dx = playerCenterX - itemCenterX;
        float dy = playerCenterY - itemCenterY;
        float distance = sqrtf(dx*dx + dy*dy);
        if (distance < 64) {
            const char* text = "E ile al";
            int textWidth = MeasureText(text, 24);
            DrawText(text, droppedItem.x + 16 - textWidth/2, droppedItem.y - 28, 24, YELLOW);
        }
    }
}
