#include <raylib.h>
#include <stdio.h>
#include <math.h>
#include "../../globals.h"
#include "enemy.h"
#include "playermov.h"
#include "health_system.h"
#include "inventory.h"

extern float playerHealth;

#define ITEM_HEALTHELIXIR 3
#define MAX_DROPPED_ITEMS 20

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
// Zombie zombie; // KALDIRILDI

// Yere düşen item değişkeni
DroppedItem droppedItems[MAX_DROPPED_ITEMS] = {0};

// Eliksir görseli için global değişkenler
extern Texture2D healthelixirTexture;
extern bool healthelixirLoaded;

// Demir kılıç görseli için global değişkenler
extern Texture2D ironswordTexture;
extern bool ironswordLoaded;

void InitZombie(Zombie* zombie, float startX, float startY) {
    // Texture'ları yükle (sadece ilk zombi için yüklenmeli, kontrol eklenecek)
    static bool texturesLoaded = false;
    if (!texturesLoaded) {
        zombie_idle = LoadTexture("assets/sprites/enemy/zombie/zombieidle.png");
        zombie_walk = LoadTexture("assets/sprites/enemy/zombie/zombiewalk.png");
        zombie_attack = LoadTexture("assets/sprites/enemy/zombie/zombieattack.png");
        texturesLoaded = true;
    }
    zombie->x = startX;
    zombie->y = startY;
    zombie->speed = 1.2f;
    zombie->frame = 0;
    zombie->frameTimer = 0.0f;
    zombie->moving = false;
    zombie->facingRight = true;
    zombie->isAttacking = false;
    zombie->attackCooldown = 0.0f;
    zombie->active = false;
    zombie->attackAnimTimer = 0.0f;
    zombie->attackAnimFrame = 0;
    zombie->isAttackAnimPlaying = false;
    zombie->attackAnimLastTime = GetTime();
    zombie->attackAnimTriggered = false;
    zombie->hasDealtDamage = false;
    zombie->isDead = false;
    zombie->hasDroppedItem = false;
    zombie->level = 1;
    zombie->maxHealth = 20.0f;
    zombie->health = zombie->maxHealth;
    zombie->baseDamage = 10.0f;
    zombie->expValue = 10.0f;
    UpdateZombieStats(zombie);
}

void UpdateZombieStats(Zombie* zombie) {
    if (zombie->level > 5) zombie->level = 5;
    zombie->maxHealth = 20.0f * (1.0f + (zombie->level - 1) * 0.5f);
    zombie->health = zombie->maxHealth;
    
    // Level bazlı hasar ayarları
    switch(zombie->level) {
        case 1: zombie->baseDamage = 10.0f; break;
        case 2: zombie->baseDamage = 15.0f; break;
        case 3: zombie->baseDamage = 20.0f; break;
        case 4: zombie->baseDamage = 30.0f; break;
        case 5: zombie->baseDamage = 40.0f; break;
        default: zombie->baseDamage = 10.0f;
    }
    
    zombie->expValue = 10.0f * (1.0f + (zombie->level - 1) * 0.3f);
}

