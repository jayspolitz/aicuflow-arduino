#pragma once
#include "imports/TFT_eSPI/TFT_eSPI.h"
#include "library/device/DeviceProps.h"

// use the global ones
extern TFT_eSPI tft;
extern const DeviceProps* device;

void onRandomPageUpdate();
