/**
 *  Aicuflow AI Flow Builder + Arduino ESP32 Quickstart
 *
 *  Created: 20260121-24, by Finn Glas @ AICU GmbH
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
 *  To start, find `registerAllSensors` and try adding new ones!
 *
 *  Check https://aicuflow.com/docs/library/arduino for more!
 */

/* AVAILABLE TFT COLORS ===
 * TFT_BLACK, TFT_NAVY, TFT_DARKGREEN, TFT_DARKCYAN, TFT_MAROON,
 * TFT_PURPLE, TFT_OLIVE, TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLUE,
 * TFT_GREEN, TFT_CYAN, TFT_RED, TFT_MAGENTA, TFT_YELLOW, TFT_WHITE,
 * TFT_ORANGE, TFT_GREENYELLOW, TFT_PINK === */

#include <TFT_eSPI.h> // v2.5.43 Bodmer, needs customisation
#include <SPI.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_wifi.h>     // some boards need: esp_wifi_set_max_tx_power(52); // ca 13dBm
#include <HTTPClient.h>
#include <ArduinoJson.h>  // 7.4.2 by Benoit
#include <Preferences.h>  // ESP32 built-in lib

#include "library/device/DeviceProps.cpp"
#include "library/aicuflow/AicuClient.cpp"
#include "library/sensors/SensorMeasurement.cpp"
#include "library/graphics/TFTKeyboard.cpp"
#include "library/graphics/aicuflow_logo_64px.h" // aicuflow_logo
#include "library/graphics/aicuflow_logo_wide.h" // aicuflow_logo_wide
#include "library/graphics/PageManager.cpp"

#include "library/apps/about.cpp"

//#settings: empty, REPLACE THIS
const char* WLAN_SSID = "your-wlan"; // connect to a stable WPA2 Wifi
const char* WLAN_PASS = "your-pass";
const char* AICU_USER = "your-mail"; // register at https://aicuflow.com/signup
const char* AICU_PASS = "your-pass";
const char* PROJ_FLOW = "your-ai-cu-flow-uuid"; // create one at https://aicuflow.com
const char* PROJ_FILE = "esp32.arrow"; // will be auto created

const int VERBOSE = true;
const char* API_URL = "https://prod-backend.aicuflow.com"; // dev or prod
const char* DEVICE_ID = "aicu0"; // if you have multiple devices
const int POINTS_BATCH_SIZE = 64; // 64 always works, 256 sometimes did, but may be too large.
const int MEASURE_DELAY_MS = 100;
const int SCREEN_IDLE_MS = 30000; // also needs TFT_BL eg 38
const int WIFI_TIMEOUT = 10000; // 10s, 0 -> blocking till wifi
//#endsettings: empty, REPLACE THIS

// globals
WiFiClientSecure client;
AicuClient aicu(API_URL, VERBOSE);
TFT_eSPI tft = TFT_eSPI();
SensorMeasurement sensors(DEVICE_ID);

const DeviceProps& device = getDeviceProps();
int screenWidth, screenHeight;
int LEFT_BUTTON, RIGHT_BUTTON;
static bool wifiAvailable = false;

// page IDs
const char* PAGE_MENU = "menu";
const char* PAGE_MEASURE = "measure";
const char* PAGE_ABOUT = "about";
const char* PAGE_RANDOM = "random";
const char* PAGE_KEYBOARD = "keyboard";

// settings
const char* PREFS = "aicu-settings";
Preferences preferences;
String deviceName = device.kind_short;
const char* PREF_DNAME = "deviceName";
void loadSettings() {
  preferences.begin(PREFS, false); // false = r/w mode
  deviceName = preferences.getString(PREF_DNAME, deviceName);
  preferences.end();
  if (VERBOSE) {
    Serial.println("Loaded pref name: " + deviceName);
  }
}
void saveSettings() {
  preferences.begin(PREFS, false); // false = r/w mode
  preferences.putString(PREF_DNAME, deviceName);
  preferences.end();
  if (VERBOSE) Serial.println("Prefs saved to mem!");
}
void clearSettings() {
  preferences.begin(PREFS, false);
  preferences.clear();
  preferences.end();
  Serial.println("Prefs cleared!");
}

