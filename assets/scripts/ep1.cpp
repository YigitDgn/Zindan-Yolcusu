#include <raylib.h>
#include "../../globals.h"
#include "playermov.h"
#include "camera.h"
#include <string.h>
#include <stdio.h>
#include <vector>
#include "../../window_manager.h"
#include "../../resource_manager.h"

// d3.png görseli için global değişken
static Texture2D d3_texture;
static bool d3_loaded = false;

// d4.png görseli için global değişken
static Texture2D d4_texture;
static bool d4_loaded = false;

// Health bar için global değişkenler
static Texture2D healthbar_texture;
static bool healthbar_loaded = false;
static int playerHealth = 80; // Örnek: 80/100 can
static int playerMaxHealth = 100;
float energy = 100, maxEnergy = 100;
float mana = 100, maxMana = 100;
static float energyRegenTimer = 0.0f;

// --- ENVANTER SİSTEMİ ---
#define INV_BAG_SIZE 12
#define INV_EQUIP_SIZE 3 // baş, gövde, bacak
#define INV_HAND_SIZE 1

enum ItemType { ITEM_NONE, ITEM_RUSTEDSWORD };

struct InventorySlot {
    ItemType type;
};

struct Inventory {
    InventorySlot bag[INV_BAG_SIZE];
    InventorySlot equip[INV_EQUIP_SIZE];
    InventorySlot hand[INV_HAND_SIZE];
};

static Inventory playerInventory;
static bool inventoryOpen = false;
static Texture2D rustedSwordTexture;
static bool rustedSwordLoaded = false;

// --- SÜRÜKLE-BIRAK DEĞİŞKENLERİ ---
static bool dragging = false;
static int dragFrom = -1; // 0-11: bag, 100: hand
static ItemType dragItem = ITEM_NONE;

// Oyun içi ekipman kutuları için değişkenler
static const int EQUIP_BOX_SIZE = 48; // Kutucuk boyutu
static const int EQUIP_BOX_MARGIN = 8; // Kutucuklar arası boşluk
static const int EQUIP_BOX_START_Y = 16; // Başlangıç Y pozisyonu

// Rusted Sword animasyonları için değişkenler
static Texture2D rustedSwordIdleTexture;
static Texture2D rustedSwordWalkTexture;
static Texture2D rustedSwordRunTexture;
static bool rustedSwordAnimationsLoaded = false;
static int selectedEquipSlot = -1; // Seçili ekipman slotu (-1: seçili değil)

// Animasyon texture'larını dışarıdan erişilebilir yap
extern Texture2D sword_idle;
extern Texture2D sword_walk;
extern Texture2D sword_run;

// ep1.h başlık dosyası için fonksiyon prototipleri
typedef void (*VoidFloat3Func)(float, float, float);
void InitEp1Scene();
void UnloadEp1Scene();
void DrawEp1Scene(float currentWidth, float currentHeight, float scalefactor);
void DrawHealthBar(float x, float y, float scalefactor, bool isMoving);
void UpdateEnergy(float deltaTime, bool isMoving, bool isRunning);
void InitInventory();
void UnloadInventory();
void DrawInventory(float scalefactor);
void UpdateEp1Scene(float deltaTime);
void DrawPauseMenu(float scalefactor);
void UpdatePauseMenu();

// Menü için global değişkenler
static bool isPaused = false;
static bool returnToMainMenu = false;
static float musicVolume = 0.7f;
static float sfxVolume = 1.0f;
static int screenMode = 0;  // 0: Pencere modu, 1: Tam ekran
static const char* menuItems[] = {"RESUME", "OPTIONS", "QUIT"};
static int selectedMenuItem = 0;
static const int menuItemCount = 3;

