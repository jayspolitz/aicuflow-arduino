#ifndef MANDELBROT_H
#define MANDELBROT_H

#include "imports/TFT_eSPI/TFT_eSPI.h"

extern TFT_eSPI tft;
extern int screenWidth;
extern int screenHeight;
extern int LEFT_BUTTON;
extern int RIGHT_BUTTON;
extern void closePageIfAnyButtonIsPressed();

void onMandelbrotPageOpen();
void onMandelbrotPageUpdate();

#endif