// Menu & UI
TFTMenu *mainMenu, *actionsMenu, *settingsMenu;
TFTKeyboard* keyboard;
PageManager* pageManager;
// Auto return
void checkButtonReturn() {
  if (digitalRead(LEFT_BUTTON) == LOW || digitalRead(RIGHT_BUTTON) == LOW) {
    pageManager->returnToPrevious();
  }
}
void onMenuPageOpen() {
  tft.fillScreen(TFT_BLACK);
  TFTMenu* prevMenu = pageManager->getPreviousMenu();
  (prevMenu ? prevMenu : mainMenu)->begin();
}
// Keyboard
void onKeyboardPageOpen() {
  keyboard->setText(deviceName);
  tft.fillScreen(TFT_BLACK);
  keyboard->begin();
}
void onTextConfirmed(String text) {
  int context = pageManager->getContext<int>();
  if (context == 1) deviceName = text; // 1 = CONTEXT_DEVICE_NAME
  saveSettings();
  if(VERBOSE) Serial.println("Saved: " + text);
  pageManager->returnToPrevious();
  pageManager->blockInputFor(300);
}

// measurement: json collect & stream, dtype
static DynamicJsonDocument points(384*POINTS_BATCH_SIZE*2);
static JsonArray arr;
static uint16_t count = 0;
static int32_t PERIOD_US = MEASURE_DELAY_MS * 1000UL;
void initPoints() {
  points.clear();
  arr = points.to<JsonArray>();
  count = 0;
}
struct JsonBatch { DynamicJsonDocument* doc; };
QueueHandle_t flushQueue; // Async queue for sending
void flushSamplesAsync() {
  if (count == 0) return;
  DynamicJsonDocument* batch =
    new DynamicJsonDocument(points.memoryUsage() + 384 * POINTS_BATCH_SIZE * 2);
  *batch = points; // deep copy
  JsonBatch jb{ batch };
  if (xQueueSend(flushQueue, &jb, 0) != pdTRUE) {
    delete batch; // drop if queue full
  }
  points.clear();
  arr = points.to<JsonArray>();
  count = 0;
}
void addSampleAndAutoSend() {
  JsonObject obj = arr.createNestedObject();
  sensors.toJson(obj);
  if (++count >= POINTS_BATCH_SIZE) flushSamplesAsync();
}
void sendTask(void* parameter) {
  // async Send Task (Core 1 on ESP32)
  // this is used so we don't see the graph stutter
  JsonBatch jb;
  for (;;) {
    if (xQueueReceive(flushQueue, &jb, portMAX_DELAY) == pdTRUE) {
      aicu.sendTimeseriesPoints(PROJ_FLOW, PROJ_FILE, *jb.doc);
      delete jb.doc; // free memory
    }
  }
}

// tft display screens
void bootScreen(int duration=1000) {
  tft.setRotation(0); // vertical
  tft.setCursor(0, 0);
  tft.fillScreen(TFT_BLACK); // aicu logo
  tft.pushImage(screenWidth/2-126/2, screenHeight/2-14, 126, 28, aicuflow_logo_wide);
  delay(duration);
}
void plotScreen(int duration=1000) {
  tft.setRotation(0); // vertical
  tft.setCursor(0, 0);
  tft.fillScreen(TFT_BLACK); // aicu logo
  int topoffset = (device.kind_slug == "lilygo-t-display-s3") ? 6 : 0;
  tft.pushImage(screenWidth/2-126/2, topoffset, 126, 28, aicuflow_logo_wide);
  tft.setCursor(0, 32);
  if (device.kind_slug == "lilygo-t-display-s3") tft.println("");
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // details
  tft.println("Aicu IoT-AI Endpoint");
  tft.println("https://aicuflow.com");
  tft.println("");
  tft.println("");
  tft.println("Device started...");
  delay(duration);
}
void setupMenus() {
  loadSettings();
  
  // actions
  actionsMenu = new TFTMenu(&tft, "Actions");
  actionsMenu->addBackItem();
  actionsMenu->setColors(TFT_BLACK, TFT_DARKGREEN, TFT_WHITE, TFT_WHITE);
  actionsMenu->addItem("Random", []() { pageManager->openPage(PAGE_RANDOM); });
  
  // settings
  settingsMenu = new TFTMenu(&tft, "Settings");
  settingsMenu->addBackItem();
  settingsMenu->setColors(TFT_BLACK, TFT_DARKGREEN, TFT_WHITE, TFT_WHITE);
  settingsMenu->addItem("Device Name", []() { pageManager->openPage(PAGE_KEYBOARD, (void*)1); }); // 1 = device name context
  settingsMenu->addItem("About", []() { pageManager->openPage(PAGE_ABOUT); });
  
  // main
  mainMenu = new TFTMenu(&tft, "Aicuflow IoT");
  mainMenu->addItem("Start", []() { pageManager->openPage(PAGE_MEASURE); });
  mainMenu->addSubmenu("Actions", actionsMenu);
  mainMenu->addSubmenu("Settings", settingsMenu);
  
  // keyboard
  keyboard = new TFTKeyboard(&tft, "Enter Text");
  keyboard->setButtonPins(LEFT_BUTTON, RIGHT_BUTTON);
  keyboard->setOnConfirm(onTextConfirmed);
  
  // after all propagate
  mainMenu->propagateButtonPins(LEFT_BUTTON, RIGHT_BUTTON);
}