void UpdateZombie(Zombie* zombie) {
    if (zombie->isDead) return;

    // Can kontrolü
    if (zombie->health <= 0) {
        zombie->health = 0;
        zombie->isDead = true;
        zombie->active = false;
        zombie->isAttacking = false;
        zombie->isAttackAnimPlaying = false;
        
        // Item düşürme kontrolü
        if (!zombie->hasDroppedItem) {
            DropItem(zombie);
            zombie->hasDroppedItem = true;
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
    float zombieX = zombie->x + (128 * scalefactor * 1.8f - zombieW) / 2;
    float zombieY = zombie->y + (128 * scalefactor * 1.8f - zombieH) / 2;

    // Attack area - Zombinin ön yarısı + önündeki alan
    float attackAreaW = zombieW * 0.5f + zombieW * 0.5f; // Yani toplamda 1.0x genişlik
    float attackAreaH = zombieH;
    float attackAreaX, attackAreaY;
    if (zombie->facingRight) {
        // Saldırı alanı: zombi hitbox'ının ön yarısı + önündeki alan
        attackAreaX = zombieX + zombieW / 2.0f;
    } else {
        // Saldırı alanı: zombi hitbox'ının ön yarısı + önündeki alan
        attackAreaX = zombieX - zombieW * 0.5f;
    }
    attackAreaY = zombieY;
    Rectangle attackArea = {attackAreaX, attackAreaY, attackAreaW, attackAreaH};

    // Attack area çakışma kontrolü (sadece bir kere tetiklenir)
    if (!zombie->attackAnimTriggered && CheckCollisionRecs(playerHitbox, attackArea)) {
        zombie->isAttackAnimPlaying = true;
        zombie->attackAnimTimer = 0.0f;
        zombie->attackAnimFrame = 0;
        zombie->attackAnimLastTime = GetTime();
        zombie->attackAnimTriggered = true;
        float attackAreaWorldX, attackAreaWorldY;
        if (zombie->facingRight) {
            attackAreaWorldX = zombieX + zombieW / 2.0f;
        } else {
            attackAreaWorldX = zombieX - zombieW * 0.5f;
        }
        attackAreaWorldY = zombieY;
        zombie->attackAreaStartX = attackAreaWorldX;
        zombie->attackAreaStartY = attackAreaWorldY;
        float zombieSpriteW = 128 * scalefactor * 1.8f;
        zombie->attackAnimStartX = zombie->x;
        if (zombie->facingRight) {
            zombie->attackAnimTargetX = attackAreaWorldX + attackAreaW - zombieSpriteW;
        } else {
            zombie->attackAnimTargetX = attackAreaWorldX;
        }
        zombie->hasDealtDamage = false;
    }

    if (zombie->isAttackAnimPlaying) {
        zombie->isAttacking = true;
        double currentTime = GetTime();
        float frameProgress = (float)((currentTime - zombie->attackAnimLastTime) / 0.4375);
        float t = (zombie->attackAnimFrame + frameProgress) / 4.0f;
        if (t > 1.0f) t = 1.0f;
        zombie->x = zombie->attackAnimStartX + (zombie->attackAnimTargetX - zombie->attackAnimStartX) * t;
        zombie->moving = true;
        // Son frame'de ve animasyon bitmeden, sadece bir kez can azalt
        if (zombie->attackAnimFrame == 3 && !zombie->hasDealtDamage) {
            Rectangle attackAreaRect = {zombie->attackAreaStartX, zombie->attackAreaStartY, attackAreaW, attackAreaH};
            // Zombi hitbox'ı ile çarpışma kontrolünü kaldır, sadece saldırı alanı ile kontrol et
            if (CheckCollisionRecs(playerHitbox, attackAreaRect)) {
                // Level bazlı hasar hesaplama
                float damage = zombie->baseDamage; // Direkt baseDamage'ı kullan
                playerHealth -= damage;
                if (playerHealth < 0) playerHealth = 0;
                lastDamageTime = GetTime();
                zombie->hasDealtDamage = true;
            }
        }
        if (currentTime - zombie->attackAnimLastTime >= 0.4375) {
            zombie->attackAnimFrame++;
            zombie->attackAnimLastTime = currentTime;
        }
        if (zombie->attackAnimFrame >= 4) {
            zombie->isAttackAnimPlaying = false;
            zombie->isAttacking = false;
            zombie->attackAnimFrame = 0;
            zombie->attackAnimTriggered = false;
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

    if (!zombie->active && CheckCollisionRecs(playerHitbox, activationArea)) {
        zombie->active = true;
    }
    if (zombie->active) {
        if (player.x > zombie->x) {
            zombie->x += zombie->speed;
            zombie->facingRight = true;
        } else if (player.x < zombie->x) {
            zombie->x -= zombie->speed;
            zombie->facingRight = false;
        }
        zombie->moving = true;
    } else {
        zombie->moving = false;
    }

    // Animasyon frame güncelle
    int maxFrames = zombie->isAttacking ? ZOMBIE_ATTACK_FRAMES : ZOMBIE_IDLE_FRAMES;
    
    zombie->frameTimer += GetFrameTime() * ZOMBIE_FRAME_SPEED;
    if (zombie->frameTimer >= 1.0f) {
        zombie->frame = (zombie->frame + 1) % maxFrames;
        zombie->frameTimer = 0.0f;
    }

    // Saldırı cooldown güncelleme
    if (zombie->attackCooldown > 0) {
        zombie->attackCooldown -= GetFrameTime();
    }
}

void DrawZombie(Zombie* zombie, float scalefactor) {
    if (zombie->isDead) return;

    // Zombi durumuna göre texture seç
    Texture2D* tex;
    int frameToDraw = 0;
    if (zombie->isAttacking) {
        tex = &zombie_attack;
        frameToDraw = zombie->attackAnimFrame;
    } else if (zombie->moving) {
        tex = &zombie_walk;
        frameToDraw = zombie->frame;
    } else {
        tex = &zombie_idle;
        frameToDraw = zombie->frame;
    }

    // Frame hesaplama
    int frameCount = (zombie->isAttacking) ? ZOMBIE_ATTACK_FRAMES :
                     (zombie->moving ? ZOMBIE_WALK_FRAMES : ZOMBIE_IDLE_FRAMES);
    int frameWidth = tex->width / frameCount;
    int frameHeight = tex->height;

    Rectangle src = {
        (float)(frameToDraw * frameWidth),
        0,
        (float)frameWidth,
        (float)frameHeight
    };

    if (!zombie->facingRight) src.width = -src.width;

    float drawX = zombie->x;
    float drawY = zombie->y;
    float scale = scalefactor * 1.8f;
    float drawW = frameWidth * scale;
    float drawH = frameHeight * scale;

    Color tint = WHITE;
    if (GetTime() - zombie->lastDamageTime < 0.3) tint = RED; // Hasar alındığında kırmızı ol

    DrawTexturePro(*tex, src, (Rectangle){drawX, drawY, drawW, drawH}, (Vector2){0,0}, 0.0f, tint);

    // Can barı çiz
    float healthBarWidth = drawW * 0.8f; // Zombi genişliğinin %80'i
    float healthBarHeight = 10.0f * scalefactor;
    float healthBarX = drawX + (drawW - healthBarWidth) / 2;
    float healthBarY = drawY - healthBarHeight - 5.0f * scalefactor;

    // Can barı arka planı (kırmızı)
    DrawRectangle(healthBarX, healthBarY, healthBarWidth, healthBarHeight, RED);
    
    // Mevcut can (yeşil)
    float currentHealthWidth = healthBarWidth * (zombie->health / zombie->maxHealth);
    DrawRectangle(healthBarX, healthBarY, currentHealthWidth, healthBarHeight, GREEN);
    
    // Can barı çerçevesi (beyaz)
    DrawRectangleLinesEx(
        (Rectangle){healthBarX, healthBarY, healthBarWidth, healthBarHeight},
        2,
        WHITE
    );

    // Level göstergesi
    char levelText[32];
    sprintf(levelText, "Lv.%d", zombie->level);
    int levelTextWidth = MeasureText(levelText, 20);
    DrawText(levelText, healthBarX + (healthBarWidth - levelTextWidth) / 2, healthBarY - 25, 20, YELLOW);
}

// Item düşürme fonksiyonu
void DropItem(Zombie* zombie) {
    if (!zombie->hasDroppedItem) {
        // Her zombi %10 ihtimalle eliksir dusurur
        int elixirChance = GetRandomValue(0, 9);
        if (elixirChance == 0) { // %10 ihtimal
            for (int i = 0; i < MAX_DROPPED_ITEMS; i++) {
                if (!droppedItems[i].active) {
                    droppedItems[i].x = zombie->x + 32;
                    droppedItems[i].y = zombie->y + 64;
                    droppedItems[i].active = true;
                    droppedItems[i].type = ITEM_HEALTHELIXIR;
                    break;
                }
            }
        }

        // Level 1, 2 ve 3 zombiler %25 ihtimalle pasli kilic dusurur
        if (zombie->level >= 1 && zombie->level <= 3) {
            int swordChance = GetRandomValue(0, 3);
            if (swordChance == 0) { // %25 ihtimal
                for (int i = 0; i < MAX_DROPPED_ITEMS; i++) {
                    if (!droppedItems[i].active) {
                        droppedItems[i].x = zombie->x + 32;
                        droppedItems[i].y = zombie->y + 64;
                        droppedItems[i].active = true;
                        droppedItems[i].type = ITEM_RUSTEDSWORD;
                        break;
                    }
                }
            }
        }

        // Level 3, 4 ve 5 zombiler demir kilic dusurur
        if (zombie->level >= 3) {
            int ironSwordChance;
            if (zombie->level == 3) {
                ironSwordChance = GetRandomValue(0, 9); // %10 ihtimal
            } else {
                ironSwordChance = GetRandomValue(0, 4); // %20 ihtimal
            }
            
            if (ironSwordChance == 0) {
                for (int i = 0; i < MAX_DROPPED_ITEMS; i++) {
                    if (!droppedItems[i].active) {
                        droppedItems[i].x = zombie->x + 32;
                        droppedItems[i].y = zombie->y + 64;
                        droppedItems[i].active = true;
                        droppedItems[i].type = ITEM_IRONSWORD;
                        break;
                    }
                }
            }
        }
    }
}

void UpdateDroppedItem() {
    for (int j = 0; j < MAX_DROPPED_ITEMS; j++) {
        if (!droppedItems[j].active) continue;
        // Oyuncu itemin yakınında mı?
        float playerCenterX = player.x + 128 * scalefactor;
        float playerCenterY = player.y + 128 * scalefactor;
        float itemCenterX = droppedItems[j].x + 16;
        float itemCenterY = droppedItems[j].y + 16;
        float dx = playerCenterX - itemCenterX;
        float dy = playerCenterY - itemCenterY;
        float distance = sqrtf(dx*dx + dy*dy);
        
        if (distance < 64) {
            if (IsKeyPressed(KEY_E)) {
                bool itemAlindi = false;
                for (int i = 0; i < INV_BAG_SIZE; i++) {
                    if (playerInventory.bag[i].type == ITEM_NONE) {
                        playerInventory.bag[i].type = (ItemType)droppedItems[j].type;
                        droppedItems[j].active = false;
                        droppedItems[j].type = 0;
                        itemAlindi = true;
                        break;
                    }
                }
            }
        }
    }
}

void DrawDroppedItem(float scalefactor) {
    for (int j = 0; j < MAX_DROPPED_ITEMS; j++) {
        if (!droppedItems[j].active) continue;
        if (droppedItems[j].type == ITEM_RUSTEDSWORD && rustedSwordLoaded) {
            DrawTextureEx(rustedSwordTexture, (Vector2){droppedItems[j].x, droppedItems[j].y}, 0, 1.0f * scalefactor, WHITE);
        }
        if (droppedItems[j].type == ITEM_HEALTHELIXIR && healthelixirLoaded) {
            DrawTextureEx(healthelixirTexture, (Vector2){droppedItems[j].x, droppedItems[j].y}, 0, 1.0f * scalefactor, WHITE);
        }
        if (droppedItems[j].type == ITEM_IRONSWORD && ironswordLoaded) {
            DrawTextureEx(ironswordTexture, (Vector2){droppedItems[j].x, droppedItems[j].y}, 0, 1.0f * scalefactor, WHITE);
        }
        // "E ile al" yazısı
        float playerCenterX = player.x + 128 * scalefactor;
        float playerCenterY = player.y + 128 * scalefactor;
        float itemCenterX = droppedItems[j].x + 16;
        float itemCenterY = droppedItems[j].y + 16;
        float dx = playerCenterX - itemCenterX;
        float dy = playerCenterY - itemCenterY;
        float distance = sqrtf(dx*dx + dy*dy);
        if (distance < 64) {
            const char* text = "E ile al";
            int textWidth = MeasureText(text, 24);
            DrawText(text, droppedItems[j].x + 16 - textWidth/2, droppedItems[j].y - 28, 24, YELLOW);
        }
    }
}

