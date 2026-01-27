#pragma once
#include "imports/TFT_eSPI/TFT_eSPI.h"
#include <esp_wifi.h> // some esp32 boards need: esp_wifi_set_max_tx_power(52); // ca 13dBm

#include "library/device/DeviceProps.h"
#include "library/device/Settings.h"
#include "library/sensors/SensorMeasurement.h"
#include "library/graphics/TFTMenu.h"
#include "library/graphics/TFTKeyboard.h"
#include "library/graphics/PageManager.h"

// use the global ones
extern TFT_eSPI tft;
extern SensorMeasurement sensors;
extern const DeviceProps* device;
extern int screenWidth;
extern int screenHeight;
extern bool wifiAvailable;
extern const int VERBOSE;
extern const int SCREEN_IDLE_MS;
extern int LEFT_BUTTON;
extern int RIGHT_BUTTON;

// this is so you can define them in the ino sketch
extern void registerSensors();

// vars exported
TFTMenu *mainMenu, *toolsMenu, *gamesMenu, *graphicsMenu, *connectToolsMenu;
TFTMenu *settingsMenu, *wifiMenu, *aicuAPIMenu, *btnSetMenu, *mucToolsMenu;

TFTKeyboard* keyboard;

PageManager* pageManager;

// function headers
void setupPages();
void setupMenus();

void initSensorGraphs();
void onTextConfirmed(String text);
void onMenuPageOpen();
void onKeyboardPageOpen();

void measurementSetup();

// to pass just
// boot.h, measurement.h
void initPoints();
void initDeviceGPIOPins();
void initSerial();
void initTFTScreen();
void bootScreen(int delay);