// connecting
void connectWifiOrTimeout() {
  if (device.has_display) tft.print("Connecting to wifi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  // try to connect (until timeout)
  unsigned long start = millis();
  while (
    (WiFi.status() != WL_CONNECTED) // not connected
    && (!WIFI_TIMEOUT || (millis() - start < WIFI_TIMEOUT)) // timeout
  ) {
    delay(500);
    if (device.has_display) tft.print("."); // Loading progress
  }

  // no connection = off
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    wifiAvailable = false;
    tft.print("\n");
    tft.println("No wifi connection!");
    return;
  } else { // connected = on
    wifiAvailable = true;
  }

  // set up wifi stuff
  client.setInsecure(); // TLS workaround
  aicu.setWiFiClient(&client);
  
  // print
  if (wifiAvailable && device.has_display) {
    tft.print("\n");
    tft.println("WiFi Connected!");
    tft.print("IP: "); 
    tft.println(WiFi.localIP());
    tft.println("");
  } else if (device.has_display) tft.print("\n");
  Serial.print("Wifi-Mac: "); Serial.println(WiFi.macAddress());
  delay(500);
}
void connectAPI() {
  if (device.has_display) tft.println("Connecting API...");

  if (!aicu.login(AICU_USER, AICU_PASS)) {
    if (device.has_display) tft.println("Auth failed! :/");
    Serial.println("Login failed!");
    return;
  } else {
    if (device.has_display) tft.println("API connected!");
    Serial.println("Login success!");
  }
}

void registerAllSensors() {
  // registerSensor(key, label, readFunc, min, max, color, enabled, showGraph)
  sensors.registerSensor("left_button", "Button L", 
    [&]() { return digitalRead(LEFT_BUTTON) == 0 ? 1.0 : 0.0; },
    0, 1, TFT_PURPLE, LOG_SEND, SHOW_GRAPH);
  sensors.registerSensor("right_button", "Button R",
    [&]() { return digitalRead(RIGHT_BUTTON) == 0 ? 1.0 : 0.0; },
    0, 1, TFT_BLUE, LOG_SEND, SHOW_GRAPH);
  if (device.has_wifi) sensors.registerSensor("rssi", "RSSI (dBm)",
    []() { return (double)WiFi.RSSI(); },
    -100, -30, TFT_GREEN, LOG_SEND, SHOW_GRAPH);
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
  
  // More sensors?
  // Ex1) Custom analog sensor
  // sensors.registerSensor("light", "Light",
  //  []() { return analogRead(35) / 4095.0 * 100; },
  //  0, 100, TFT_WHITE, LOG_SEND, SHOW_GRAPH);
  
  // Ex2) Not graphed sensor
  // sensors.registerSensor("uptime", "Uptime (s)",
  //  []() { return millis() / 1000.0; },
  //  0, 86400, TFT_MAGENTA, LOG_SEND, HIDE_GRAPH);
  
  // Ex3) Disabled sensor
  // sensors.registerSensor("unused", "Unused",
  //  []() { return 0.0; },
  //  0, 1, TFT_WHITE, LOG_NONE, HIDE_GRAPH);

  if (VERBOSE) {
    Serial.print("Registered sensors: ");
    Serial.println(sensors.getEnabledCount());
  }
}
void initSensorGraphs() {
  if (device.kind_slug == "lilygo-t-display-s3")
    sensors.setGraphSpacing(22, 5);
  else sensors.setGraphSpacing(14, 3);
  tft.println("Measuring...");
  sensors.initGraphs(&tft, screenWidth, screenHeight);
  if (VERBOSE) {
    Serial.print("Graphed sensors: ");
    Serial.println(sensors.getGraphCount());
  }
}

