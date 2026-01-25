#pragma once
#include <TFT_eSPI.h>

// use the global ones
extern TFT_eSPI tft;
extern const decltype(getDeviceProps())& device;

void onRandomPageUpdate();
