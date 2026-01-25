#ifndef GAMEOFLIFE_H
#define GAMEOFLIFE_H

#include <TFT_eSPI.h>

extern TFT_eSPI tft;
extern int screenWidth;
extern int screenHeight;
extern int LEFT_BUTTON;
extern int RIGHT_BUTTON;
extern void closePageIfAnyButtonIsPressed();

void onGameOfLifePageOpen();
void onGameOfLifePageUpdate();

#endif