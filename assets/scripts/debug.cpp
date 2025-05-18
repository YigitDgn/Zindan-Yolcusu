#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include "../../globals.h"
#include "playermov.h"
#include "enemy.h"
#include "health_system.h"
#include "camera.h"
#include "ep1.h" // Texture değişkenleri için
#include "inventory.h"

// EP1_ZOMBIE_COUNT sabitini tanımla
#define EP1_ZOMBIE_COUNT 49

// Zombies dizisini extern olarak tanımla
extern Zombie zombies[EP1_ZOMBIE_COUNT];

// Debug konsolu için değişkenler
bool debugConsoleOpen = false;
bool gravePressed = false;
char debugText[1024] = {0}; // Debug metni için buffer
char commandInput[256] = {0}; // Komut girişi için buffer
int cursorPosition = 0;
bool commandMode = false;
bool enterPressed = false;
bool showHitboxes = false; // Hitbox gösterme durumu
bool infiniteMode = false; // Sonsuz mod durumu

// Komut geçmişi
#define MAX_HISTORY 10
char commandHistory[MAX_HISTORY][256];
int historyCount = 0;
int currentHistoryIndex = -1;

void ExecuteCommand(const char* command) {
    // Komutu geçmişe ekle
    if (historyCount < MAX_HISTORY) {
        strcpy(commandHistory[historyCount], command);
        historyCount++;
    } else {
        // En eski komutu sil ve yenisini ekle
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            strcpy(commandHistory[i], commandHistory[i + 1]);
        }
        strcpy(commandHistory[MAX_HISTORY - 1], command);
    }
    currentHistoryIndex = -1;

    // Komutları işle
    if (strcmp(command, "help") == 0) {
        snprintf(debugText, sizeof(debugText),
            "Kullanılabilir Komutlar:\n"
            "help - Komut listesini gösterir\n"
            "pos - Oyuncu pozisyonunu gösterir\n"
            "energy [değer] - Enerjiyi ayarlar\n"
            "speed [değer] - Oyuncu hızını ayarlar\n"
            "clear - Konsolu temizler\n"
            "zombie [x] [y] - Zombi pozisyonunu ayarlar\n"
            "hitbox [true/false] - Hitboxları gösterir/gizler"
        );
    }
    else if (strcmp(command, "pos") == 0) {
        snprintf(debugText, sizeof(debugText),
            "Oyuncu Pozisyonu: (%.1f, %.1f)\n"
            "Zombi Pozisyonu: (%.1f, %.1f)",
            player.x, player.y,
            zombies[0].x, zombies[0].y
        );
    }
    else if (strncmp(command, "energy ", 7) == 0) {
        float newEnergy;
        if (sscanf(command + 7, "%f", &newEnergy) == 1) {
            energy = newEnergy;
            if (energy > maxEnergy) energy = maxEnergy;
            if (energy < 0) energy = 0;
            snprintf(debugText, sizeof(debugText), "Enerji %.1f olarak ayarlandı", energy);
        }
    }
    else if (strncmp(command, "speed ", 6) == 0) {
        float newSpeed;
        if (sscanf(command + 6, "%f", &newSpeed) == 1) {
            player.speed = newSpeed;
            snprintf(debugText, sizeof(debugText), "Hız %.1f olarak ayarlandı", player.speed);
        }
    }
    else if (strcmp(command, "clear") == 0) {
        debugText[0] = '\0';
    }
    else if (strncmp(command, "zombie ", 7) == 0) {
        float x, y;
        if (sscanf(command + 7, "%f %f", &x, &y) == 2) {
            zombies[0].x = x;
            zombies[0].y = y;
            snprintf(debugText, sizeof(debugText), "Zombi pozisyonu (%.1f, %.1f) olarak ayarlandı", x, y);
        }
    }
    else if (strncmp(command, "hitbox ", 7) == 0) {
        if (strcmp(command + 7, "true") == 0) {
            showHitboxes = true;
            snprintf(debugText, sizeof(debugText), "Hitboxlar gösteriliyor");
        }
        else if (strcmp(command + 7, "false") == 0) {
            showHitboxes = false;
            snprintf(debugText, sizeof(debugText), "Hitboxlar gizlendi");
        }
    }
    else if (strcmp(command, "itemdebug") == 0) {
        extern bool rustedSwordLoaded;
        snprintf(debugText, sizeof(debugText),
            "rustedSwordLoaded: %d",
            rustedSwordLoaded
        );
    }
    else if (strncmp(command, "giveitem ", 9) == 0) {
        extern Inventory playerInventory;
        const char* itemName = command + 9;
        if (strcmp(itemName, "rustedsword") == 0) {
            for (int i = 0; i < INV_BAG_SIZE; i++) {
                if (playerInventory.bag[i].type == ITEM_NONE) {
                    playerInventory.bag[i].type = ITEM_RUSTEDSWORD;
                    snprintf(debugText, sizeof(debugText), "%s envantere eklendi.", itemName);
                    break;
                }
            }
        } else if (strcmp(itemName, "ironsword") == 0) {
            for (int i = 0; i < INV_BAG_SIZE; i++) {
                if (playerInventory.bag[i].type == ITEM_NONE) {
                    playerInventory.bag[i].type = ITEM_IRONSWORD;
                    snprintf(debugText, sizeof(debugText), "%s envantere eklendi.", itemName);
                    break;
                }
            }
        } else if (strcmp(itemName, "healthelixir") == 0) {
            for (int i = 0; i < INV_BAG_SIZE; i++) {
                if (playerInventory.bag[i].type == ITEM_NONE) {
                    playerInventory.bag[i].type = ITEM_HEALTHELIXIR;
                    snprintf(debugText, sizeof(debugText), "%s envantere eklendi.", itemName);
                    break;
                }
            }
        } else {
            snprintf(debugText, sizeof(debugText), "Bilinmeyen item: %s", itemName);
        }
    }
    else if (strncmp(command, "inf ", 4) == 0) {
        const char* value = command + 4;
        if (strcmp(value, "true") == 0) {
            infiniteMode = true;
            player.speed = 24.0f; // Normal hızın 12 katı
            snprintf(debugText, sizeof(debugText), "Sonsuz mod aktif, hız 24.0 olarak ayarlandı");
        } else if (strcmp(value, "false") == 0) {
            infiniteMode = false;
            player.speed = 2.0f; // Normal hıza geri dön
            snprintf(debugText, sizeof(debugText), "Sonsuz mod pasif, hız 2.0 olarak ayarlandı");
        }
    }
    else {
        snprintf(debugText, sizeof(debugText), "Bilinmeyen komut: %s\n'help' yazarak komutları görebilirsiniz", command);
    }
}

