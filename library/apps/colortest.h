#pragma once
#include <TFT_eSPI.h>
#include "library/device/DeviceProps.h"

extern TFT_eSPI tft;
extern const DeviceProps* device;
extern int screenWidth;
extern int screenHeight;

extern void closePageIfAnyButtonIsPressed();

void onColorTestPageOpen();
void onColorTestPageUpdate();