void InitEp1Scene() {
    if (!d3_loaded) {
        d3_texture = LoadTexture("assets/sprites/env/d3.png");
        d3_loaded = true;
    }
    if (!d4_loaded) {
        d4_texture = LoadTexture("assets/sprites/env/d4.png");
        d4_loaded = true;
    }
    if (!healthbar_loaded) {
        healthbar_texture = LoadTexture("assets/sprites/env/healthbar.png");
        healthbar_loaded = true;
    }
    
    // Ground seviyesi: ekranın altı (ör: 1080p için 1080)
    float groundY = GetScreenHeight();
    float playerY = groundY - 256 * scalefactor; // Karakterin yüksekliğinin 2 katı kadar yukarıda
    float playerX = GetScreenWidth() / 2.0f - (128 * scalefactor) / 2.0f; // Karakterin genişliğinin yarısı kadar sola
    InitPlayer(playerX, playerY);
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
    if (healthbar_loaded) {
        UnloadTexture(healthbar_texture);
        healthbar_loaded = false;
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
    EndMode2D();
    
    // Sol üste health bar çiz
    DrawHealthBar(16, 16, scalefactor, isMoving);
    
    // Oyun içi ekipman kutularını çiz
    for (int i = 0; i < INV_EQUIP_SIZE; i++) {
        float x = GetScreenWidth() - (EQUIP_BOX_SIZE * (INV_EQUIP_SIZE - i) + EQUIP_BOX_MARGIN * (INV_EQUIP_SIZE - i - 1) + 16);
        float y = EQUIP_BOX_START_Y;
        
        // Kutucuk arkaplanı
        Color boxColor = (i == selectedEquipSlot) ? (Color){60, 60, 60, 200} : (Color){40, 40, 40, 200};
        DrawRectangle((int)x, (int)y, EQUIP_BOX_SIZE, EQUIP_BOX_SIZE, boxColor);
        DrawRectangleLines((int)x, (int)y, EQUIP_BOX_SIZE, EQUIP_BOX_SIZE, GRAY);
        
        // Ekipman varsa çiz
        if (playerInventory.equip[i].type == ITEM_RUSTEDSWORD && rustedSwordLoaded) {
            DrawTextureEx(rustedSwordTexture, 
                (Vector2){x + 4.0f, y + 4.0f}, 
                0, 
                0.75f, 
                WHITE);
        }
        
        // Tuş numarasını göster
        char keyText[2];
        snprintf(keyText, sizeof(keyText), "%d", i + 1);
        DrawText(keyText, (int)x + 4, (int)y + EQUIP_BOX_SIZE - 16, 16, WHITE);
        
        // Kutucuğa tıklama kontrolü
        Rectangle boxRect = {(float)x, (float)y, (float)EQUIP_BOX_SIZE, (float)EQUIP_BOX_SIZE};
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), boxRect)) {
            selectedEquipSlot = i;
            player.hasSword = (playerInventory.equip[i].type == ITEM_RUSTEDSWORD);
        }
    }
    
    // ESC tuşu ile menüyü aç
    if (IsKeyPressed(KEY_ESCAPE) && !isPaused) {
        isPaused = true;
    }
    
    // Oyun duraklatılmışsa menüyü göster ve güncelle
    if (isPaused) {
        UpdatePauseMenu();
        DrawPauseMenu(scalefactor);
    }
}

