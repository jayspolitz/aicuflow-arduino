#pragma once
#ifndef SETTINGS_H
#include <Preferences.h>  // ESP32 built-in lib
#include "library/device/DeviceProps.h"
#include "library/sensors/SensorMeasurement.h"

// use the global ones
extern const DeviceProps* device;
extern SensorMeasurement sensors;

// import factory settings
extern const char* WLAN_SSID;
extern const char* WLAN_PASS;
extern const char* AICU_USER;
extern const char* AICU_PASS;
extern const char* PROJ_FLOW;

extern const char* PROJ_FILE;
extern const char* DEVICE_ID_SUFFIX;

void loadSettings();
void saveSettings();
void clearSettings();
void applySettings();

const char* en_de(const char* en, const char* de);

#endif //SETTINGS_H