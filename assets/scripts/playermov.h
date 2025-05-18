#ifndef PLAYERMOV_H
#define PLAYERMOV_H

#include <raylib.h>
#include "inventory.h"

struct Player {
    float x, y;
    float speed;
    int frame;
    float frameTimer;
    bool moving;
    bool facingRight;
    bool hasSword;  // Kılıç var mı?
    ItemType swordType; // Elindeki kılıcın türü
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
    // --- Eliksir için ---
    bool isHealing;
    double healStartTime;
    float healStartHealth;
    float healTargetHealth;
};

void InitPlayer(float startX, float startY);
void UpdatePlayer();
void DrawPlayer(float scalefactor);

extern struct Player player;
extern double lastDamageTime;

#endif 