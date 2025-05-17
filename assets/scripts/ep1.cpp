#include <raylib.h>
#include "../../globals.h"
#include "playermov.h"
#include "camera.h"
#include "inventory.h"
#include "health_system.h"
#include "menu_system.h"
#include "enemy.h"
#include <string.h>
#include <stdio.h>
#include <vector>
#include "../../window_manager.h"
#include "../../resource_manager.h"
#include "ep1.h"
#include "debug.h"

// d3.png görseli için global değişken
Texture2D d3_texture;
bool d3_loaded = false;

// d4.png görseli için global değişken
Texture2D d4_texture;
bool d4_loaded = false;

// ep1.h başlık dosyası için fonksiyon prototipleri
typedef void (*VoidFloat3Func)(float, float, float);
void InitEp1Scene();
void UnloadEp1Scene();
void DrawEp1Scene(float currentWidth, float currentHeight, float scalefactor);
void UpdateEp1Scene(float deltaTime);
void StartNewGame();

void InitEp1Scene() {
    if (!d3_loaded) {
        d3_texture = LoadTexture("assets/sprites/env/d3.png");
        d3_loaded = true;
    }
    if (!d4_loaded) {
        d4_texture = LoadTexture("assets/sprites/env/d4.png");
        d4_loaded = true;
    }
    
    float groundOffset = 80 * scalefactor; // Zemini biraz yukarı al
    float groundY = GetScreenHeight() - groundOffset;

    // Player için
    float playerSpriteH = 128 * scalefactor * 2.0f;
    float playerHitboxH = playerSpriteH * 0.9f;
    float playerHitboxY = (playerSpriteH - playerHitboxH) / 2.0f;
    float playerY = groundY - (playerHitboxY + playerHitboxH);
    float playerX = 0;
    InitPlayer(playerX, playerY);
    
    // Zombie için
    float zombieSpriteH = 128 * scalefactor * 1.8f;
    float zombieHitboxH = zombieSpriteH * 0.9f;
    float zombieHitboxY = (zombieSpriteH - zombieHitboxH) / 2.0f;
    float zombieX = d3_texture.width * scalefactor - 200 * scalefactor;
    float zombieY = groundY - (zombieHitboxY + zombieHitboxH);
    InitZombie(zombieX, zombieY);
    
    InitCamera();
    InitInventory();
}

void UnloadEp1Scene() {
    if (d3_loaded) {
        UnloadTexture(d3_texture);
        d3_loaded = false;
    }
    if (d4_loaded) {
        UnloadTexture(d4_texture);
        d4_loaded = false;
    }
    UnloadInventory();
}

void DrawEp1Scene(float currentWidth, float currentHeight, float scalefactor) {
    if (IsKeyPressed(KEY_TAB)) inventoryOpen = !inventoryOpen;
    if (inventoryOpen) {
        DrawInventory(scalefactor);
        return;
    }
    if (!d3_loaded || !d4_loaded) return;
    
    // Oyun duraklatılmamışsa player'ı güncelle
    if (!isPaused) {
        UpdatePlayer();
        UpdateZombie();
        UpdateDroppedItem();
    }
    
    bool isMoving = player.moving;
    
    // Sayı tuşlarıyla eşya seçme kontrolü
    for (int i = 0; i < INV_EQUIP_SIZE; i++) {
        if (IsKeyPressed(KEY_ONE + i)) {
            selectedEquipSlot = i;
            player.hasSword = (playerInventory.equip[i].type == ITEM_RUSTEDSWORD);
        }
    }
    
    // Toplam harita genişliği: d3 + 5*d4
    float mapWidth = d3_texture.width + 5 * d4_texture.width;
    UpdateCameraToPlayer(mapWidth, d3_texture.height);
    BeginMode2D(*GetCamera());
    
    float texW = d3_texture.width * scalefactor;
    float texH = d3_texture.height * scalefactor;
    float x = currentWidth/2 - texW/2;
    float y = currentHeight/2 - texH/2;
    DrawTexturePro(d3_texture,
        (Rectangle){0,0,(float)d3_texture.width,(float)d3_texture.height},
        (Rectangle){x, y, texW, texH},
        (Vector2){0,0}, 0.0f, WHITE);
    float d4W = d4_texture.width * scalefactor;
    float d4H = d4_texture.height * scalefactor;
    DrawTexturePro(d4_texture,
        (Rectangle){0,0,(float)d4_texture.width,(float)d4_texture.height},
        (Rectangle){x + texW, y, d4W, d4H},
        (Vector2){0,0}, 0.0f, WHITE);
    for (int i = 0; i < 5; i++) {
        DrawTexturePro(d4_texture,
            (Rectangle){0,0,(float)d4_texture.width,(float)d4_texture.height},
            (Rectangle){x + texW + i * d4W, y, d4W, d4H},
            (Vector2){0,0}, 0.0f, WHITE);
    }
    DrawPlayer(scalefactor);
    DrawZombie(scalefactor);
    DrawDroppedItem(scalefactor);
    DrawHitboxes();
    DrawRegionNumbers(); // Bölge numaralarını çiz
    EndMode2D();
    
    // Sol üste health bar çiz
    DrawHealthBar(16, 16, scalefactor, isMoving);
    
    // Oyun duraklatılmışsa menüyü göster ve güncelle
    if (isPaused) {
        UpdatePauseMenu(scalefactor);
        DrawPauseMenu(scalefactor);
    }

    // Oyun içi ekipman slotları
    DrawEquipSlots(scalefactor);

    // Debug konsolunu en sona al
    DrawDebugConsole();
}

void UpdateEp1Scene(float deltaTime) {
    // ESC tuşu kontrolü
    CheckPauseInput();
    
    // Oyun duraklatıldıysa güncelleme yapma
    if (isPaused) return;
    
    // Debug konsolunu güncelle
    UpdateDebugConsole();
    
    // Oyuncu güncelleme
    UpdatePlayer();
    
    // Zombi güncelleme
    UpdateZombie();
    
    // Yere düşen itemi güncelle
    UpdateDroppedItem();
    
    // Kamera güncelleme
    UpdateCameraToPlayer(d3_texture.width + 5 * d4_texture.width, d3_texture.height);
}

void StartNewGame() {
    // Oyun değişkenlerini sıfırla ve sahneyi başlat
    UnloadEp1Scene();
    playerHealth = 100;
    playerMaxHealth = 100;
    energy = 100;
    maxEnergy = 100;
    mana = 100;
    maxMana = 100;
    totalSeconds = 0.0f;
    gameStartTime = GetTime(); // Oyun başlangıç zamanını sıfırla
    isPaused = false;
    InitEp1Scene();
}
