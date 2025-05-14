#include <raylib.h>
#include "../../globals.h"
#include "playermov.h"
#include "camera.cpp"

// d3.png görseli için global değişken
static Texture2D d3_texture;
static bool d3_loaded = false;

// d4.png görseli için global değişken
static Texture2D d4_texture;
static bool d4_loaded = false;

// ep1.h başlık dosyası için fonksiyon prototipleri
typedef void (*VoidFloat3Func)(float, float, float);
void InitEp1Scene();
void UnloadEp1Scene();
void DrawEp1Scene(float currentWidth, float currentHeight, float scalefactor);

void InitEp1Scene() {
    if (!d3_loaded) {
        d3_texture = LoadTexture("assets/sprites/env/d3.png");
        d3_loaded = true;
    }
    if (!d4_loaded) {
        d4_texture = LoadTexture("assets/sprites/env/d4.png");
        d4_loaded = true;
    }
    // Ground seviyesi: ekranın altı (ör: 1080p için 1080)
    float groundY = GetScreenHeight();
    float playerY = groundY - 125 * scalefactor - idle.height * scalefactor;
    float playerX = GetScreenWidth() / 2.0f - (idle.width / 4) * scalefactor / 2.0f;
    InitPlayer(playerX, playerY);
    InitCamera();
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
}

void DrawEp1Scene(float currentWidth, float currentHeight, float scalefactor) {
    if (!d3_loaded || !d4_loaded) return;
    UpdatePlayer();
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
    // d4.png'yi d3.png'nin sağına 5 kere tileable şekilde çiz
    for (int i = 0; i < 5; i++) {
        DrawTexturePro(d4_texture,
            (Rectangle){0,0,(float)d4_texture.width,(float)d4_texture.height},
            (Rectangle){x + texW + i * d4W, y, d4W, d4H},
            (Vector2){0,0}, 0.0f, WHITE);
    }
    DrawPlayer(scalefactor);
    EndMode2D();
}
