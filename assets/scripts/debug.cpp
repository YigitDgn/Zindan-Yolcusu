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

// Debug konsolu için değişkenler
bool debugConsoleOpen = false;
bool gravePressed = false;
char debugText[1024] = {0}; // Debug metni için buffer
char commandInput[256] = {0}; // Komut girişi için buffer
int cursorPosition = 0;
bool commandMode = false;
bool enterPressed = false;
bool showHitboxes = false; // Hitbox gösterme durumu

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
            zombie.x, zombie.y
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
            zombie.x = x;
            zombie.y = y;
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
        extern DroppedItem droppedItem;
        snprintf(debugText, sizeof(debugText),
            "rustedSwordLoaded: %d\ndroppedItem.active: %d\ndroppedItem.type: %d",
            rustedSwordLoaded, droppedItem.active, droppedItem.type
        );
    }
    else if (strncmp(command, "giveitem ", 9) == 0) {
        extern Inventory playerInventory;
        const char* itemName = command + 9;
        int itemType = 0;
        if (strcmp(itemName, "rustedsword") == 0) {
            itemType = 1; // ITEM_RUSTEDSWORD
        }
        if (itemType > 0) {
            bool added = false;
            for (int i = 0; i < INV_BAG_SIZE; i++) {
                if (playerInventory.bag[i].type == 0) {
                    playerInventory.bag[i].type = (ItemType)itemType;
                    snprintf(debugText, sizeof(debugText), "%s envantere eklendi.", itemName);
                    added = true;
                    break;
                }
            }
            if (!added) {
                snprintf(debugText, sizeof(debugText), "Envanterde boş slot yok!");
            }
        } else {
            snprintf(debugText, sizeof(debugText), "Bilinmeyen item: %s", itemName);
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
    float d3X = GetScreenWidth()/2 - (d3_texture.width * scalefactor)/2;
    float d3Y = GetScreenHeight()/2 - (d3_texture.height * scalefactor)/2;
    int d3CenterX = d3X + (d3_texture.width * scalefactor)/2;
    DrawText("1", d3CenterX - MeasureText("1", fontSize)/2, padding, fontSize, WHITE);
    
    // d4.png'ler için 2,3,4,5,6 numaraları
    float d4X = d3X + (d3_texture.width * scalefactor);
    float d4Y = d3Y;
    float d4Width = d4_texture.width * scalefactor;
    
    for (int i = 0; i < 5; i++) {
        int num = i + 2; // 2'den başla
        int x = d4X + (i * d4Width) + (d4Width/2) - MeasureText(TextFormat("%d", num), fontSize)/2;
        
        // Karakterin bulunduğu bölgeyi belirle
        float playerX = player.x;
        float currentRegionX = d4X + (i * d4Width);
        bool isPlayerInRegion = (playerX >= currentRegionX && playerX < currentRegionX + d4Width) ||
                              (i == 0 && playerX >= d3X && playerX < d4X);
        
        // Eğer bu bölge karakterin bulunduğu bölgeyse, numarayı büyüt
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

// Hitbox çizme fonksiyonu
void DrawHitboxes() {
    if (!showHitboxes) return;

    Camera2D* camera = GetCamera();
    if (!camera) return;

    // Oyuncu hitbox'ı
    float spriteW = 128 * scalefactor * 2.0f;
    float spriteH = 128 * scalefactor * 2.0f;
    Vector2 playerScreenTopLeft = GetWorldToScreen2D((Vector2){player.x, player.y}, *camera);
    float playerHitboxW = spriteW * 0.6f;
    float playerHitboxH = spriteH * 0.9f;
    float playerHitboxX = playerScreenTopLeft.x + (spriteW - playerHitboxW) / 2;
    float playerHitboxY = playerScreenTopLeft.y + (spriteH - playerHitboxH) / 2;
    DrawRectangleLinesEx(
        (Rectangle){playerHitboxX, playerHitboxY, playerHitboxW, playerHitboxH},
        2,
        RED
    );

    // Oyuncu saldırı alanı (attack area)
    float playerAttackAreaW = playerHitboxW * 0.5f;
    float playerAttackAreaH = playerHitboxH;
    float playerAttackAreaX, playerAttackAreaY;
    if (player.facingRight) {
        playerAttackAreaX = playerHitboxX + playerHitboxW;
    } else {
        playerAttackAreaX = playerHitboxX - playerAttackAreaW;
    }
    playerAttackAreaY = playerHitboxY;
    DrawRectangleLinesEx(
        (Rectangle){playerAttackAreaX, playerAttackAreaY, playerAttackAreaW, playerAttackAreaH},
        2,
        GREEN
    );

    // Zombi hitbox'ı
    float zombieSpriteW = 128 * scalefactor * 1.8f;
    float zombieSpriteH = 128 * scalefactor * 1.8f;
    Vector2 zombieScreenTopLeft = GetWorldToScreen2D((Vector2){zombie.x, zombie.y}, *camera);
    float zombieHitboxW = zombieSpriteW * 0.8f;
    float zombieHitboxH = zombieSpriteH * 0.9f;
    float zombieHitboxX = zombieScreenTopLeft.x + (zombieSpriteW - zombieHitboxW) / 2;
    float zombieHitboxY = zombieScreenTopLeft.y + (zombieSpriteH - zombieHitboxH) / 2;
    DrawRectangleLinesEx(
        (Rectangle){zombieHitboxX, zombieHitboxY, zombieHitboxW, zombieHitboxH},
        2,
        RED
    );

    // Zombi saldırı alanı (attack area)
    float attackAreaW = zombieHitboxW * 0.5f; // Eninin yarısı
    float attackAreaH = zombieHitboxH;        // Kendi yüksekliğiyle aynı
    float attackAreaX, attackAreaY;
    if (zombie.isAttackAnimPlaying) {
        // Saldırı animasyonu sırasında sabit DÜNYA konumunu kameradan geçirerek kullan
        Vector2 attackAreaScreen = GetWorldToScreen2D((Vector2){zombie.attackAreaStartX, zombie.attackAreaStartY}, *camera);
        attackAreaX = attackAreaScreen.x;
        attackAreaY = attackAreaScreen.y;
    } else {
        if (zombie.facingRight) {
            attackAreaX = zombieHitboxX + zombieHitboxW;
        } else {
            attackAreaX = zombieHitboxX - attackAreaW;
        }
        attackAreaY = zombieHitboxY;
    }
    DrawRectangleLinesEx(
        (Rectangle){attackAreaX, attackAreaY, attackAreaW, attackAreaH},
        2,
        ORANGE
    );

    // Zombi aktifleşme alanı (activation area)
    float activationAreaW = zombieHitboxW * 10.0f;
    float activationAreaH = zombieHitboxH;
    float activationAreaX = zombieHitboxX + (zombieHitboxW - activationAreaW) / 2;
    float activationAreaY = zombieHitboxY;
    DrawRectangleLinesEx(
        (Rectangle){activationAreaX, activationAreaY, activationAreaW, activationAreaH},
        2,
        BLUE
    );
} 