// Health bar çizim fonksiyonu
void DrawHealthBar(float x, float y, float scalefactor, bool isMoving) {
    // Kırmızı barı önce çiz
    int barWidth = healthbar_texture.width * scalefactor * 1.7f;
    int barHeight = healthbar_texture.height * scalefactor * 1.7f;
    int marginX = 18 * scalefactor;
    int marginY = 12 * scalefactor;
    int fillWidth = (barWidth - 2 * marginX) * ((float)playerHealth / playerMaxHealth);
    int fillHeight = barHeight - 2 * marginY;
    Color darkRed = (Color){120, 0, 0, 220};
    DrawRectangle(x + marginX, y + marginY, fillWidth, fillHeight, darkRed);

    // PNG'yi üstüne çiz
    DrawTextureEx(healthbar_texture, (Vector2){x, y}, 0.0f, scalefactor * 1.7f, WHITE);

    // Can bilgisini sağa yaz (daha büyük)
    char healthText[32];
    snprintf(healthText, sizeof(healthText), "%d/%d", playerHealth, playerMaxHealth);
    int fontSize = barHeight * 0.6f;
    int textX = x + barWidth + 32 * scalefactor;
    int textY = y + (barHeight - fontSize) / 2;
    DrawText(healthText, textX, textY, fontSize, RAYWHITE);

    // --- Enerji ve Mana Barları ---
    int barSpacing = 16 * scalefactor;
    int smallBarW = barWidth;
    int smallBarH = barHeight * 0.38f;
    int smallBarX = x;
    int energyY = y + barHeight + barSpacing;
    int manaY = energyY + smallBarH + barSpacing;
    float energyRatio = energy / maxEnergy;
    float manaRatio = mana / maxMana;

    // Enerji barı (sarı, çerçeve siyah, kalın)
    DrawRectangle(smallBarX, energyY, smallBarW, smallBarH, BLANK);
    DrawRectangle(smallBarX, energyY, smallBarW * energyRatio, smallBarH, YELLOW);
    
    // Enerji yenilenme göstergesi (yarı saydam sarı)
    if (!isMoving && energy < maxEnergy) {
        float nextEnergy = energy + 10;
        if (nextEnergy > maxEnergy) nextEnergy = maxEnergy;
        float regenRatio = (nextEnergy - energy) / maxEnergy;
        Color regenColor = (Color){255, 255, 0, 128}; // Yarı saydam sarı
        DrawRectangle(smallBarX + smallBarW * energyRatio, energyY, 
                     smallBarW * regenRatio, smallBarH, regenColor);
    }
    
    for(int i=0; i<6; i++)
        DrawRectangleLines(smallBarX-i, energyY-i, smallBarW+2*i, smallBarH+2*i, BLACK);
    // Enerji oranı yazısı (x/y)
    char energyText[16];
    snprintf(energyText, sizeof(energyText), "%d/%d", (int)energy, (int)maxEnergy);
    int smallFont = smallBarH * 1.1f;
    int energyTextX = smallBarX + smallBarW + 28 * scalefactor;
    int energyTextY = energyY + (smallBarH - smallFont) / 2;
    DrawText(energyText, energyTextX, energyTextY, smallFont, YELLOW);

    // Mana barı (mavi, çerçeve siyah, kalın)
    DrawRectangle(smallBarX, manaY, smallBarW, smallBarH, BLANK);
    DrawRectangle(smallBarX, manaY, smallBarW * manaRatio, smallBarH, BLUE);
    for(int i=0; i<6; i++)
        DrawRectangleLines(smallBarX-i, manaY-i, smallBarW+2*i, smallBarH+2*i, BLACK);
    // Mana oranı yazısı (x/y)
    char manaText[16];
    snprintf(manaText, sizeof(manaText), "%d/%d", (int)mana, (int)maxMana);
    int manaTextX = smallBarX + smallBarW + 28 * scalefactor;
    int manaTextY = manaY + (smallBarH - smallFont) / 2;
    DrawText(manaText, manaTextX, manaTextY, smallFont, BLUE);
}

void UpdateEnergy(float deltaTime, bool isMoving, bool isRunning) {
    // Enerji kaybı
    if (isMoving) {
        if (isRunning) {
            energy -= 10 * deltaTime;
        } else {
            energy -= 5 * deltaTime;
        }
    }
    // Enerji yenilenmesi (sadece hareket etmiyorsa)
    if (!isMoving) {
        energyRegenTimer += deltaTime;
        if (energyRegenTimer >= 1.0f) {
            energy += 10;
            energyRegenTimer = 0.0f;
        }
    } else {
        energyRegenTimer = 0.0f;
    }
    if (energy > maxEnergy) energy = maxEnergy;
    if (energy < 0) energy = 0;
}

void InitInventory() {
    for (int i = 0; i < INV_BAG_SIZE; i++) playerInventory.bag[i].type = ITEM_NONE;
    for (int i = 0; i < INV_EQUIP_SIZE; i++) playerInventory.equip[i].type = ITEM_NONE;
    for (int i = 0; i < INV_HAND_SIZE; i++) playerInventory.hand[i].type = ITEM_NONE;
    playerInventory.bag[0].type = ITEM_RUSTEDSWORD;
    if (!rustedSwordLoaded) {
        rustedSwordTexture = LoadTexture("assets/sprites/items/rustedsword.png");
        rustedSwordLoaded = true;
    }
}

