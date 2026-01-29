#pragma once

#include "imports/TFT_eSPI/TFT_eSPI.h"
#include "library/device/DeviceProps.h"
#include "library/device/Settings.h"
#include <WiFi.h>
#include <esp_system.h>
#include <esp_efuse.h>

extern TFT_eSPI tft;
extern SensorMeasurement sensors;
extern const DeviceProps* device;
extern int screenWidth;
extern int screenHeight;
extern bool wifiAvailable;
extern int LEFT_BUTTON;
extern int RIGHT_BUTTON;
extern void closePageIfAnyButtonIsPressed();

void onHWDiagPageOpen();
void onHWDiagPageUpdate();