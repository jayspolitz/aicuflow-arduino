/**
 *  Aicuflow AI Flow Builder + Arduino ESP32 Quickstart
 *
 *  Created: 20260121-26, by Finn Glas @ AICU GmbH
 *
 *  This sketch helps you stream data to the aicuflow platform easily.
 *  Just provide credentials, and all available sensor data is sent.
 *  If the device has a display, it is detected and filled with charts.
 *
 *  Library TFT_eSPI needs Customizing before, sorry! check README.
 *
 *  Tested devices:
 *  - ESP32 ttgo t1 (display);
 *  - liligo t3 s3 t-display (nerdminer case);
 *  - ESP32 devkit c (no display, 2020 build with hall sensor);
 *
 *  Features
 *  - Wifi, if device has that
 *  - Sensor data logging (TFT Screen)
 *  - Non-blocking 2 Core execution (0 measures, 1 talks)
 *
 ******************************************************************
 *
 *  To start, find `registerSensors` and try adding new ones!
 *  Generally, search for "add more" and you find customisations!
 *
 ******************************************************************
 *
 *  Check https://aicuflow.com/docs/library/arduino for more!
 */

// === Imports ===
#include <WiFiClientSecure.h> // alt (insecure): #include <WiFiClient.h>
#include <TFT_eSPI.h>         // LIB NEED TO INSTALL v2.5.43 Bodmer, needs customisation
#include "imports/ArduinoJson/ArduinoJson.h"      // LIB NEED TO INSTALL 
#include "library/device/DeviceProps.cpp"        // device detection
#include "library/device/Settings.cpp"           // persistent settings
#include "library/aicuflow/AicuClient.cpp"       // --> client library you can use <--
#include "library/sensors/SensorMeasurement.cpp" // sensor registry for measuring, plots and saving
#include "library/graphics/TFTKeyboard.cpp"      // enter strings using buttons (cool)
#include "library/graphics/PageManager.cpp"      // multi-page app on screens
#include "library/graphics/closeFunctions.cpp"   // for close timing & recognition
#include "library/apps/core.cpp" // imports all screens indirectly

// === Setup ===
// needed to work
const char* WLAN_SSID = "your-wlan"; // connect to a stable WPA2 Wifi
const char* WLAN_PASS = "your-pass";
const char* AICU_USER = "your-mail"; // register at https://aicuflow.com/signup
const char* AICU_PASS = "your-pass";
const char* PROJ_FLOW = "your-ai-cu-flow-uuid"; // create / select at https://aicuflow.com/flows

// optional options
const char* PROJ_FILE = "esp32"; // will be auto created with .arrow extension
const int VERBOSE = true;
const char* API_URL = "https://prod-backend.aicuflow.com"; // dev or prod
const char* DEVICE_ID_SUFFIX = ""; // 0,1,2,3 appended to id if you have multiple of same kind
const int POINTS_BATCH_SIZE = 64; // 64 always works, 256 sometimes did, but may be too large.
const int MEASURE_DELAY_MS = 100;
const int SCREEN_IDLE_MS = 60000; // also needs TFT_BL eg 38
const int WIFI_TIMEOUT = 10000; // 10s, 0 -> blocking till wifi

// === Globals ===
WiFiClientSecure client;
AicuClient aicu(API_URL, VERBOSE);
TFT_eSPI tft = TFT_eSPI();
SensorMeasurement sensors("dev");

const DeviceProps* device = nullptr;
int screenWidth, screenHeight;
int LEFT_BUTTON, RIGHT_BUTTON;
bool wifiAvailable = false;

// === Sensors ===
void registerSensors() {
  // Register sensors for data collection,
  // and sending to the aicuflow cloud.

  sensors.registerSensor("left_button", "Button L", 
    [&]() { return digitalRead(LEFT_BUTTON) == 0 ? 1.0 : 0.0; },
    0, 1, TFT_PURPLE, LOG_SEND, SHOW_GRAPH);
  
    sensors.registerSensor("right_button", "Button R",
    [&]() { return digitalRead(RIGHT_BUTTON) == 0 ? 1.0 : 0.0; },
    0, 1, TFT_BLUE, LOG_SEND, SHOW_GRAPH);
  
  if (device->has_wifi && wifiAvailable)
    sensors.registerSensor(
      "rssi", "RSSI (dBm)", []() { return (double)WiFi.RSSI(); },
      -100, -30, TFT_GREEN, LOG_SEND, SHOW_GRAPH
    );
  
  sensors.registerSensor("voltage", "Voltage (V)", // 2=ResDiv;3.3RefV;4095-12-bitADCres
    []() { return (analogRead(34)/4095.0)*2.0*3.3*1.1; },
    2.0, 6.0, TFT_RED, LOG_SEND, SHOW_GRAPH);

  sensors.registerSensor("temperature", "Temp (°C)",
    []() { return temperatureRead(); },
    20, 60, TFT_ORANGE, LOG_SEND, SHOW_GRAPH);
  
  sensors.registerSensor("heapfree", "Heap (KB)",
    []() { return ESP.getFreeHeap() / 1024.0; },
    100, 300, TFT_CYAN, LOG_SEND, SHOW_GRAPH);

  sensors.registerSensor("cpu_freq_hz", "CPU (MHz)",
    []() { return (double)getCpuFrequencyMhz(); },
    80, 240, TFT_YELLOW, LOG_SEND, HIDE_GRAPH);
  
  // Want to add more sensors?
  //   Parameters: (key, label, readFunc, min, max, color, enabled, showGraph)
  
  //   Ex1) Custom analog sensor
  //      sensors.registerSensor("light", "Light", []() { return analogRead(35) / 4095.0 * 100; }, 0, 100, TFT_WHITE, LOG_SEND, SHOW_GRAPH);
  
  //   Ex2) Not graphed, deactivated sensor
  //      sensors.registerSensor("uptime", "Uptime (s)", []() { return millis() / 1000.0; }, 0, 86400, TFT_MAGENTA, LOG_NONE, HIDE_GRAPH);
  
  if (VERBOSE) {
    Serial.print("Registered sensors: ");
    Serial.println(sensors.getEnabledCount());
  }
}

// === Program Sequence ===
void setup() {
  device = &getDeviceProps();

  initPoints();
  initDeviceGPIOPins();
  initSerial();
  loadSettings();
  applySettings();
  
  if (!device->has_display) delay(1000);
  if (device->has_display)  initTFTScreen();
  if (device->has_display)  bootScreen(3000);
  if (device->has_display)  setupMenus();
  
  setupPages();

  pageManager->setDefaultPage(device->has_display ? "menu" : "measure");
  pageManager->begin();
}
void loop() {
  pageManager->update(); // render menus, pages / apps
}