void UnloadInventory() {
    if (rustedSwordLoaded) {
        UnloadTexture(rustedSwordTexture);
        rustedSwordLoaded = false;
    }
}

void DrawInventory(float scalefactor) {
    int slotSize = 64 * scalefactor;
    int margin = 16 * scalefactor;
    int startX = 200 * scalefactor;
    int startY = 100 * scalefactor;
    
    // Çanta (bag)
    DrawText("BAG", startX, startY - 40 * scalefactor, 32 * scalefactor, RAYWHITE);
    for (int i = 0; i < INV_BAG_SIZE; i++) {
        int row = i / 6;
        int col = i % 6;
        int x = startX + col * (slotSize + margin);
        int y = startY + row * (slotSize + margin);
        DrawRectangleLines(x, y, slotSize, slotSize, GRAY);
        Rectangle slotRect = {(float)x, (float)y, (float)slotSize, (float)slotSize};
        
        // Mouse ile üzerine gelme kontrolü
        if (CheckCollisionPointRec(GetMousePosition(), slotRect)) {
            if (playerInventory.bag[i].type != ITEM_NONE) {
                const char* itemName = "";
                switch (playerInventory.bag[i].type) {
                    case ITEM_RUSTEDSWORD:
                        itemName = "Rusted Sword";
                        break;
                    default:
                        itemName = "Unknown Item";
                        break;
                }
                // İtem ismini göster
                int textWidth = MeasureText(itemName, 20 * scalefactor);
                DrawRectangle(GetMouseX() + 10, GetMouseY() + 10, textWidth + 20, 30 * scalefactor, (Color){0, 0, 0, 200});
                DrawText(itemName, GetMouseX() + 20, GetMouseY() + 15, 20 * scalefactor, WHITE);
            }
        }
        
        // Sürükle başlat
        if (!dragging && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), slotRect)) {
            if (playerInventory.bag[i].type != ITEM_NONE) {
                dragging = true;
                dragFrom = i;
                dragItem = playerInventory.bag[i].type;
                playerInventory.bag[i].type = ITEM_NONE;
            }
        }
        
        // Sürükle bırak
        if (dragging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), slotRect)) {
            if (playerInventory.bag[i].type == ITEM_NONE) {
                playerInventory.bag[i].type = dragItem;
                dragging = false;
                dragFrom = -1;
                dragItem = ITEM_NONE;
                
                // Seçili slot güncellendiğinde hasSword'u kontrol et
                if (selectedEquipSlot >= 0 && selectedEquipSlot < INV_EQUIP_SIZE) {
                    player.hasSword = (playerInventory.equip[selectedEquipSlot].type == ITEM_RUSTEDSWORD);
                }
            }
        }
        
        if (playerInventory.bag[i].type == ITEM_RUSTEDSWORD && rustedSwordLoaded) {
            DrawTextureEx(rustedSwordTexture, (Vector2){x+8*scalefactor, y+8*scalefactor}, 0, scalefactor, WHITE);
        }
    }

    // Sağ üstteki karakter ekipmanları
    int equipStartX = GetScreenWidth() - (INV_EQUIP_SIZE * (slotSize + margin) + 50 * scalefactor);
    int equipStartY = 50 * scalefactor;
    DrawText("CHARACTER", equipStartX, equipStartY - 40 * scalefactor, 32 * scalefactor, RAYWHITE);
    
    for (int i = 0; i < INV_EQUIP_SIZE; i++) {
        int x = equipStartX + i * (slotSize + margin);
        int y = equipStartY;
        DrawRectangleLines(x, y, slotSize, slotSize, GRAY);
        Rectangle equipRect = {(float)x, (float)y, (float)slotSize, (float)slotSize};
        
        // Mouse ile üzerine gelme kontrolü
        if (CheckCollisionPointRec(GetMousePosition(), equipRect)) {
            if (playerInventory.equip[i].type != ITEM_NONE) {
                const char* itemName = "";
                switch (playerInventory.equip[i].type) {
                    case ITEM_RUSTEDSWORD:
                        itemName = "Rusted Sword";
                        break;
                    default:
                        itemName = "Unknown Item";
                        break;
                }
                // İtem ismini göster
                int textWidth = MeasureText(itemName, 20 * scalefactor);
                DrawRectangle(GetMouseX() + 10, GetMouseY() + 10, textWidth + 20, 30 * scalefactor, (Color){0, 0, 0, 200});
                DrawText(itemName, GetMouseX() + 20, GetMouseY() + 15, 20 * scalefactor, WHITE);
            }
        }
        
        // Sürükle başlat (ekipman)
        if (!dragging && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), equipRect)) {
            if (playerInventory.equip[i].type != ITEM_NONE) {
                dragging = true;
                dragFrom = 200 + i; // Ekipman slotları için özel indeks
                dragItem = playerInventory.equip[i].type;
                playerInventory.equip[i].type = ITEM_NONE;
            }
        }
        
        // Sürükle bırak (ekipman)
        if (dragging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), equipRect)) {
            if (playerInventory.equip[i].type == ITEM_NONE) {
                playerInventory.equip[i].type = dragItem;
                dragging = false;
                dragFrom = -1;
                dragItem = ITEM_NONE;
                
                // Seçili slot güncellendiğinde hasSword'u kontrol et
                if (selectedEquipSlot >= 0 && selectedEquipSlot < INV_EQUIP_SIZE) {
                    player.hasSword = (playerInventory.equip[selectedEquipSlot].type == ITEM_RUSTEDSWORD);
                }
            }
        }
        
        if (playerInventory.equip[i].type == ITEM_RUSTEDSWORD && rustedSwordLoaded) {
            DrawTextureEx(rustedSwordTexture, (Vector2){x+8*scalefactor, y+8*scalefactor}, 0, scalefactor, WHITE);
        }
    }

    // Sürüklenen item mouse ile birlikte çiz
    if (dragging && dragItem != ITEM_NONE) {
        Vector2 mp = GetMousePosition();
        if (dragItem == ITEM_RUSTEDSWORD && rustedSwordLoaded) {
            DrawTextureEx(rustedSwordTexture, (Vector2){mp.x-24*scalefactor, mp.y-24*scalefactor}, 0, scalefactor, (Color){255,255,255,200});
        }
    }
}

