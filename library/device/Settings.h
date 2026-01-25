#pragma once
#include <Preferences.h>  // ESP32 built-in lib
#include "library/device/DeviceProps.h"

// use the global ones
extern const DeviceProps* device;
extern const int VERBOSE;

// import factory settings
extern const char* PROJ_FILE;
extern const char* DEVICE_ID_SUFFIX;