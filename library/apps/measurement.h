#pragma once
#include "imports/TFT_eSPI/TFT_eSPI.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "imports/ArduinoJson/ArduinoJson.h"
#include "library/device/DeviceProps.h"
#include "library/device/Settings.h"
#include "library/sensors/SensorMeasurement.h"
#include "library/aicuflow/AicuClient.h"

// link to global variables
extern TFT_eSPI tft;
extern SensorMeasurement sensors;
extern AicuClient aicu;
extern WiFiClientSecure client;
extern const DeviceProps* device;

extern int screenWidth;
extern int screenHeight;
extern bool wifiAvailable;
extern const int WIFI_TIMEOUT;
extern const int MEASURE_DELAY_MS;
extern const int POINTS_BATCH_SIZE;
extern int LEFT_BUTTON;
extern int RIGHT_BUTTON;

// declare functions importable
void connectWifiOrTimeout();
void connectAPI();
void initPoints();
void onMeasurePageUpdate();