void UpdateEp1Scene(float deltaTime) {
    // Oyun duraklatılmışsa sadece enerji ve animasyon güncellemelerini durdur
    if (isPaused) {
        return;
    }
    
    bool isMoving = player.moving;
    bool isRunning = IsKeyDown(KEY_LEFT_SHIFT);
    UpdateEnergy(deltaTime, isMoving, isRunning);
}

void DrawPauseMenu(float scalefactor) {
    // Ayarlar menüsü açıksa sadece ayarlar menüsünü göster
    if (showSettings) {
        // Referans çözünürlük: 1920x1080
        float refSettingsW = 1056.0f; // 1920 * 0.55
        float refSettingsH = 702.0f;  // 1080 * 0.65
        float settingsW = refSettingsW * scalefactor;
        float settingsH = refSettingsH * scalefactor;
        float settingsX = GetScreenWidth()/2 - settingsW/2;
        float settingsYOffset = 60 * scalefactor; // Tüm içeriği aşağı kaydırmak için offset
        float settingsY = GetScreenHeight()/2 - settingsH/2 + settingsYOffset;
        DrawRectangleRounded((Rectangle){settingsX, settingsY, settingsW, settingsH}, 0.1f, 16, Fade(BLACK, 0.8f));
        DrawRectangleLinesEx((Rectangle){settingsX, settingsY, settingsW, settingsH}, 4, WHITE);
        int titleFont = (int)(76 * scalefactor); // 1080*0.07
        const char* ayarlarText = "Ayarlar";
        int titleWidth = MeasureText(ayarlarText, titleFont);
        DrawText(ayarlarText, settingsX + settingsW/2 - titleWidth/2, settingsY + 30 * scalefactor, titleFont, WHITE);

        // --- Ses Ayarları ---
        int ayarFont = (int)(54 * scalefactor); // 1080*0.05
        float sliderW = settingsW * 0.6f;
        float sliderH = 12 * scalefactor;
        float sliderX = settingsX + settingsW/2 - sliderW/2;
        float contentOffset = 60 * scalefactor;
        float sliderY1 = settingsY + 140 * scalefactor + contentOffset;
        float sliderY2 = sliderY1 + 110 * scalefactor;
        DrawText("Müzik Sesi", sliderX, sliderY1 - 80 * scalefactor, ayarFont, WHITE);
        DrawRectangleRec((Rectangle){sliderX, sliderY1, sliderW, sliderH}, Fade(GRAY,0.5f));
        float knobX1 = sliderX + musicVolume * sliderW;
        DrawCircle(knobX1, sliderY1 + sliderH/2, 18 * scalefactor, WHITE);
        DrawText("Efekt Sesi", sliderX, sliderY2 - 80 * scalefactor, ayarFont, WHITE);
        DrawRectangleRec((Rectangle){sliderX, sliderY2, sliderW, sliderH}, Fade(GRAY,0.5f));
        float knobX2 = sliderX + sfxVolume * sliderW;
        DrawCircle(knobX2, sliderY2 + sliderH/2, 18 * scalefactor, WHITE);

        // --- Değerleri sayıyla göster ---
        char musicVal[8], sfxVal[8];
        snprintf(musicVal, sizeof(musicVal), "%d", (int)(musicVolume*100));
        snprintf(sfxVal, sizeof(sfxVal), "%d", (int)(sfxVolume*100));
        DrawText(musicVal, sliderX + sliderW + 20 * scalefactor, sliderY1 - 10 * scalefactor, ayarFont, WHITE);
        DrawText(sfxVal, sliderX + sliderW + 20 * scalefactor, sliderY2 - 10 * scalefactor, ayarFont, WHITE);

        // Geri butonu
        float geriW = settingsW * 0.4f;
        float geriH = 50 * scalefactor;
        float geriX = settingsX + settingsW/2 - geriW/2;
        float geriY = settingsY + settingsH - geriH - 30 * scalefactor;
        Rectangle geriBtn = {geriX, geriY, geriW, geriH};
        // Hover efekti
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, geriBtn)) {
            DrawRectangleRec(geriBtn, Fade(WHITE, 0.2f));
        }
        DrawRectangleRounded(geriBtn, 0.3f, 8, Fade(GRAY, 0.7f));
        DrawRectangleLinesEx(geriBtn, 2, WHITE);
        int geriFont = (int)(geriH * 0.5f);
        const char* geriText = "Geri";
        int geriTextW = MeasureText(geriText, geriFont);
        int geriTextX = geriX + geriW/2 - geriTextW/2;
        int geriTextY = geriY + geriH/2 - geriFont/2;
        DrawText(geriText, geriTextX, geriTextY, geriFont, WHITE);
        return;
    }

    // Pause menüsü
    float panelW = GetScreenWidth() * 0.45f;
    float panelH = GetScreenHeight() * 0.38f;
    float panelX = GetScreenWidth()/2 - panelW/2;
    float panelY = GetScreenHeight()/2 - panelH/2;
    
    // Panel arka planı (daha transparant)
    DrawRectangleRounded((Rectangle){panelX, panelY, panelW, panelH}, 0.1f, 16, Fade(BLACK, 0.7f));
    DrawRectangleLinesEx((Rectangle){panelX, panelY, panelW, panelH}, 4, Fade(WHITE, 0.8f));
    
    // Başlık
    int titleFont = (int)(panelH * 0.18f);
    const char* panelTitle = "PAUSED";
    int titleWidth = MeasureText(panelTitle, titleFont);
    DrawText(panelTitle, panelX + panelW/2 - titleWidth/2, panelY + 30 * scalefactor, titleFont, Fade(WHITE, 0.9f));
    
    // Butonlar
    float btnW = panelW * 0.7f;
    float btnH = 60 * scalefactor;
    float btnX = panelX + panelW/2 - btnW/2;
    float btnY1 = panelY + panelH/2 - btnH * 1.5f;
    float btnY2 = panelY + panelH/2;
    float btnY3 = panelY + panelH/2 + btnH * 1.5f;
    
    Rectangle resumeBtn = {btnX, btnY1, btnW, btnH};
    Rectangle optionsBtn = {btnX, btnY2, btnW, btnH};
    Rectangle quitBtn = {btnX, btnY3, btnW, btnH};
    
    Vector2 mouse = GetMousePosition();
    
    // Hover efektleri (daha belirgin)
    if (CheckCollisionPointRec(mouse, resumeBtn)) {
        DrawRectangleRec(resumeBtn, Fade(WHITE, 0.3f));
    }
    if (CheckCollisionPointRec(mouse, optionsBtn)) {
        DrawRectangleRec(optionsBtn, Fade(WHITE, 0.3f));
    }
    if (CheckCollisionPointRec(mouse, quitBtn)) {
        DrawRectangleRec(quitBtn, Fade(WHITE, 0.3f));
    }
    
    // Buton arka planları (daha transparant)
    DrawRectangleRounded(resumeBtn, 0.3f, 8, Fade(GRAY, 0.5f));
    DrawRectangleRounded(optionsBtn, 0.3f, 8, Fade(GRAY, 0.5f));
    DrawRectangleRounded(quitBtn, 0.3f, 8, Fade(GRAY, 0.5f));
    DrawRectangleLinesEx(resumeBtn, 2, Fade(WHITE, 0.8f));
    DrawRectangleLinesEx(optionsBtn, 2, Fade(WHITE, 0.8f));
    DrawRectangleLinesEx(quitBtn, 2, Fade(WHITE, 0.8f));
    
    // Buton metinleri
    int btnFont = (int)(btnH * 0.5f);
    const char* resumeText = "DEVAM ET";
    const char* optionsText = "AYARLAR";
    const char* quitText = "QUIT";
    int resumeTextW = MeasureText(resumeText, btnFont);
    int optionsTextW = MeasureText(optionsText, btnFont);
    int quitTextW = MeasureText(quitText, btnFont);
    
    DrawText(resumeText, btnX + btnW/2 - resumeTextW/2, btnY1 + btnH/2 - btnFont/2, btnFont, Fade(WHITE, 0.9f));
    DrawText(optionsText, btnX + btnW/2 - optionsTextW/2, btnY2 + btnH/2 - btnFont/2, btnFont, Fade(WHITE, 0.9f));
    DrawText(quitText, btnX + btnW/2 - quitTextW/2, btnY3 + btnH/2 - btnFont/2, btnFont, Fade(WHITE, 0.9f));
}

