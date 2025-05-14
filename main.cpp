#include <raylib.h>
#include "globals.h"
#include "window_manager.h"
#include "menu.h"
#include "assets/scripts/ep1.h"




extern struct menuState m; // m'yi dışarıdan erişilebilir yap

int main() {
    //Önce pencere modunu ayarlayalım
    InitWindow(WINDOWED_WIDTH, WINDOWED_HEIGHT, TITLE);
    SetTargetFPS(60);

    
    isfullscreen = true;  // Başlangıçta tam ekran
    scalefactor = 1.0f;   // Tam ekran için normal ölçek
    
    if(isfullscreen) {
        RestartWindow(isfullscreen, FULLSCREEN_WIDTH, FULLSCREEN_HEIGHT, TITLE);
        scalefactor = 1.0f;  // Tam ekran için normal ölçek
        m.screenMode = 1;
    } else {
        RestartWindow(isfullscreen, WINDOWED_WIDTH, WINDOWED_HEIGHT, TITLE);
        // Pencere modu için scalefactor'ü ekran boyutuna göre ayarla
        scalefactor = (float)WINDOWED_WIDTH / FULLSCREEN_WIDTH;
        m.screenMode = 0;
    }

    while(!WindowShouldClose() && !shouldExit) {
        float currentWidth = GetScreenWidth();
        float currentHeight = GetScreenHeight();

        // Ekran modu değişikliği kontrolü
        if (shouldToggleFullscreen) {
            isfullscreen = !isfullscreen;  // Mevcut modun tersini al
            if(isfullscreen) {
                RestartWindow(isfullscreen, FULLSCREEN_WIDTH, FULLSCREEN_HEIGHT, TITLE);
                scalefactor = 1.0f;  // Tam ekran için normal ölçek
                m.screenMode = 1;
            } else {
                RestartWindow(isfullscreen, WINDOWED_WIDTH, WINDOWED_HEIGHT, TITLE);
                // Pencere modu için scalefactor'ü ekran boyutuna göre ayarla
                scalefactor = (float)WINDOWED_WIDTH / FULLSCREEN_WIDTH;
                m.screenMode = 0;
            }
            shouldToggleFullscreen = false;  // Değişikliği uyguladıktan sonra sıfırla
        }

        BeginDrawing();
        ClearBackground(BLACK);
        if (fading) {
            // Önce mevcut sahneyi çiz
            if (currentScene == SCENE_MENU)
                DrawMenu(currentWidth, currentHeight);
            else if (currentScene == SCENE_EP1)
                DrawEp1Scene(currentWidth, currentHeight, scalefactor);

            // Fade overlay
            fadeAlpha += fadeDirection * 0.04f; // Hız ayarı
            if (fadeAlpha >= 1.0f) {
                fadeAlpha = 1.0f;
                fadeDirection = -1;
                currentScene = nextScene; // Sahne değiştir
                // Yeni sahneye geçerken başlatıcı fonksiyonu çağır
                if (currentScene == SCENE_EP1) {
                    InitEp1Scene();
                }
            }
            if (fadeAlpha <= 0.0f) {
                fadeAlpha = 0.0f;
                fading = false;
            }
            DrawRectangle(0, 0, currentWidth, currentHeight, Fade(BLACK, fadeAlpha));
        } else {
            // Normal sahne çizimi
            if (currentScene == SCENE_MENU)
                DrawMenu(currentWidth, currentHeight);
            else if (currentScene == SCENE_EP1)
                DrawEp1Scene(currentWidth, currentHeight, scalefactor);
        }
        EndDrawing();
    }

    CleanWindow();
}
