#ifndef ENEMY_H
#define ENEMY_H

#include <raylib.h>

// Yere düşen item yapısı
struct DroppedItem {
    float x, y;
    bool active;
    int type; // 0: yok, 1: rusted sword
};

struct Zombie {
    float x, y;
    float speed;
    int frame;
    float frameTimer;
    bool moving;
    bool facingRight;
    bool isAttacking;
    float attackCooldown;
    bool active;
    float attackAnimTimer;
    int attackAnimFrame;
    bool isAttackAnimPlaying;
    double attackAnimLastTime;
    bool attackAnimTriggered;
    float attackAreaStartX;
    float attackAreaStartY;
    float attackAnimStartX;
    float attackAnimTargetX;
    bool hasDealtDamage;
    float health; // Zombi canı
    double lastDamageTime; // Son hasar alma zamanı
    bool isDead; // Zombi öldü mü?
    int level; // Zombi seviyesi
    float maxHealth; // Maksimum can
    float baseDamage; // Temel hasar
    float expValue; // Öldüğünde verilecek deneyim puanı
    bool hasDroppedItem; // Item düşürdü mü?
    bool willDropRustedSword; // Rusted sword düşürecek mi?
};

void InitZombie(struct Zombie* zombie, float startX, float startY);
void UpdateZombie(struct Zombie* zombie);
void DrawZombie(struct Zombie* zombie, float scalefactor);
void UpdateZombieStats(struct Zombie* zombie); // Level bazlı özellikleri güncelleme fonksiyonu
void DropItem(struct Zombie* zombie); // Item düşürme fonksiyonu

// Yere düşen item ile ilgili fonksiyonlar
typedef struct DroppedItem DroppedItem;
extern DroppedItem droppedItem;
void UpdateDroppedItem();
void DrawDroppedItem(float scalefactor);

#endif // ENEMY_H 