#include <raylib.h>
#include <algorithm>
#include "../../globals.h"
#include "playermov.h"

Camera2D camera;

void InitCamera() {
    camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    camera.target = (Vector2){ player.x, player.y };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void UpdateCameraToPlayer(float mapWidth, float mapHeight) {
    float halfScreenW = GetScreenWidth() / 2.0f;
    float halfScreenH = GetScreenHeight() / 2.0f;
    float targetX = player.x + 64 * scalefactor;
    float targetY = player.y + 64 * scalefactor;
    float minX = halfScreenW;
    float maxX = mapWidth * scalefactor - halfScreenW;
    float minY = halfScreenH;
    float maxY = mapHeight * scalefactor - halfScreenH;
    camera.target.x = std::clamp(targetX, minX, maxX);
    camera.target.y = std::clamp(targetY, minY, maxY);
}

Camera2D* GetCamera() {
    return &camera;
}
