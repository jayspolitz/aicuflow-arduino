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
#include <esp_wifi.h>  // esp_wifi_set_max_tx_power(52); // ca 13dBm
#include <HTTPClient.h>
#include <ArduinoJson.h> // 7.4.2 by Benoit
#include <Preferences.h>  // ESP32 built-in lib

#include "library/device/DeviceProps.cpp"
#include "library/aicuflow/AicuClient.cpp"
#include "library/sensors/SensorMeasurement.cpp"
#include "library/graphics/TFTMenuModern.cpp"    // TFTMenuModern or TFTMenu
#include "library/graphics/TFTKeyboard.cpp"
#include "library/graphics/aicuflow_logo_64px.h" // aicuflow_logo
#include "library/graphics/aicuflow_logo_wide.h" // aicuflow_logo_wide

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

const auto& device = getDeviceProps();
int screenWidth, screenHeight;
int LEFT_BUTTON, RIGHT_BUTTON;
static uint32_t lastInputMs = 0;
static bool screenAwake = true;
static bool wifiAvailable = false;

// enums need to be high up or build fails
enum Page {
  PAGE_MENU,
  PAGE_MEASURE,
  PAGE_ABOUT,
  PAGE_RANDOM,
  PAGE_KEYBOARD
};
enum KeyboardContext {
  CONTEXT_DEVICE_NAME
};

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

/**
 * Menu and Page logic
 * for TFT Screen controls
 */
TFTMenu *mainMenu;
TFTMenu *actionsMenu;
TFTMenu *settingsMenu;
TFTMenu* previouslyActiveMenu = nullptr;
Page currentPage = PAGE_MENU;
void openPage(Page p) {
  previouslyActiveMenu = TFTMenu::currentActiveMenu;
  currentPage = p;
}
bool blockInput = false; // block input after page changes
unsigned long blockInputUntil = 0;
void blockInputFor(unsigned long ms) {
  blockInput = true;
  blockInputUntil = millis() + ms;
}
bool isInputBlocked() {
  if (blockInput && millis() >= blockInputUntil) blockInput = false;
  return blockInput;
}
void returnToMenu() {
  // restore last menu
  openPage(PAGE_MENU);
  tft.fillScreen(TFT_BLACK);
  if (previouslyActiveMenu != nullptr) {
    previouslyActiveMenu->begin();  // This sets it as active and redraws
  } else {
    mainMenu->begin();  // Fallback to main menu
  }
  delay(200);         // crude debounce
}
void returnToMenuIfButton() {
  if (
    digitalRead(LEFT_BUTTON) == LOW ||
    digitalRead(RIGHT_BUTTON) == LOW
  ) returnToMenu();
}
// kbd
TFTKeyboard* keyboard;
KeyboardContext currentKeyboardContext;
void openNameKeyboard() {
  currentKeyboardContext = CONTEXT_DEVICE_NAME;
  openPage(PAGE_KEYBOARD);
  keyboard->setText(deviceName);
  tft.fillScreen(TFT_BLACK);
  keyboard->begin();
  blockInputFor(300);
}
void onTextConfirmed(String text) {
  if (currentKeyboardContext == CONTEXT_DEVICE_NAME) {
    deviceName = text;
    if(VERBOSE) Serial.println("Saved: " + deviceName);
  }
  saveSettings();
  returnToMenu();
  blockInputFor(300);
}

/**
 * About page - example of a custom page
 */
void openAboutPage() {
  openPage(PAGE_ABOUT);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  int w = tft.width();
  int h = tft.height();

  int margin = device.kind_slug == "esp32-ttgo-t1" ? 0 : 12;

  tft.setCursor(margin, margin);

  int titleSize = (w >= 240) ? 2 : 1;

  // Title
  tft.setTextSize(titleSize);
  tft.println("Aicuflow");

  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.println("by AICU GmbH");
  tft.println();

  // Divider
  tft.drawFastHLine(margin, tft.getCursorY(), w - margin * 2, TFT_DARKGREY);
  tft.println();
  tft.println();

  // Description
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Create big data workflows &");
  tft.println("automations by chatting with AI.");
  tft.println("Train custom AI models.");
  tft.println("Deploy & scale with one click.");
  tft.println();

  // Divider
  tft.drawFastHLine(margin, tft.getCursorY(), w - margin * 2, TFT_DARKGREY);
  tft.println();
  tft.println();

  // Company details
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.println("AICU GmbH, Heilbronn");
  tft.println("HRB 794842 · Amtsgericht Stuttgart");
  tft.println("VAT: DE368976811");
  tft.println("CEO: Julia C. Yukovich");

  // Footer (explicit positioning only here)
  tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
  tft.setCursor(margin, h - 22);
  tft.print("Docs: aicuflow.com");

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(w - 90, h - 10);
  tft.print("Press any key");
}
/**
 * Random Page
 */
