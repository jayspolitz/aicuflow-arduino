#pragma once
#include <TFT_eSPI.h>
#include "library/device/DeviceProps.h"

// use the global ones
extern TFT_eSPI tft;
extern const DeviceProps* device;

void onRandomPageUpdate();
