#pragma once

#include <raylib.h>

// Envanter boyutları
#define INV_BAG_SIZE 12
#define INV_EQUIP_SIZE 3 // baş, gövde, bacak
#define INV_HAND_SIZE 1

// Ekipman kutuları için sabitler
#define EQUIP_BOX_SIZE 48 // Kutucuk boyutu
#define EQUIP_BOX_MARGIN 8 // Kutucuklar arası boşluk
#define EQUIP_BOX_START_Y 16 // Başlangıç Y pozisyonu

// Eşya tipleri
enum ItemType { ITEM_NONE, ITEM_RUSTEDSWORD };

// Envanter slot yapısı
struct InventorySlot {
    ItemType type;
};

// Ana envanter yapısı
struct Inventory {
    InventorySlot bag[INV_BAG_SIZE];
    InventorySlot equip[INV_EQUIP_SIZE];
    InventorySlot hand[INV_HAND_SIZE];
};

// Envanter fonksiyonları
void InitInventory();
void UnloadInventory();
void DrawInventory(float scalefactor);
void DrawEquipSlots(float scalefactor);
void UpdateInventory();

// Dışarıdan erişilebilir değişkenler
extern Inventory playerInventory;
extern bool inventoryOpen;
extern Texture2D rustedSwordTexture;
extern bool rustedSwordLoaded;
extern int selectedEquipSlot; 