void UpdateDebugConsole() {
    // Grave tuşu kontrolü
    if (IsKeyPressed(KEY_GRAVE)) {
        if (!gravePressed) {
            debugConsoleOpen = !debugConsoleOpen;
            gravePressed = true;
            if (debugConsoleOpen) {
                commandMode = true;
                cursorPosition = 0;
                commandInput[0] = '\0';
            }
        }
    } else {
        gravePressed = false;
    }

    if (!debugConsoleOpen) return;

    // Komut girişi kontrolü
    if (commandMode) {
        // Konsol yeni açıldıysa, bu frame karakter girişi alma!
        static bool justOpened = false;
        if (IsKeyPressed(KEY_GRAVE)) {
            justOpened = true;
        }

        // Enter tuşu kontrolü
        if (IsKeyPressed(KEY_ENTER)) {
            if (!enterPressed) {
                if (strlen(commandInput) > 0) {
                    ExecuteCommand(commandInput);
                }
                commandInput[0] = '\0';
                cursorPosition = 0;
                enterPressed = true;
            }
        } else {
            enterPressed = false;
        }

        // Geri tuşu kontrolü
        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (cursorPosition > 0) {
                commandInput[--cursorPosition] = '\0';
            }
        }

        // Yukarı/Aşağı ok tuşları ile komut geçmişi
        if (IsKeyPressed(KEY_UP)) {
            if (currentHistoryIndex < historyCount - 1) {
                currentHistoryIndex++;
                strcpy(commandInput, commandHistory[historyCount - 1 - currentHistoryIndex]);
                cursorPosition = strlen(commandInput);
            }
        }
        else if (IsKeyPressed(KEY_DOWN)) {
            if (currentHistoryIndex > 0) {
                currentHistoryIndex--;
                strcpy(commandInput, commandHistory[historyCount - 1 - currentHistoryIndex]);
                cursorPosition = strlen(commandInput);
            }
            else if (currentHistoryIndex == 0) {
                currentHistoryIndex = -1;
                commandInput[0] = '\0';
                cursorPosition = 0;
            }
        }

        // Normal karakter girişi
        if (!justOpened) {
            int key = GetCharPressed();
            while (key > 0) {
                if (cursorPosition < 255) {
                    commandInput[cursorPosition++] = (char)key;
                    commandInput[cursorPosition] = '\0';
                }
                key = GetCharPressed();
            }
        } else {
            justOpened = false; // Sadece ilk frame atla
        }
    }
}

