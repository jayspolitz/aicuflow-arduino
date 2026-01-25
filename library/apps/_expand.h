#pragma once
#include <TFT_eSPI.h>
#include "library/device/DeviceProps.h"

// TUTORIAL: Check _expand.cpp

// link to global variables
extern TFT_eSPI tft;
extern const DeviceProps* device;
extern int screenWidth;
extern int screenHeight;

// return function for apps in sketch
extern void closePageIfAnyButtonIsPressed();

// declare functions importable
void onPageOpen();
void onPageUpdate();
