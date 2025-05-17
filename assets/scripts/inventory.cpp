#include "inventory.h"
#include <raylib.h>
#include <cstdio>

// Global değişkenler
Inventory playerInventory;
bool inventoryOpen = false;
Texture2D rustedSwordTexture;
bool rustedSwordLoaded = false;
int selectedEquipSlot = -1;

// Sürükle-bırak değişkenleri
static bool dragging = false;
static int dragFrom = -1; // 0-11: bag, 100: hand
static ItemType dragItem = ITEM_NONE;

void InitInventory() {
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

// Oyun içi ekipman slotlarını çiz
void DrawEquipSlots(float scalefactor) {
    int startX = GetScreenWidth() - (INV_EQUIP_SIZE * (EQUIP_BOX_SIZE + EQUIP_BOX_MARGIN)) - 16;
    int startY = EQUIP_BOX_START_Y;
    
    for (int i = 0; i < INV_EQUIP_SIZE; i++) {
        int x = startX + i * (EQUIP_BOX_SIZE + EQUIP_BOX_MARGIN);
        int y = startY;
        
        // Slot arkaplanı
        Color boxColor = (i == selectedEquipSlot) ? (Color){60, 60, 60, 220} : (Color){40, 40, 40, 220};
        DrawRectangle(x, y, EQUIP_BOX_SIZE, EQUIP_BOX_SIZE, boxColor);
        DrawRectangleLinesEx((Rectangle){(float)x, (float)y, (float)EQUIP_BOX_SIZE, (float)EQUIP_BOX_SIZE}, 2, GRAY);
        
        // Eşya varsa çiz
        if (playerInventory.equip[i].type == ITEM_RUSTEDSWORD && rustedSwordLoaded) {
            DrawTextureEx(rustedSwordTexture, (Vector2){(float)(x + 4), (float)(y + 4)}, 0, 0.75f, WHITE);
        }
        
        // Slot numarası
        char keyText[2];
        snprintf(keyText, sizeof(keyText), "%d", i + 1);
        DrawText(keyText, x + 4, y + EQUIP_BOX_SIZE - 18, 18, WHITE);
    }
}

void DrawInventory(float scalefactor) {
    // Arkaplan
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 180});

    // Başlık
    int titleFont = 44 * scalefactor;
    const char* title = "ENVANTER";
    int titleWidth = MeasureText(title, titleFont);
    DrawText(title, GetScreenWidth()/2 - titleWidth/2, 32 * scalefactor, titleFont, WHITE);

    // Çanta slotları (2 satır, 6 sütun)
    int bagCols = 6;
    int bagRows = 2;
    int bagStartX = GetScreenWidth()/2 - (bagCols * EQUIP_BOX_SIZE + (bagCols-1) * EQUIP_BOX_MARGIN)/2;
    int bagStartY = 120 * scalefactor;
    for (int i = 0; i < INV_BAG_SIZE; i++) {
        int col = i % bagCols;
        int row = i / bagCols;
        int x = bagStartX + col * (EQUIP_BOX_SIZE + EQUIP_BOX_MARGIN);
        int y = bagStartY + row * (EQUIP_BOX_SIZE + EQUIP_BOX_MARGIN);
        // Slot arkaplanı
        DrawRectangle(x, y, EQUIP_BOX_SIZE, EQUIP_BOX_SIZE, (Color){40, 40, 40, 220});
        DrawRectangleLinesEx((Rectangle){(float)x, (float)y, (float)EQUIP_BOX_SIZE, (float)EQUIP_BOX_SIZE}, 2, GRAY);
        // Eşya varsa çiz
        if (playerInventory.bag[i].type == ITEM_RUSTEDSWORD && rustedSwordLoaded) {
            DrawTextureEx(rustedSwordTexture, (Vector2){(float)(x + 4), (float)(y + 4)}, 0, 0.75f, WHITE);
        }
    }

    // Ekipman slotları (baş, gövde, bacak)
    int equipStartX = GetScreenWidth()/2 - (INV_EQUIP_SIZE * EQUIP_BOX_SIZE + (INV_EQUIP_SIZE-1) * EQUIP_BOX_MARGIN)/2;
    int equipStartY = bagStartY + bagRows * (EQUIP_BOX_SIZE + EQUIP_BOX_MARGIN) + 32 * scalefactor;
    for (int i = 0; i < INV_EQUIP_SIZE; i++) {
        int x = equipStartX + i * (EQUIP_BOX_SIZE + EQUIP_BOX_MARGIN);
        int y = equipStartY;
        Color boxColor = (i == selectedEquipSlot) ? (Color){60, 60, 60, 220} : (Color){40, 40, 40, 220};
        DrawRectangle(x, y, EQUIP_BOX_SIZE, EQUIP_BOX_SIZE, boxColor);
        DrawRectangleLinesEx((Rectangle){(float)x, (float)y, (float)EQUIP_BOX_SIZE, (float)EQUIP_BOX_SIZE}, 2, GRAY);
        // Eşya varsa çiz
        if (playerInventory.equip[i].type == ITEM_RUSTEDSWORD && rustedSwordLoaded) {
            DrawTextureEx(rustedSwordTexture, (Vector2){(float)(x + 4), (float)(y + 4)}, 0, 0.75f, WHITE);
        }
        // Slot numarası
        char keyText[2];
        snprintf(keyText, sizeof(keyText), "%d", i + 1);
        DrawText(keyText, x + 4, y + EQUIP_BOX_SIZE - 18, 18, WHITE);
    }

    // Sürükle-bırak işlemleri
    Vector2 mousePos = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (!dragging) {
            // Sürükleme başlat
            for (int i = 0; i < INV_BAG_SIZE; i++) {
                int col = i % bagCols;
                int row = i / bagCols;
                int x = bagStartX + col * (EQUIP_BOX_SIZE + EQUIP_BOX_MARGIN);
                int y = bagStartY + row * (EQUIP_BOX_SIZE + EQUIP_BOX_MARGIN);
                if (CheckCollisionPointRec(mousePos, (Rectangle){(float)x, (float)y, (float)EQUIP_BOX_SIZE, (float)EQUIP_BOX_SIZE})) {
                    if (playerInventory.bag[i].type != ITEM_NONE) {
                        dragging = true;
                        dragFrom = i;
                        dragItem = playerInventory.bag[i].type;
                        playerInventory.bag[i].type = ITEM_NONE;
                    }
                    break;
                }
            }
            for (int i = 0; i < INV_EQUIP_SIZE; i++) {
                int x = equipStartX + i * (EQUIP_BOX_SIZE + EQUIP_BOX_MARGIN);
                int y = equipStartY;
                if (CheckCollisionPointRec(mousePos, (Rectangle){(float)x, (float)y, (float)EQUIP_BOX_SIZE, (float)EQUIP_BOX_SIZE})) {
                    if (playerInventory.equip[i].type != ITEM_NONE) {
                        dragging = true;
                        dragFrom = 100 + i;
                        dragItem = playerInventory.equip[i].type;
                        playerInventory.equip[i].type = ITEM_NONE;
                    }
                    break;
                }
            }
        }
    } else if (dragging) {
        // Sürükleme bitir
        bool placed = false;
        // Çanta slotlarına bırakma
        for (int i = 0; i < INV_BAG_SIZE; i++) {
            int col = i % bagCols;
            int row = i / bagCols;
            int x = bagStartX + col * (EQUIP_BOX_SIZE + EQUIP_BOX_MARGIN);
            int y = bagStartY + row * (EQUIP_BOX_SIZE + EQUIP_BOX_MARGIN);
            if (CheckCollisionPointRec(mousePos, (Rectangle){(float)x, (float)y, (float)EQUIP_BOX_SIZE, (float)EQUIP_BOX_SIZE})) {
                if (playerInventory.bag[i].type == ITEM_NONE) {
                    playerInventory.bag[i].type = dragItem;
                    placed = true;
                }
                break;
            }
        }
        // Ekipman slotlarına bırakma
        if (!placed) {
            for (int i = 0; i < INV_EQUIP_SIZE; i++) {
                int x = equipStartX + i * (EQUIP_BOX_SIZE + EQUIP_BOX_MARGIN);
                int y = equipStartY;
                if (CheckCollisionPointRec(mousePos, (Rectangle){(float)x, (float)y, (float)EQUIP_BOX_SIZE, (float)EQUIP_BOX_SIZE})) {
                    if (playerInventory.equip[i].type == ITEM_NONE) {
                        playerInventory.equip[i].type = dragItem;
                        placed = true;
                    }
                    break;
                }
            }
        }
        // Eşyayı yerleştiremediyse geri koy
        if (!placed) {
            if (dragFrom >= 0 && dragFrom < INV_BAG_SIZE) {
                playerInventory.bag[dragFrom].type = dragItem;
            } else if (dragFrom >= 100 && dragFrom < 100 + INV_EQUIP_SIZE) {
                playerInventory.equip[dragFrom - 100].type = dragItem;
            }
        }
        dragging = false;
        dragFrom = -1;
        dragItem = ITEM_NONE;
    }
    // Sürüklenen eşyayı çiz
    if (dragging) {
        if (rustedSwordLoaded && dragItem == ITEM_RUSTEDSWORD) {
            DrawTextureEx(rustedSwordTexture, (Vector2){mousePos.x - EQUIP_BOX_SIZE/2, mousePos.y - EQUIP_BOX_SIZE/2}, 0, 0.75f, WHITE);
        }
    }
} 