// Bölge numaralarını çizme fonksiyonu
void DrawRegionNumbers() {
    extern Texture2D d3_texture;
    extern Texture2D d4_texture;
    extern float scalefactor;
    
    int fontSize = 40;
    int padding = 20;
    
    // d3.png için 1 numarası
    float d3W = d3_texture.width * scalefactor;
    float d4W = d4_texture.width * scalefactor;
    float d3X = GetScreenWidth()/2 - d3W/2;
    float d3Y = GetScreenHeight()/2 - (d3_texture.height * scalefactor)/2;
    int d3CenterX = d3X + d3W/2;
    
    // d4.png'ler için 2,3,...,50 numaraları
    float d4X = d3X + d3W;
    float d4Y = d3Y;
    
    float playerX = player.x;
    // 1. bölge kontrolü
    if (playerX < d3W) {
        int x = d3CenterX - MeasureText("1", fontSize)/2;
        DrawText("1", x, padding - 10, 60, WHITE);
    }
    
    // 2-50 arası bölgeler için numaralar
    for (int i = 0; i < 49; i++) { // 49 tane d4.png, toplam 50 bölge
        int num = i + 2; // 2'den başla
        int x = d4X + (i * d4W) + (d4W/2) - MeasureText(TextFormat("%d", num), fontSize)/2;
        float regionStart = d3W + i * d4W;
        float regionEnd = d3W + (i+1) * d4W;
        bool isPlayerInRegion = (playerX >= regionStart && playerX < regionEnd);
        if (isPlayerInRegion) {
            fontSize = 60;
            DrawText(TextFormat("%d", num), x, padding - 10, fontSize, WHITE);
        } else {
            fontSize = 40;
            DrawText(TextFormat("%d", num), x, padding, fontSize, WHITE);
        }
    }
}

void DrawDebugConsole() {
    if (!debugConsoleOpen) return;

    // Yarı saydam siyah arkaplan
    DrawRectangle(10, 10, 600, 400, (Color){0, 0, 0, 200});
    
    // Debug metnini çiz
    DrawText(debugText, 20, 20, 20, WHITE);
    
    // Komut girişi alanı
    DrawRectangle(20, 380, 560, 30, (Color){0, 0, 0, 255});
    DrawText("> ", 20, 385, 20, WHITE);
    DrawText(commandInput, 40, 385, 20, WHITE);
    
    // İmleç çiz
    if (commandMode) {
        float cursorX = 40 + MeasureText(commandInput, 20);
        DrawRectangle(cursorX, 385, 2, 20, WHITE);
    }
    
    // Konsol çerçevesi
    DrawRectangleLinesEx((Rectangle){10, 10, 600, 400}, 2, WHITE);
}

