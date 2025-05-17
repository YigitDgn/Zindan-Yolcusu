#include "menu_system.h"
#include <cstdio>

// Global değişkenler
bool isPaused = false;
bool returnToMainMenu = false;
float musicVolume = 0.7f;
float sfxVolume = 1.0f;
int screenMode = 0;  // 0: Pencere modu, 1: Tam ekran
static const char* menuItems[] = {"Devam Et", "Ayarlar", "Ana Menü"};
static int selectedMenuItem = 0;
static const int menuItemCount = 3;
static bool escPressed = false;

void CheckPauseInput() {
    // ESC tuşu kontrolü
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (!escPressed) {
            isPaused = !isPaused;
            escPressed = true;
        }
    } else {
        escPressed = false;
    }
}

void DrawPauseMenu(float scalefactor) {
    // Menü kapalıysa çizme
    if (!isPaused) return;

    // Yarı saydam siyah arkaplan
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 200});
    
    // Menü başlığı
    const char* title = "Duraklatıldı";
    int titleFont = (int)(76 * scalefactor);
    int titleWidth = MeasureText(title, titleFont);
    DrawText(title, 
        GetScreenWidth()/2 - titleWidth/2, 
        100 * scalefactor, 
        titleFont, 
        WHITE);
    
    // Menü öğeleri
    float buttonWidth = 400 * scalefactor;
    float buttonHeight = 60 * scalefactor;
    float buttonX = GetScreenWidth()/2 - buttonWidth/2;
    float buttonY_start = 300 * scalefactor;
    float spacing = 30 * scalefactor;

    for (int i = 0; i < menuItemCount; i++) {
        float buttonY = buttonY_start + i * (buttonHeight + spacing);
        Rectangle btnRect = {buttonX, buttonY, buttonWidth, buttonHeight};
        Vector2 mouse = GetMousePosition();

        // Hover efekti
        if (CheckCollisionPointRec(mouse, btnRect)) {
            DrawRectangleRec(btnRect, Fade(WHITE, 0.2f));
        }

        // Buton arkaplanı
        DrawRectangleRounded(btnRect, 0.3f, 8, Fade(GRAY, 0.7f));
        DrawRectangleLinesEx(btnRect, 2, WHITE);

        // Buton metni
        const char* item = menuItems[i];
        int btnFont = (int)(buttonHeight * 0.5f);
        int textWidth = MeasureText(item, btnFont);
        DrawText(item, 
            buttonX + buttonWidth/2 - textWidth/2, 
            buttonY + buttonHeight/2 - btnFont/2, 
            btnFont, 
            (i == selectedMenuItem) ? YELLOW : WHITE);
    }
    
    // Ayarlar menüsü
    if (selectedMenuItem == 1) { // AYARLAR seçili
        float settingsW = GetScreenWidth() * 0.45f;
        float settingsH = GetScreenHeight() * 0.38f;
        float settingsX = GetScreenWidth()/2 - settingsW/2;
        float settingsY = GetScreenHeight()/2 - settingsH/2;
        
        // Ayarlar paneli
        DrawRectangleRounded((Rectangle){settingsX, settingsY, settingsW, settingsH}, 0.1f, 16, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx((Rectangle){settingsX, settingsY, settingsW, settingsH}, 4, WHITE);

        // Başlık
        int titleFont = (int)(settingsH * 0.18f);
        const char* settingsTitle = "Ayarlar";
        int titleWidth = MeasureText(settingsTitle, titleFont);
        DrawText(settingsTitle, settingsX + settingsW/2 - titleWidth/2, settingsY + 30 * scalefactor, titleFont, WHITE);

        // Ses ayarları
        int ayarFont = (int)(54 * scalefactor);
        float sliderW = settingsW * 0.6f;
        float sliderH = 12 * scalefactor;
        float sliderX = settingsX + settingsW/2 - sliderW/2;
        float contentOffset = 60 * scalefactor;
        float sliderY1 = settingsY + 140 * scalefactor + contentOffset;
        float sliderY2 = sliderY1 + 110 * scalefactor;

        // Müzik sesi
        DrawText("Müzik Sesi", sliderX, sliderY1 - 80 * scalefactor, ayarFont, WHITE);
        DrawRectangleRec((Rectangle){sliderX, sliderY1, sliderW, sliderH}, Fade(GRAY,0.5f));
        float knobX1 = sliderX + musicVolume * sliderW;
        DrawCircle(knobX1, sliderY1 + sliderH/2, 18 * scalefactor, WHITE);

        // Efekt sesi
        DrawText("Efekt Sesi", sliderX, sliderY2 - 80 * scalefactor, ayarFont, WHITE);
        DrawRectangleRec((Rectangle){sliderX, sliderY2, sliderW, sliderH}, Fade(GRAY,0.5f));
        float knobX2 = sliderX + sfxVolume * sliderW;
        DrawCircle(knobX2, sliderY2 + sliderH/2, 18 * scalefactor, WHITE);

        // Ses değerleri
        char musicVal[8], sfxVal[8];
        snprintf(musicVal, sizeof(musicVal), "%d", (int)(musicVolume*100));
        snprintf(sfxVal, sizeof(sfxVal), "%d", (int)(sfxVolume*100));
        DrawText(musicVal, sliderX + sliderW + 20 * scalefactor, sliderY1 - 10 * scalefactor, ayarFont, WHITE);
        DrawText(sfxVal, sliderX + sliderW + 20 * scalefactor, sliderY2 - 10 * scalefactor, ayarFont, WHITE);

        // Ekran modu
        float screenModeY = sliderY2 + 110 * scalefactor;
        const char* modes[] = {"Pencere", "Tam Ekran"};
        int modeFont = (int)(50 * scalefactor);
        const char* modeText = TextFormat("Ekran Modu : %s", modes[screenMode]);
        int modeTextW = MeasureText(modeText, modeFont);
        float modeX = sliderX + sliderW/2 - modeTextW/2;
        float modeY = screenModeY;
        
        // Sabit metin
        const char* prefix = "Ekran Modu : ";
        int prefixW = MeasureText(prefix, modeFont);
        DrawText(prefix, modeX, modeY, modeFont, WHITE);
        
        // Tıklanabilir mod ismi
        float modeNameX = modeX + prefixW;
        int modeNameW = MeasureText(modes[screenMode], modeFont);
        Rectangle modeRect = {modeNameX, modeY, (float)modeNameW, (float)modeFont};
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, modeRect)) {
            DrawText(modes[screenMode], modeNameX, modeY, modeFont, Fade(WHITE, 0.7f));
        } else {
            DrawText(modes[screenMode], modeNameX, modeY, modeFont, WHITE);
        }
    }
}

