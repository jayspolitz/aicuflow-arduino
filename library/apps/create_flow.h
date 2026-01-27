#pragma once

#include "imports/TFT_eSPI/TFT_eSPI.h"
#include "library/device/DeviceProps.h"
#include "library/device/Settings.h"

extern TFT_eSPI tft;
extern const DeviceProps* device;
extern int screenWidth;
extern int screenHeight;
extern int LEFT_BUTTON;
extern int RIGHT_BUTTON;

extern void connectWifiOrTimeout();
extern void connectAPI();
extern void closePageIfAnyButtonIsPressed();

void onCreateFlowPageOpen();
void onCreateFlowPageUpdate();
