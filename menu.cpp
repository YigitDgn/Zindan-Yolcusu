#include "menu.h"
#include "globals.h"
#include <raylib.h>
#include <cstdio>
#include "assets/scripts/ep1.h" 
#include <stdio.h>



menuState m {
    .showSettings = false,
    .language = 0,
    .musicVolume = 1.0f,
    .sfxVolume = 1.0f,
    .screenMode = 0,  // 0: Pencere modu, 1: Tam ekran
    .showPlayPanel = false,
    .playPanelJustOpened = false
};

void InitializeMenu() {
    m.showSettings = false;
    m.language = 0;
    m.musicVolume = 0.7f;
    m.sfxVolume = 1.0f;
    m.screenMode = 0;
    m.showPlayPanel = false;
    m.playPanelJustOpened = false;
}

void UpdateMenu(float currentWidth,float currentHeight) {
    float settingScale = currentWidth / 1920.0f;
}

void DrawMenu(float currentWidth, float currentHeight) {
    // Menü arka planı (menu.png)
    DrawTexturePro(menu, 
        (Rectangle){0,0,(float)menu.width,(float)menu.height},
        (Rectangle){0,0,currentWidth,currentHeight},
        (Vector2){0,0},0.0f,WHITE);

    // Header görseli (üstte ortalanmış)
    float headerWidth = header.width * scalefactor;
    float headerHeight = header.height * scalefactor;
    float headerX = currentWidth/2 - headerWidth/2;
    float headerY = 40 * scalefactor;
    DrawTexturePro(header,
        (Rectangle){0,0,(float)header.width,(float)header.height},
        (Rectangle){headerX,headerY,headerWidth,headerHeight},
        (Vector2){0,0},0.0f,WHITE);

    // Sağda alt alta üç buton
    const char* buttonLabels[3] = {"Oyna", "Ayarlar", "Ayril"};
    float buttonWidth = selectionpanel.width * scalefactor;
    float buttonHeight = selectionpanel.height * scalefactor;
    float spacing = 30 * scalefactor; // Butonlar arası boşluk
    float buttonX = currentWidth - buttonWidth - 40 * scalefactor; // Sağdan 40px boşluk
    float buttonY_start = currentHeight/2 - (buttonHeight * 1.5f + spacing);
    for(int i=0; i<3; i++) {
        float buttonY = buttonY_start + i * (buttonHeight + spacing);
        Rectangle btnRect = {buttonX, buttonY, buttonWidth, buttonHeight};

        Vector2 mouse = GetMousePosition();
        if (!m.showSettings && !m.showPlayPanel && CheckCollisionPointRec(mouse, btnRect)) {
            DrawRectangleRec(btnRect, Fade(WHITE, 0.2f));
        }

        DrawTexturePro(selectionpanel,
            (Rectangle){0,0,(float)selectionpanel.width,(float)selectionpanel.height},
            (Rectangle){buttonX,buttonY,buttonWidth,buttonHeight},
            (Vector2){0,0},0.0f,WHITE);
        int fontSize = (int)(buttonHeight * 0.4f);
        int textWidth = MeasureText(buttonLabels[i], fontSize);
        int textX = buttonX + buttonWidth/2 - textWidth/2;
        int textY = buttonY + buttonHeight/2 - fontSize/2;
        DrawText(buttonLabels[i], textX, textY, fontSize, WHITE);

        // Oyna ve Ayril butonları sadece ayarlar menüsü ve play panel kapalıysa tıklanabilir
        if (!m.showSettings && !m.showPlayPanel) {
            // Oyna butonuna tıklama
            if (i == 0 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mouse, btnRect)) {
                    m.showPlayPanel = true;
                    m.playPanelJustOpened = true;
                }
            }
            // Ayarlar butonuna tıklama
            if (i == 1 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mouse, btnRect)) {
                    m.showSettings = true;
                }
            }
            // Ayril butonuna tıklama
            if (i == 2 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mouse, btnRect)) {
                    shouldExit = true;
                }
            }
        }
    }

    // Ayarlar menüsü açıkken ortada yarı saydam kutu ve 'Geri' butonu
    if (m.showSettings) {
        // Referans çözünürlük: 1920x1080
        float refSettingsW = 1056.0f; // 1920 * 0.55
        float refSettingsH = 702.0f;  // 1080 * 0.65
        float settingsW = refSettingsW * scalefactor;
        float settingsH = refSettingsH * scalefactor;
        float settingsX = currentWidth/2 - settingsW/2;
        float settingsYOffset = 60 * scalefactor; // Tüm içeriği aşağı kaydırmak için offset
        float settingsY = currentHeight/2 - settingsH/2 + settingsYOffset;
        DrawRectangleRounded((Rectangle){settingsX, settingsY, settingsW, settingsH}, 0.1f, 16, Fade(BLACK, 0.8f));
        DrawRectangleLinesEx((Rectangle){settingsX, settingsY, settingsW, settingsH}, 4, WHITE);
        int titleFont = (int)(76 * scalefactor); // 1080*0.07
        const char* ayarlarText = "Ayarlar";
        int titleWidth = MeasureText(ayarlarText, titleFont);
        DrawText(ayarlarText, settingsX + settingsW/2 - titleWidth/2, settingsY + 30 * scalefactor, titleFont, WHITE);
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
        // Geri butonuna tıklama
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, geriBtn)) {
            m.showSettings = false;
        }

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
        float knobX1 = sliderX + m.musicVolume * sliderW;
        DrawCircle(knobX1, sliderY1 + sliderH/2, 18 * scalefactor, WHITE);
        DrawText("Efekt Sesi", sliderX, sliderY2 - 80 * scalefactor, ayarFont, WHITE);
        DrawRectangleRec((Rectangle){sliderX, sliderY2, sliderW, sliderH}, Fade(GRAY,0.5f));
        float knobX2 = sliderX + m.sfxVolume * sliderW;
        DrawCircle(knobX2, sliderY2 + sliderH/2, 18 * scalefactor, WHITE);
        // --- Kaydırıcı işlevi ---
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            // Müzik
            Rectangle sliderRect1 = {sliderX, sliderY1, sliderW, sliderH + 20 * scalefactor};
            if (CheckCollisionPointRec(mouse, sliderRect1)) {
                m.musicVolume = (mouse.x - sliderX) / sliderW;
                if (m.musicVolume < 0) m.musicVolume = 0;
                if (m.musicVolume > 1) m.musicVolume = 1;
            }
            // Efekt
            Rectangle sliderRect2 = {sliderX, sliderY2, sliderW, sliderH + 20 * scalefactor};
            if (CheckCollisionPointRec(mouse, sliderRect2)) {
                m.sfxVolume = (mouse.x - sliderX) / sliderW;
                if (m.sfxVolume < 0) m.sfxVolume = 0;
                if (m.sfxVolume > 1) m.sfxVolume = 1;
            }
        }
        // --- Değerleri sayıyla göster ---
        char musicVal[8], sfxVal[8];
        snprintf(musicVal, sizeof(musicVal), "%d", (int)(m.musicVolume*100));
        snprintf(sfxVal, sizeof(sfxVal), "%d", (int)(m.sfxVolume*100));
        DrawText(musicVal, sliderX + sliderW + 20 * scalefactor, sliderY1 - 10 * scalefactor, ayarFont, WHITE);
        DrawText(sfxVal, sliderX + sliderW + 20 * scalefactor, sliderY2 - 10 * scalefactor, ayarFont, WHITE);

        // --- Ekran Modu Seçimi ---
        float screenModeY = sliderY2 + 110 * scalefactor;
        // Ekran modu seçici
        const char* modes[] = {"Pencere", "Tam Ekran"};
        int modeFont = (int)(50 * scalefactor);
        const char* modeText = TextFormat("Ekran Modu : %s", modes[m.screenMode]);
        int modeTextW = MeasureText(modeText, modeFont);
        float modeX = sliderX + sliderW/2 - modeTextW/2;
        float modeY = screenModeY;
        // Sabit metin
        const char* prefix = "Ekran Modu : ";
        int prefixW = MeasureText(prefix, modeFont);
        DrawText(prefix, modeX, modeY, modeFont, WHITE);
        // Tıklanabilir mod ismi
        float modeNameX = modeX + prefixW;
        int modeNameW = MeasureText(modes[m.screenMode], modeFont);
        Rectangle modeRect = {modeNameX, modeY, (float)modeNameW, (float)modeFont};
        if (CheckCollisionPointRec(mouse, modeRect)) {
            DrawText(modes[m.screenMode], modeNameX, modeY, modeFont, Fade(WHITE, 0.7f));
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                m.screenMode = (m.screenMode + 1) % 2;  // 0 ve 1 arasında değiştir
                shouldToggleFullscreen = true;
            }
        } else {
            DrawText(modes[m.screenMode], modeNameX, modeY, modeFont, WHITE);
        }
    }

    // Oyna paneli
    if (m.showPlayPanel) {
        float panelW = currentWidth * 0.45f;
        float panelH = currentHeight * 0.38f;
        float panelX = currentWidth/2 - panelW/2;
        float panelY = currentHeight/2 - panelH/2;
        DrawRectangleRounded((Rectangle){panelX, panelY, panelW, panelH}, 0.1f, 16, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx((Rectangle){panelX, panelY, panelW, panelH}, 4, WHITE);
        int titleFont = (int)(panelH * 0.18f);
        const char* panelTitle = "Oyun Seçimi";
        int titleWidth = MeasureText(panelTitle, titleFont);
        DrawText(panelTitle, panelX + panelW/2 - titleWidth/2, panelY + 30 * scalefactor, titleFont, WHITE);
        // Butonlar
        float btnW = panelW * 0.7f;
        float btnH = 60 * scalefactor;
        float btnX = panelX + panelW/2 - btnW/2;
        float btnY1 = panelY + panelH/2 - btnH - 10 * scalefactor;
        float btnY2 = panelY + panelH/2 + 10 * scalefactor;
        Rectangle yeniBtn = {btnX, btnY1, btnW, btnH};
        Rectangle devamBtn = {btnX, btnY2, btnW, btnH};
        Vector2 mouse = GetMousePosition();
        // Hover efektleri
        if (CheckCollisionPointRec(mouse, yeniBtn)) {
            DrawRectangleRec(yeniBtn, Fade(WHITE, 0.2f));
        }
        if (CheckCollisionPointRec(mouse, devamBtn)) {
            DrawRectangleRec(devamBtn, Fade(WHITE, 0.2f));
        }
        DrawRectangleRounded(yeniBtn, 0.3f, 8, Fade(GRAY, 0.7f));
        DrawRectangleRounded(devamBtn, 0.3f, 8, Fade(GRAY, 0.7f));
        DrawRectangleLinesEx(yeniBtn, 2, WHITE);
        DrawRectangleLinesEx(devamBtn, 2, WHITE);
        int btnFont = (int)(btnH * 0.5f);
        const char* yeniText = "Yeni Oyun";
        const char* devamText = "Devam Et";
        int yeniTextW = MeasureText(yeniText, btnFont);
        int devamTextW = MeasureText(devamText, btnFont);
        DrawText(yeniText, btnX + btnW/2 - yeniTextW/2, btnY1 + btnH/2 - btnFont/2, btnFont, WHITE);
        DrawText(devamText, btnX + btnW/2 - devamTextW/2, btnY2 + btnH/2 - btnFont/2, btnFont, WHITE);
        // Butonlara tıklama
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (m.playPanelJustOpened) {
                wprintf(L"[DEBUG] Oyna paneli yeni açıldı, ilk tıklama yoksayılıyor.\n");
                m.playPanelJustOpened = false;
            } else {
                bool buttonClicked = false;
                if (CheckCollisionPointRec(mouse, yeniBtn)) {
                    printf("[DEBUG] 'Yeni Oyun' butonuna tıklandı!\n");
                    m.showPlayPanel = false;
                    // Fade ile sahne geçişi başlat
                    nextScene = SCENE_EP1;
                    fadeAlpha = 0.0f;
                    fadeDirection = 1;
                    fading = true;
                    buttonClicked = true;
                    // Yeni oyunu başlat
                    StartNewGame();
                }
                if (CheckCollisionPointRec(mouse, devamBtn)) {
                    wprintf(L"[DEBUG] 'Devam Et' butonuna tıklandı!\n");
                    m.showPlayPanel = false;
                    // Buraya devam etme kodu eklenebilir
                    buttonClicked = true;
                }
                if (!buttonClicked && !CheckCollisionPointRec(mouse, (Rectangle){panelX, panelY, panelW, panelH})) {
                    wprintf(L"[DEBUG] Panel dışında bir yere tıklandı, panel kapatılıyor.\n");
                    m.showPlayPanel = false;
                }
            }
        }
    }
}
    
    
    