void UpdatePauseMenu() {
    float panelW = GetScreenWidth() * 0.45f;
    float panelH = GetScreenHeight() * 0.38f;
    float panelX = GetScreenWidth()/2 - panelW/2;
    float panelY = GetScreenHeight()/2 - panelH/2;
    
    float btnW = panelW * 0.7f;
    float btnH = 60 * scalefactor;
    float btnX = panelX + panelW/2 - btnW/2;
    float btnY1 = panelY + panelH/2 - btnH * 1.5f;
    float btnY2 = panelY + panelH/2;
    float btnY3 = panelY + panelH/2 + btnH * 1.5f;
    
    Rectangle resumeBtn = {btnX, btnY1, btnW, btnH};
    Rectangle optionsBtn = {btnX, btnY2, btnW, btnH};
    Rectangle quitBtn = {btnX, btnY3, btnW, btnH};
    
    Vector2 mouse = GetMousePosition();
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (!showSettings) {
            if (CheckCollisionPointRec(mouse, resumeBtn)) {
                isPaused = false;
            }
            else if (CheckCollisionPointRec(mouse, optionsBtn)) {
                showSettings = true;
            }
            else if (CheckCollisionPointRec(mouse, quitBtn)) {
                UnloadEp1Scene();
                nextScene = SCENE_MENU;
                fadeAlpha = 0.0f;
                fadeDirection = 1;
                fading = true;
                isPaused = false;
            }
            else if (!CheckCollisionPointRec(mouse, (Rectangle){panelX, panelY, panelW, panelH})) {
                isPaused = false;
            }
        }
        else {
            // Ayarlar menüsü kontrolleri
            float refSettingsW = 1056.0f; // 1920 * 0.55
            float refSettingsH = 702.0f;  // 1080 * 0.65
            float settingsW = refSettingsW * scalefactor;
            float settingsH = refSettingsH * scalefactor;
            float settingsX = GetScreenWidth()/2 - settingsW/2;
            float settingsYOffset = 60 * scalefactor;
            float settingsY = GetScreenHeight()/2 - settingsH/2 + settingsYOffset;
            
            // Geri butonu
            float geriW = settingsW * 0.4f;
            float geriH = 50 * scalefactor;
            float geriX = settingsX + settingsW/2 - geriW/2;
            float geriY = settingsY + settingsH - geriH - 30 * scalefactor;
            Rectangle geriBtn = {geriX, geriY, geriW, geriH};
            
            if (CheckCollisionPointRec(mouse, geriBtn)) {
                showSettings = false;
            }
        }
    }
    
    // Ses ayarları için sürükleme kontrolü
    if (showSettings && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        float refSettingsW = 1056.0f;
        float refSettingsH = 702.0f;
        float settingsW = refSettingsW * scalefactor;
        float settingsH = refSettingsH * scalefactor;
        float settingsX = GetScreenWidth()/2 - settingsW/2;
        float settingsYOffset = 60 * scalefactor;
        float settingsY = GetScreenHeight()/2 - settingsH/2 + settingsYOffset;
        float sliderW = settingsW * 0.6f;
        float sliderH = 12 * scalefactor;
        float sliderX = settingsX + settingsW/2 - sliderW/2;
        float sliderY1 = settingsY + 140 * scalefactor + settingsYOffset;
        float sliderY2 = sliderY1 + 110 * scalefactor;
        
        // Müzik sesi
        Rectangle sliderRect1 = {sliderX, sliderY1 - 20 * scalefactor, sliderW, sliderH + 40 * scalefactor};
        if (CheckCollisionPointRec(mouse, sliderRect1)) {
            musicVolume = (mouse.x - sliderX) / sliderW;
            if (musicVolume < 0) musicVolume = 0;
            if (musicVolume > 1) musicVolume = 1;
        }
        
        // Efekt sesi
        Rectangle sliderRect2 = {sliderX, sliderY2 - 20 * scalefactor, sliderW, sliderH + 40 * scalefactor};
        if (CheckCollisionPointRec(mouse, sliderRect2)) {
            sfxVolume = (mouse.x - sliderX) / sliderW;
            if (sfxVolume < 0) sfxVolume = 0;
            if (sfxVolume > 1) sfxVolume = 1;
        }
    }
}

// Ana menüye dönüş kontrolü için yeni fonksiyon
bool ShouldReturnToMainMenu() {
    return returnToMainMenu;
}

// Ana menüye dönüş durumunu sıfırlama için yeni fonksiyon
void ResetReturnToMainMenu() {
    returnToMainMenu = false;
}

// Yeni oyun başlatma fonksiyonu
void StartNewGame() {
    // Mevcut sahneyi temizle
    UnloadEp1Scene();
    
    // Oyun değişkenlerini sıfırla
    playerHealth = 100;
    playerMaxHealth = 100;
    energy = 100;
    maxEnergy = 100;
    mana = 100;
    maxMana = 100;
    energyRegenTimer = 0.0f;
    isPaused = false;
    showSettings = false;
    
    // Envanteri sıfırla
    for (int i = 0; i < INV_BAG_SIZE; i++) playerInventory.bag[i].type = ITEM_NONE;
    for (int i = 0; i < INV_EQUIP_SIZE; i++) playerInventory.equip[i].type = ITEM_NONE;
    for (int i = 0; i < INV_HAND_SIZE; i++) playerInventory.hand[i].type = ITEM_NONE;
    
    // Sahneyi yeniden başlat
    InitEp1Scene();
}
