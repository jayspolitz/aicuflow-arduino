#pragma once
#include "imports/TFT_eSPI/TFT_eSPI.h"
#include "library/device/DeviceProps.h"
#include "library/device/Settings.h"

extern TFT_eSPI tft;
extern SensorMeasurement sensors;
extern const DeviceProps* device;
extern int screenWidth;
extern int screenHeight;
extern bool wifiAvailable;
extern const int VERBOSE;
extern int LEFT_BUTTON;
extern int RIGHT_BUTTON;
extern void closePageIfAnyButtonIsPressed();
extern const char* en_de(const char* en, const char* de);

void onVoltmeterPageOpen();
void onVoltmeterPageUpdate();