void openRandomPage() {
  openPage(PAGE_RANDOM);
}
void updateRandomPage() {
  int w = tft.width();
  int h = tft.height();

  // Seed randomness (ESP32 has HW RNG, this just mixes a bit)
  randomSeed(esp_random());

  tft.startWrite();
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      uint16_t color = random(0xFFFF); // full 16-bit RGB565 space
      tft.drawPixel(x, y, color);
    }
  }
  tft.endWrite();

  // Footer only (explicit positioning allowed)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(4, h - 10);
  tft.print("Press any key");
  
  returnToMenuIfButton();
}


/**
 *  Function definitions
*/

// json collect & stream, dtype
static DynamicJsonDocument points(384*POINTS_BATCH_SIZE*2);
static JsonArray arr;
static uint16_t count = 0;
static int32_t PERIOD_US = MEASURE_DELAY_MS * 1000UL;
void initPoints() {
  points.clear();
  arr = points.to<JsonArray>();
  count = 0;
}

// Async queue for sending
struct JsonBatch { DynamicJsonDocument* doc; };
QueueHandle_t flushQueue;
void flushSamplesAsync() {
  if (count == 0) return;

  DynamicJsonDocument* batch = new DynamicJsonDocument(points.memoryUsage() + 384*POINTS_BATCH_SIZE*2);
  *batch = points; // deep copy
  JsonBatch jb = {batch};
  if (xQueueSend(flushQueue, &jb, 0) != pdTRUE) {
    delete batch; // drop if queue full
  }

  points.clear();
  arr = points.to<JsonArray>();
  count = 0;
}
void addSampleAndAutoSend() {
  JsonObject o = arr.createNestedObject();
  sensors.toJson(o);
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
  lastInputMs = millis();
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
  actionsMenu->addItem("Random", openRandomPage);

  // settings
  settingsMenu = new TFTMenu(&tft, "Settings");
  settingsMenu->addBackItem();
  settingsMenu->setColors(TFT_BLACK, TFT_DARKGREEN, TFT_WHITE, TFT_WHITE);
  settingsMenu->addItem("Device Name", openNameKeyboard);
  settingsMenu->addItem("About", openAboutPage);
  
  // main
  mainMenu = new TFTMenu(&tft, "Aicuflow IoT");
  mainMenu->addItem("Start", openMeasurementPage);
  mainMenu->addSubmenu("Actions", actionsMenu);
  mainMenu->addSubmenu("Settings", settingsMenu);

  // keyboard
  keyboard = new TFTKeyboard(&tft, "Enter Text");
  keyboard->setButtonPins(LEFT_BUTTON, RIGHT_BUTTON);
  keyboard->setOnConfirm(onTextConfirmed);
  
  // after all propagate
  mainMenu->propagateButtonPins(LEFT_BUTTON, RIGHT_BUTTON);
  mainMenu->begin();
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
  else                     openMeasurementPage();
}

void openMeasurementPage() {
  openPage(PAGE_MEASURE);

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
void updateMeasurementPage() {
  // Precise time
  static uint32_t next = micros();
  uint32_t now = micros();
  if ((int32_t)(now - next) < 0) return;
  next += PERIOD_US;

  // Measure Data
  sensors.measure();

  // Forward Data
  if (VERBOSE && !device.has_display)    sensors.printValues(); // May be BLOCKING!
  if (screenAwake && device.has_display) sensors.updateGraphs();
  if (wifiAvailable && device.has_wifi)  addSampleAndAutoSend();

  // Screen power management
  if (device.has_display && SCREEN_IDLE_MS) {
    bool anyInput = sensors.getValue("left_button") || sensors.getValue("right_button");
    if (anyInput) {
      lastInputMs = millis();
      if (!screenAwake) {
        digitalWrite(TFT_BL, HIGH); // screen on
        screenAwake = true;
      }
    } else if (screenAwake && (millis() - lastInputMs > SCREEN_IDLE_MS)) {
      digitalWrite(TFT_BL, LOW); // screen off
      screenAwake = false;
    }
  }
}

void loop() {
  if (isInputBlocked()) {
    delay(20);
    return;
  }

  if (currentPage == PAGE_MENU) {
    mainMenu->update();
  } else if (currentPage == PAGE_KEYBOARD) {
    keyboard->update();
  } else if (currentPage == PAGE_MEASURE) {
    updateMeasurementPage();
    return; // no delay
  } else if (currentPage == PAGE_RANDOM) {
    updateRandomPage();
  } else {
    returnToMenuIfButton();
  }

  delay(20);
}
