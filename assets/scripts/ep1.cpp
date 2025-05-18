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

// Çoklu zombi desteği için
typedef struct {
    float x, y;
    int level;
    bool active;
} Ep1Zombie;

#define EP1_ZOMBIE_COUNT 49 // 2-50 arası 49 zombi
#define MAX_DROPPED_ITEMS 20 // Maksimum düşen item sayısı

Ep1Zombie ep1Zombies[EP1_ZOMBIE_COUNT];
Zombie zombies[EP1_ZOMBIE_COUNT];

// Düşen itemler için global değişkenler
extern DroppedItem droppedItems[MAX_DROPPED_ITEMS];

// Game over ekranı için değişkenler
bool isGameOver = false;
Rectangle restartButton;
Rectangle menuButton;

// ep1.h başlık dosyası için fonksiyon prototipleri
typedef void (*VoidFloat3Func)(float, float, float);
void InitEp1Zombies();
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
    
    // 2-50. bölümlerin en sağına zombi koy
    for (int i = 0; i < EP1_ZOMBIE_COUNT; i++) {
        float d4W = d4_texture.width * scalefactor;
        float zombieSpriteH = 128 * scalefactor * 1.8f;
        float zombieHitboxH = zombieSpriteH * 0.9f;
        float zombieHitboxY = (zombieSpriteH - zombieHitboxH) / 2.0f;
        float zombieX = d3_texture.width * scalefactor + i * d4W + d4W - 200 * scalefactor;
        float zombieY = groundY - (zombieHitboxY + zombieHitboxH);
        ep1Zombies[i].x = zombieX;
        ep1Zombies[i].y = zombieY;
        // Her 10 bölümde bir level artıyor (2-50 arası bölümler için)
        ep1Zombies[i].level = 1 + (i / 10);
        ep1Zombies[i].active = false;
        InitZombie(&zombies[i], zombieX, zombieY);
        zombies[i].level = ep1Zombies[i].level;
        UpdateZombieStats(&zombies[i]);
        zombies[i].active = false;
    }
    InitEp1Zombies();
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
    }
    
    bool isMoving = player.moving;
    
    // Game over kontrolü
    if (playerHealth <= 0 && !isGameOver) {
        isGameOver = true;
        // Butonları oluştur
        float buttonWidth = 200.0f;
        float buttonHeight = 50.0f;
        float centerX = GetScreenWidth() / 2.0f;
        float centerY = GetScreenHeight() / 2.0f;
        
        restartButton = (Rectangle){
            centerX - buttonWidth - 20.0f,
            centerY + 50.0f,
            buttonWidth,
            buttonHeight
        };
        
        menuButton = (Rectangle){
            centerX + 20.0f,
            centerY + 50.0f,
            buttonWidth,
            buttonHeight
        };
    }
    
    // Sayı tuşlarıyla eşya seçme kontrolü
    for (int i = 0; i < INV_EQUIP_SIZE; i++) {
        if (IsKeyPressed(KEY_ONE + i)) {
            selectedEquipSlot = i;
            player.swordType = playerInventory.equip[i].type;
            player.hasSword = (player.swordType == ITEM_RUSTEDSWORD || player.swordType == ITEM_IRONSWORD);
        }
    }
    
    // Toplam harita genişliği: d3 + 49*d4
    float mapWidth = d3_texture.width + 49 * d4_texture.width;
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
    for (int i = 0; i < 49; i++) { // 49 tane d4.png
        DrawTexturePro(d4_texture,
            (Rectangle){0,0,(float)d4_texture.width,(float)d4_texture.height},
            (Rectangle){x + texW + i * d4W, y, d4W, d4H},
            (Vector2){0,0}, 0.0f, WHITE);
    }
    DrawPlayer(scalefactor);
    for (int i = 0; i < EP1_ZOMBIE_COUNT; i++) {
        DrawZombie(&zombies[i], scalefactor);
    }
    DrawDroppedItem(scalefactor);
    DrawHitboxes(zombies, EP1_ZOMBIE_COUNT);
    DrawRegionNumbers();
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

    // Game over ekranı
    if (isGameOver) {
        // Yarı saydam siyah arkaplan
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 180});
        
        // Game Over yazısı
        const char* gameOverText = "GAME OVER";
        int fontSize = 60;
        int textWidth = MeasureText(gameOverText, fontSize);
        DrawText(gameOverText, 
            GetScreenWidth()/2 - textWidth/2,
            GetScreenHeight()/2 - 100,
            fontSize,
            RED);
        
        // Yeniden başlat butonu
        DrawRectangleRec(restartButton, (Color){60, 60, 60, 220});
        DrawRectangleLinesEx(restartButton, 2, WHITE);
        const char* restartText = "Yeniden Baslat";
        int restartTextWidth = MeasureText(restartText, 20);
        DrawText(restartText,
            restartButton.x + (restartButton.width - restartTextWidth)/2,
            restartButton.y + (restartButton.height - 20)/2,
            20,
            WHITE);
        
        // Menü butonu
        DrawRectangleRec(menuButton, (Color){60, 60, 60, 220});
        DrawRectangleLinesEx(menuButton, 2, WHITE);
        const char* menuText = "Ana Menu";
        int menuTextWidth = MeasureText(menuText, 20);
        DrawText(menuText,
            menuButton.x + (menuButton.width - menuTextWidth)/2,
            menuButton.y + (menuButton.height - 20)/2,
            20,
            WHITE);
        
        // Buton tıklama kontrolü
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            
            if (CheckCollisionPointRec(mousePos, restartButton)) {
                StartNewGame();
                isGameOver = false;
            }
            else if (CheckCollisionPointRec(mousePos, menuButton)) {
                // Ana menüye dön
                isGameOver = false;
                extern bool returnToMainMenu;
                returnToMainMenu = true;  // Ana menüye dönüş işlevi
            }
        }
    }
}

void UpdateEp1Scene(float deltaTime) {
    // ESC tuşu kontrolü
    CheckPauseInput();
    if (isPaused) return;
    UpdateDebugConsole();
    UpdatePlayer();
    for (int i = 0; i < EP1_ZOMBIE_COUNT; i++) {
        UpdateZombie(&zombies[i]);
    }
    UpdateDroppedItem(); // Düşen itemleri güncelle
    // Kamera güncelleme
    UpdateCameraToPlayer(d3_texture.width + 49 * d4_texture.width, d3_texture.height);
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
    gameStartTime = GetTime();
    isPaused = false;
    isGameOver = false;  // Game over durumunu sıfırla
    
    // Envanteri sıfırla
    for (int i = 0; i < INV_BAG_SIZE; i++) {
        playerInventory.bag[i].type = ITEM_NONE;
    }
    for (int i = 0; i < INV_EQUIP_SIZE; i++) {
        playerInventory.equip[i].type = ITEM_NONE;
    }
    player.hasSword = false;
    player.swordType = ITEM_NONE;
    
    // Düşen itemleri sıfırla
    for (int i = 0; i < MAX_DROPPED_ITEMS; i++) {
        droppedItems[i].active = false;
        droppedItems[i].type = ITEM_NONE;
    }
    
    InitEp1Scene();
}

// Zombileri başlatmak için fonksiyon
void InitEp1Zombies() {
    for (int i = 0; i < EP1_ZOMBIE_COUNT; i++) {
        InitZombie(&zombies[i], ep1Zombies[i].x, ep1Zombies[i].y);
        zombies[i].level = ep1Zombies[i].level;
        UpdateZombieStats(&zombies[i]);
        zombies[i].active = ep1Zombies[i].active;
    }
}
