#ifndef CAMERA_H
#define CAMERA_H

#include <raylib.h>

extern Camera2D camera;

void InitCamera();
void UpdateCameraToPlayer(float mapWidth, float mapHeight);
Camera2D* GetCamera();

#endif // CAMERA_H 