// Fonksiyon prototipi
void DrawHitboxes(Zombie* zombies, int zombieCount);

// Hitbox çizme fonksiyonu
void DrawHitboxes(Zombie* zombies, int zombieCount) {
    if (!showHitboxes) return;
    Camera2D* camera = GetCamera();
    if (!camera) return;
    BeginMode2D(*camera);
    // Oyuncu hitbox'ı
    float spriteW = 128 * scalefactor * 2.0f;
    float spriteH = 128 * scalefactor * 2.0f;
    float playerHitboxW = spriteW * 0.6f;
    float playerHitboxH = spriteH * 0.9f;
    float playerHitboxX = player.x + (spriteW - playerHitboxW) / 2;
    float playerHitboxY = player.y + (spriteH - playerHitboxH) / 2;
    DrawRectangleLinesEx((Rectangle){playerHitboxX, playerHitboxY, playerHitboxW, playerHitboxH}, 2, RED);
    // Oyuncu saldırı alanı
    float playerAttackAreaW = playerHitboxW * 0.5f;
    float playerAttackAreaH = playerHitboxH;
    float playerAttackAreaX, playerAttackAreaY;
    if (player.facingRight) {
        playerAttackAreaX = playerHitboxX + playerHitboxW;
    } else {
        playerAttackAreaX = playerHitboxX - playerAttackAreaW;
    }
    playerAttackAreaY = playerHitboxY;
    DrawRectangleLinesEx((Rectangle){playerAttackAreaX, playerAttackAreaY, playerAttackAreaW, playerAttackAreaH}, 2, GREEN);
    // Çoklu zombi hitbox'ı
    for (int i = 0; i < zombieCount; i++) {
        if (zombies[i].isDead) continue; // Ölü zombilerin hitbox'larını çizme
        
        float zombieSpriteW = 128 * scalefactor * 1.8f;
        float zombieSpriteH = 128 * scalefactor * 1.8f;
        float zombieHitboxW = zombieSpriteW * 0.8f;
        float zombieHitboxH = zombieSpriteH * 0.9f;
        float zombieHitboxX = zombies[i].x + (zombieSpriteW - zombieHitboxW) / 2;
        float zombieHitboxY = zombies[i].y + (zombieSpriteH - zombieHitboxH) / 2;
        DrawRectangleLinesEx((Rectangle){zombieHitboxX, zombieHitboxY, zombieHitboxW, zombieHitboxH}, 2, RED);
        // Saldırı alanı
        float attackAreaW = zombieHitboxW * 0.5f;
        float attackAreaH = zombieHitboxH;
        float attackAreaX, attackAreaY;
        if (zombies[i].isAttackAnimPlaying) {
            attackAreaX = zombies[i].attackAreaStartX;
            attackAreaY = zombies[i].attackAreaStartY;
        } else {
            if (zombies[i].facingRight) {
                attackAreaX = zombieHitboxX + zombieHitboxW;
            } else {
                attackAreaX = zombieHitboxX - attackAreaW;
            }
            attackAreaY = zombieHitboxY;
        }
        DrawRectangleLinesEx((Rectangle){attackAreaX, attackAreaY, attackAreaW, attackAreaH}, 2, ORANGE);
        // Aktifleşme alanı
        float activationAreaW = zombieHitboxW * 10.0f;
        float activationAreaH = zombieHitboxH;
        float activationAreaX = zombieHitboxX + (zombieHitboxW - activationAreaW) / 2;
        float activationAreaY = zombieHitboxY;
        DrawRectangleLinesEx((Rectangle){activationAreaX, activationAreaY, activationAreaW, activationAreaH}, 2, BLUE);
    }
    EndMode2D();
} 