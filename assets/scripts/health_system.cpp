#include "health_system.h"
#include <raylib.h>
#include <cstdio>
#include <cmath>

// Global değişkenler
float playerHealth = 100.0f;
float playerMaxHealth = 100.0f;
float energy = 100.0f;
float maxEnergy = 100.0f;
float mana = 100.0f;
float maxMana = 100.0f;
float totalSeconds = 0.0f; // Toplam geçen süre (saniye)
double gameStartTime = GetTime(); // Oyun başlangıç zamanı

// Sağlık çubuğu için değişkenler
static Texture2D healthbar_texture;
static bool healthbar_loaded = false;

// Enerji sistemi sabitleri
#define ENERGY_REGEN_INTERVAL 1.0f  // 1 saniye
#define ENERGY_REGEN_AMOUNT 10.0f   // Saniyede 10 birim
#define ENERGY_DRAIN_RATE 20.0f     // Saniyede 20 birim tüketim

void DrawHealthBar(float x, float y, float scalefactor, bool isMoving) {
    if (!healthbar_loaded) {
        healthbar_texture = LoadTexture("assets/sprites/env/healthbar.png");
        healthbar_loaded = true;
    }

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
    snprintf(healthText, sizeof(healthText), "%d/%d", (int)playerHealth, (int)playerMaxHealth);
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
        float regenAmount = 5.0f; // 0.5 saniyede bir 5 birim
        float nextEnergy = energy + regenAmount;
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
    static double lastUpdateTime = gameStartTime;
    double currentTime = GetTime();
    double elapsedTime = currentTime - lastUpdateTime;

    if (elapsedTime >= 0.5) {
        if (isMoving) {
            if (isRunning) {
                energy -= 5.0f; // 0.5 saniyede 5 birim (saniyede 10)
            } else {
                energy -= 2.5f;  // 0.5 saniyede 2.5 birim (saniyede 5)
            }
        } else {
            energy += 5.0f; // 0.5 saniyede 5 birim artır (saniyede 10 birim)
        }
        totalSeconds = (currentTime - gameStartTime);
        lastUpdateTime = currentTime;
    }

    // Sınırları kontrol et
    if (energy > maxEnergy) energy = maxEnergy;
    if (energy < 0.0f) energy = 0.0f;
} 