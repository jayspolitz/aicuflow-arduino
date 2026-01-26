#pragma once
#include <TFT_eSPI.h>
#include <esp_wifi.h> // some esp32 boards need: esp_wifi_set_max_tx_power(52); // ca 13dBm
#include "library/graphics/aicuflow_logo_64px.h" // image: aicuflow_logo
#include "library/graphics/aicuflow_logo_wide.h" // image: aicuflow_logo_wide
#include "library/device/DeviceProps.h"
#include "library/device/Settings.h"
#include "library/sensors/SensorMeasurement.h"

// use the global ones
extern TFT_eSPI tft;
extern SensorMeasurement sensors;
extern const DeviceProps* device;
extern int screenWidth;
extern int screenHeight;
extern bool wifiAvailable;
extern const int VERBOSE;
extern int LEFT_BUTTON;
extern int RIGHT_BUTTON;

// function headers
void bootScreen(int duration);
void plotScreen(int duration);
void initSerial();
void initTFTScreen();
void initDeviceGPIOPins();

void initSensorGraphs();