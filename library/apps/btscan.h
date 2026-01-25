#pragma once
#include <TFT_eSPI.h>
#include "library/device/DeviceProps.h"
#include <BLEDevice.h>
#include <BLEScan.h>

extern TFT_eSPI tft;
extern const DeviceProps* device;
extern int screenWidth;
extern int screenHeight;

extern void closePageIfAnyButtonIsPressed();

void onBTPageOpen();
void onBTPageUpdate();
