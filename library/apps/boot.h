#pragma once
#include <TFT_eSPI.h>
#include "library/graphics/aicuflow_logo_64px.h" // image: aicuflow_logo
#include "library/graphics/aicuflow_logo_wide.h" // image: aicuflow_logo_wide
#include "library/device/DeviceProps.h"
#include "library/device/Settings.h"

// use the global ones
extern TFT_eSPI tft;
extern const DeviceProps* device;
extern int screenWidth;
extern int screenHeight;
extern const int VERBOSE;
extern int LEFT_BUTTON;
extern int RIGHT_BUTTON;

// function headers
void bootScreen(int duration);
void plotScreen(int duration);
void initSerial();
void initTFTScreen();
void initDeviceGPIOPins();
