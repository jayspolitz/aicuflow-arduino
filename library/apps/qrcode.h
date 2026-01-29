#pragma once
#include "imports/TFT_eSPI/TFT_eSPI.h"
#include "library/device/DeviceProps.h"
#include "library/device/Settings.h"
#include "imports/qrcode/qrcode.c"

extern TFT_eSPI tft;
extern const DeviceProps* device;
extern int screenWidth;
extern int screenHeight;
extern bool wifiAvailable;
extern int LEFT_BUTTON;
extern int RIGHT_BUTTON;

extern void closePageIfAnyButtonIsPressed();

void onQRCodePageOpen();
void onQRCodePageUpdate();