void UpdatePauseMenu(float scalefactor) {
    // Menü navigasyonu
    if (IsKeyPressed(KEY_UP)) {
        selectedMenuItem = (selectedMenuItem - 1 + menuItemCount) % menuItemCount;
    }
    else if (IsKeyPressed(KEY_DOWN)) {
        selectedMenuItem = (selectedMenuItem + 1) % menuItemCount;
    }
    
    // Menü seçimi
    if (IsKeyPressed(KEY_ENTER)) {
        switch (selectedMenuItem) {
            case 0: // Devam Et
                isPaused = false;
                break;
            case 1: // Ayarlar
                // Ayarlar menüsünde ses seviyesi kontrolü
                if (IsKeyDown(KEY_LEFT)) {
                    musicVolume -= 0.1f;
                    if (musicVolume < 0.0f) musicVolume = 0.0f;
                }
                if (IsKeyDown(KEY_RIGHT)) {
                    musicVolume += 0.1f;
                    if (musicVolume > 1.0f) musicVolume = 1.0f;
                }
                
                // Ekran modu değiştirme
                if (IsKeyPressed(KEY_SPACE)) {
                    screenMode = (screenMode + 1) % 2;
                    if (screenMode == 0) {
                        SetWindowState(FLAG_WINDOW_RESIZABLE);
                        SetWindowSize(1280, 720);
                    } else {
                        ClearWindowState(FLAG_WINDOW_RESIZABLE);
                        SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
                    }
                }
                break;
            case 2: // Ana Menü
                returnToMainMenu = true;
                break;
        }
    }
    
    // Mouse ile tıklama kontrolü
    Vector2 mouse = GetMousePosition();
    float buttonWidth = 400 * scalefactor;
    float buttonHeight = 60 * scalefactor;
    float buttonX = GetScreenWidth()/2 - buttonWidth/2;
    float buttonY_start = 300 * scalefactor;
    float spacing = 30 * scalefactor;
    
    for (int i = 0; i < menuItemCount; i++) {
        float buttonY = buttonY_start + i * (buttonHeight + spacing);
        Rectangle btnRect = {buttonX, buttonY, buttonWidth, buttonHeight};
        
        if (CheckCollisionPointRec(mouse, btnRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectedMenuItem = i;
            switch (i) {
                case 0: // Devam Et
                    isPaused = false;
                    break;
                case 1: // Ayarlar
                    // Ayarlar menüsünde ses seviyesi kontrolü
                    if (IsKeyDown(KEY_LEFT)) {
                        musicVolume -= 0.1f;
                        if (musicVolume < 0.0f) musicVolume = 0.0f;
                    }
                    if (IsKeyDown(KEY_RIGHT)) {
                        musicVolume += 0.1f;
                        if (musicVolume > 1.0f) musicVolume = 1.0f;
                    }
                    break;
                case 2: // Ana Menü
                    returnToMainMenu = true;
                    break;
            }
        }
    }
}

bool ShouldReturnToMainMenu() {
    return returnToMainMenu;
}

void ResetReturnToMainMenu() {
    returnToMainMenu = false;
} 