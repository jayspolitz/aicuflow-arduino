#ifndef SNAKEGAME_H
#define SNAKEGAME_H

#include <TFT_eSPI.h>

extern TFT_eSPI tft;
extern int screenWidth;
extern int screenHeight;
extern int LEFT_BUTTON;
extern int RIGHT_BUTTON;

void onSnakeGamePageOpen();
void onSnakeGamePageUpdate();

#endif