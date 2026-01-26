/**
 *  Stream Arduino data to aicuflow AI cloud easily.
 *
 *  Docs:  https://aicuflow.com/docs/library/arduino
 *  From:  20260121-26, by Finn Glas @ AICU GmbH
 *  Board: ESP32 recommended, others work too
 *  Start:
 *        1. Log in and all sensor data is sent to your flow.
 *        2. Device, wifi chip and display is autodetected.
 *        3. Explore cool menus and tools on esp32 devices,
 *           if you use a low-end device, it just measures.
 *  Edit:
 *        1. Find `registerSensors` and add new sensors!
 *        2. Use VS-Code for better code editing!
 *        3. Search `add more` to find customisations!
 */

// === Imports ===
#include <WiFiClientSecure.h>                    // WPA2/3? alt (insecure): #include <WiFiClient.h>
#include "imports/TFT_eSPI/TFT_eSPI.cpp"         // LIB NEED TO INSTALL 
#include "imports/ArduinoJson/ArduinoJson.h"     // LIB NEED TO INSTALL 
#include "library/device/DeviceProps.cpp"        // device detection
#include "library/device/Settings.cpp"           // persistent settings
#include "library/aicuflow/AicuClient.cpp"       // --> client library you can use <--
#include "library/sensors/SensorMeasurement.cpp" // sensor registry for measuring, plots and saving
#include "library/graphics/TFTKeyboard.cpp"      // enter strings using buttons (cool)
#include "library/graphics/PageManager.cpp"      // multi-page app on screens
#include "library/graphics/closeFunctions.cpp"   // for app close timing & recognition
#include "library/apps/core.cpp"                 // import all apps indirectly

// === Setup ===
// TODO CHANGE THESE FACTORY SETTINGS
// or do so later, manually on device
// needed to work
const char* WLAN_SSID = "your-wlan"; // connect to a stable WPA2 Wifi
const char* WLAN_PASS = "your-pass";
const char* AICU_USER = "your-mail"; // register at https://aicuflow.com/signup
const char* AICU_PASS = "your-pass";
const char* PROJ_FLOW = "your-ai-cu-flow-uuid"; // create / select at https://aicuflow.com/flows

// optional options
const char* PROJ_FILE = "esp32";   // will be auto created with .arrow extension
const int VERBOSE = true;
const char* API_URL = "https://prod-backend.aicuflow.com"; // dev or prod
const char* DEVICE_ID_SUFFIX = ""; // 0,1,2,3 appended to id if you have multiple of same kind
const int POINTS_BATCH_SIZE = 64;  // 64 always works, 256 sometimes did, but may be too large.
const int MEASURE_DELAY_MS = 100;  // millisec interval 100ms = 10 per second
const int SCREEN_IDLE_MS = 60000;  // also needs TFT_BL eg 38
const int WIFI_TIMEOUT = 10000;    // 10s, 0 -> blocking till wifi

// === Globals ===
// Do not change these, app needs em
WiFiClientSecure client;             // secure wifi conn
AicuClient aicu(API_URL, VERBOSE);   // aicu api client
TFT_eSPI tft = TFT_eSPI();           // screen stuff
SensorMeasurement sensors("dev");    // sensor registry (->graphs,measure)
const DeviceProps* device = nullptr; // auto device info here
int screenWidth, screenHeight;       // are auto detected
int LEFT_BUTTON, RIGHT_BUTTON;       // are auto detected
bool wifiAvailable = false;          // true once wifi connected

// === Sensors ===
// Register sensors for data collection,
// and sending to the aicuflow cloud.
void registerSensors() {
  
  // Left button = binary sensor
  sensors.registerSensor("left_button", "Button L", 
    [&]() { return digitalRead(LEFT_BUTTON) == 0 ? 1.0 : 0.0; },
    0, 1, TFT_PURPLE, LOG_SEND, SHOW_GRAPH);
  
  // Right button = binary sensor
  sensors.registerSensor("right_button", "Button R",
    [&]() { return digitalRead(RIGHT_BUTTON) == 0 ? 1.0 : 0.0; },
    0, 1, TFT_BLUE, LOG_SEND, SHOW_GRAPH);
  
  // (if wifi) signal strength = analog sensor
  if (device->has_wifi && wifiAvailable)
    sensors.registerSensor(
      "rssi", "RSSI (dBm)", []() { return (double)WiFi.RSSI(); },
      -100, -30, TFT_GREEN, LOG_SEND, SHOW_GRAPH
    );
  
  // Voltage = analog sensor
  sensors.registerSensor("voltage", "Voltage (V)", // 2=ResDiv;3.3RefV;4095-12-bitADCres
    []() { return (analogRead(34)/4095.0)*2.0*3.3*1.1; },
    2.0, 6.0, TFT_RED, LOG_SEND, SHOW_GRAPH);

  // Temperature = analog sensor
  sensors.registerSensor("temperature", "Temp (°C)",
    []() { return temperatureRead(); },
    20, 60, TFT_ORANGE, LOG_SEND, SHOW_GRAPH);
  
  // Free Memory = discrete number "sensor"
  sensors.registerSensor("heapfree", "Heap (KB)",
    []() { return ESP.getFreeHeap() / 1024.0; },
    100, 300, TFT_CYAN, LOG_SEND, SHOW_GRAPH);

  // CPU Speed = discrete number "sensor" (not graphed)
  sensors.registerSensor("cpu_freq_hz", "CPU (MHz)",
    []() { return (double)getCpuFrequencyMhz(); },
    80, 240, TFT_YELLOW, LOG_SEND, HIDE_GRAPH);
  
  // Want to add more sensors?
  //   Parameters: (key, label, readFunc, min, max, color, enabled, showGraph)
  
  //   Ex1) Custom analog sensor
  //      sensors.registerSensor("light", "Light", []() { return analogRead(35) / 4095.0 * 100; }, 0, 100, TFT_WHITE, LOG_SEND, SHOW_GRAPH);
  
  //   Ex2) Not graphed, deactivated sensor
  //      sensors.registerSensor("uptime", "Uptime (s)", []() { return millis() / 1000.0; }, 0, 86400, TFT_MAGENTA, LOG_NONE, HIDE_GRAPH);
}

// === Program Sequence ===
// Only change this if you're sure
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
  
  setupPages(); pageManager->begin();
}
void loop() {
  pageManager->update(); // render apps like measurement
}
