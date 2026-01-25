#pragma once
#include <TFT_eSPI.h>
#include "library/graphics/aicuflow_logo_64px.h" // image: aicuflow_logo
#include "library/graphics/aicuflow_logo_wide.h" // image: aicuflow_logo_wide
#include "library/device/DeviceProps.h"

// use the global ones
extern TFT_eSPI tft;
extern const DeviceProps* device;
extern int screenWidth;
extern int screenHeight;

// function headers
void bootScreen(int duration);
void plotScreen(int duration);