// Measurement page handlers
void onMeasurePageOpen() {
  if (device.has_display)  plotScreen(1000);
  if (device.has_wifi)     connectWifiOrTimeout();
  if (device.has_wifi && wifiAvailable) connectAPI();

  registerAllSensors();
  if (device.has_display)  initSensorGraphs();
  
  if (device.has_wifi) {
    flushQueue = xQueueCreate(8, sizeof(JsonBatch));
    xTaskCreatePinnedToCore(sendTask, "sendTask", 8192, NULL, 1, NULL, 1);
  }
}
void onMeasurePageUpdate() {
  // Precise time
  static uint32_t next = micros();
  uint32_t now = micros();
  if ((int32_t)(now - next) < 0) return;
  next += PERIOD_US;

  // Measure Data
  sensors.measure();

  // Forward Data
  if (VERBOSE && !device.has_display)    sensors.printValues(); // May be BLOCKING!
  if (pageManager->screenAwake && device.has_display) // save energy
                                         sensors.updateGraphs();
  if (wifiAvailable && device.has_wifi)  addSampleAndAutoSend();
}



// Random Page
void onRandomPageUpdate() {
  int w = tft.width(), h = tft.height();
  // Seed randomness (ESP32 has HW RNG, this mixes it)
  randomSeed(esp_random());
  tft.startWrite();
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      tft.drawPixel(x, y, random(0xFFFF)); // full 16-bit RGB565 space
    }
  }
  tft.endWrite();
  // Footer (explicit positioning)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(4, h - 10);
  tft.print("Press any key");
}

// initing
void initDeviceGPIOPins() {
  // specifics
  if (device.kind_slug == "esp32-ttgo-t1") {
    // voltage divider something
    pinMode(14, OUTPUT);
    digitalWrite(14, HIGH);

    // buttons
    LEFT_BUTTON = 0;
    RIGHT_BUTTON = 35;
    pinMode(LEFT_BUTTON, INPUT_PULLUP);
    pinMode(RIGHT_BUTTON, INPUT);
  } else if (device.kind_slug == "lilygo-t-display-s3") {
    // buttons
    LEFT_BUTTON = 0;
    RIGHT_BUTTON = 14;
    pinMode(LEFT_BUTTON, INPUT_PULLUP);
    pinMode(RIGHT_BUTTON, INPUT);

    // power up
    esp_wifi_set_max_tx_power(52); // ca 13dBm
  }
}
void initSerial() {
  Serial.begin(115200); // Serial.begin(0); would try to detect
  // if (Serial.baudRate() == 0) // no rate detected
  while (!Serial && millis() < 2000) delay(10);
  if (VERBOSE) Serial.println("Hello World!");
  delay(500);
  Serial.print("Aicuflow booted on "); Serial.print(device.kind_slug); Serial.println("!");
}
void initTFTScreen() {
  screenWidth  = tft.width();
  screenHeight = tft.height();
  if (VERBOSE) Serial.print("Width: "); Serial.println(screenWidth);   // ttgo-t1: 135 
  if (VERBOSE) Serial.print("Height: "); Serial.println(screenHeight); // ttgo-t1: 240
  tft.init();
  
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH); // screen on
}
void setupPageManager() {
  pageManager = new PageManager(&tft, LEFT_BUTTON, RIGHT_BUTTON, SCREEN_IDLE_MS);
  pageManager->registerPage(PAGE_MENU, onMenuPageOpen, []() { mainMenu->update(); })
             .registerPage(PAGE_MEASURE, onMeasurePageOpen, onMeasurePageUpdate, 0, false) // no delay!
             .registerPage(PAGE_ABOUT, onAboutPageOpen, checkButtonReturn)
             .registerPage(PAGE_RANDOM, nullptr, []() { onRandomPageUpdate(); checkButtonReturn(); })
             .registerPage(PAGE_KEYBOARD, onKeyboardPageOpen, []() { keyboard->update(); })
             .setDefaultPage(
              device.has_display ? PAGE_MENU : PAGE_MEASURE // auto measure on no screen devices
            ).begin();
}

/**
 *  Aicuflow x Arduino Setup & Loop
*/
void setup() {
  initPoints();
  initDeviceGPIOPins();
  initSerial();
  
  if (!device.has_display) delay(1000);
  if (device.has_display)  initTFTScreen();
  if (device.has_display)  bootScreen(3000);
  if (device.has_display)  setupMenus();
  if (device.has_display)  setupPageManager();
}

void loop() {
  pageManager->update(); // render individual menus and pages
}