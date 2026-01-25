#ifndef WIREFRAME3D_H
#define WIREFRAME3D_H

#include <TFT_eSPI.h>

extern TFT_eSPI tft;
extern int screenWidth;
extern int screenHeight;
extern int LEFT_BUTTON;
extern int RIGHT_BUTTON;
extern void closePageIfAnyButtonIsPressed();

void onWireframe3dPageOpen();
void onWireframe3dPageUpdate();

#endif