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
 *  To start, find `registerAllSensors` and try adding new ones!
 *  Generally, search for "add more" and you find customisations!
 *
 ******************************************************************
 *
 *  Check https://aicuflow.com/docs/library/arduino for more!
 */

#include <TFT_eSPI.h>         // LIB NEED TO INSTALL v2.5.43 Bodmer, needs customisation
#include <SPI.h>              // built-in periphery protocol
#include <WiFi.h>             // built-in wifi connection (some devices)
#include <WiFiClientSecure.h> // alt (insecure): #include <WiFiClient.h>
#include <HTTPClient.h>       // built-in webbrowser :)
#include <ArduinoJson.h>      // LIB NEED TO INSTALL 7.4.2 by Benoit
#include <esp_wifi.h>         // some esp32 boards need: esp_wifi_set_max_tx_power(52); // ca 13dBm

#include "library/device/DeviceProps.cpp"        // device detection
#include "library/device/Settings.cpp"           // persistent settings
#include "library/aicuflow/AicuClient.cpp"       // --> client library you can use <--
#include "library/sensors/SensorMeasurement.cpp" // sensor registry for measuring, plots and saving
#include "library/graphics/TFTKeyboard.cpp"      // enter strings using buttons (cool)
#include "library/graphics/PageManager.cpp"      // multi-page app on screens
#include "library/graphics/closeFunctions.cpp"   // for close timing & recognition

#include "library/apps/boot.cpp"    // page: boot screen & setup
#include "library/apps/about.cpp"   // page: about (message)
#include "library/apps/random.cpp"  // page: random colors screen test
#include "library/apps/wifiscan.cpp"  // page: wifi scan
#include "library/apps/btscan.cpp"  // page: bluethooth scan
#include "library/apps/colortest.cpp"  // page: gradient color test page
#include "library/apps/colorwheeltest.cpp"  // page: color test page
#include "library/apps/snakegame.cpp"  // page: snake game
#include "library/apps/gol.cpp"  // page: conways game of life
#include "library/apps/mandelbrot.cpp"  // page: mandelbrot set render
#include "library/apps/wireframe3d.cpp"  // page: 3d rotation renderer
// add more pages here
// #include "library/apps/_expand.cpp" // page: custom page

// START FACTORY SETTINGS REPLACE THIS
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
// END FACTORY SETTINGS REPLACE THIS

// globals
WiFiClientSecure client;
AicuClient aicu(API_URL, VERBOSE);
TFT_eSPI tft = TFT_eSPI();
SensorMeasurement sensors("dev");

const DeviceProps* device = nullptr;
int screenWidth, screenHeight;
int LEFT_BUTTON, RIGHT_BUTTON;
static bool wifiAvailable = false;

void applySettings() {
  sensors.deviceId = deviceName.c_str();
}

// Menu & UI
TFTMenu *mainMenu, *toolsMenu, *gamesMenu, *graphicsMenu, *connectToolsMenu;
TFTMenu *settingsMenu, *wifiMenu, *aicuAPIMenu;
TFTKeyboard* keyboard;
PageManager* pageManager;
void setupMenus() {
  // connection tools
  connectToolsMenu = new TFTMenu(&tft, "Wireless");
  connectToolsMenu->addBackItem();
  connectToolsMenu->addItem("Wifi Scan", []() { pageManager->openPage("wifiscan"); });
  connectToolsMenu->addItem("BT Scan", []() { pageManager->openPage("btscan"); });

  // graphicstest
  graphicsMenu = new TFTMenu(&tft, "Graphics");
  graphicsMenu->addBackItem();
  graphicsMenu->addItem("Mandelbrot", []() { pageManager->openPage("mandelbrot"); });
  graphicsMenu->addItem("3D Objects", []() { pageManager->openPage("3d"); });
  graphicsMenu->addItem("Colortest", []() { pageManager->openPage("colors"); });
  graphicsMenu->addItem("Colorwheel", []() { pageManager->openPage("colorwheel"); });
  graphicsMenu->addItem("Random Color", []() { pageManager->openPage("random"); });

  // games
  gamesMenu = new TFTMenu(&tft, "Games");
  gamesMenu->addBackItem();
  gamesMenu->addItem("Snake Game", []() { pageManager->openPage("snake"); });
  gamesMenu->addItem("Game of Life", []() { pageManager->openPage("gol"); });

  // tools
  toolsMenu = new TFTMenu(&tft, "Tools");
  toolsMenu->addBackItem();
  toolsMenu->addSubmenu("Wireless", connectToolsMenu);
  toolsMenu->addSubmenu("Graphics", graphicsMenu);
  toolsMenu->addSubmenu("Games", gamesMenu);
  
  // wifi settings
  wifiMenu = new TFTMenu(&tft, "WiFi Settings");
  wifiMenu->addBackItem();
  wifiMenu->addItem("WiFi SSID", []() { pageManager->openPage("keyboard", (void*)3); });
  wifiMenu->addItem("WiFi Pass", []() { pageManager->openPage("keyboard", (void*)4); });
  
  // aicuflow settings
  aicuAPIMenu = new TFTMenu(&tft, "Aicuflow API");
  aicuAPIMenu->addBackItem();
  aicuAPIMenu->addItem("User Mail", []() { pageManager->openPage("keyboard", (void*)5); });
  aicuAPIMenu->addItem("User Pass", []() { pageManager->openPage("keyboard", (void*)6); });
  aicuAPIMenu->addItem("Flow ID", []() { pageManager->openPage("keyboard", (void*)7); });
  aicuAPIMenu->addItem("Device Name", []() { pageManager->openPage("keyboard", (void*)1); }); // 1 = device name context
  aicuAPIMenu->addItem("File Name", []() { pageManager->openPage("keyboard", (void*)2); }); // 2 = streamfilename context
  
  // settings
  settingsMenu = new TFTMenu(&tft, "Settings");
  settingsMenu->addBackItem();
  settingsMenu->addSubmenu("WiFi Settings", wifiMenu);
  settingsMenu->addSubmenu("API Settings", aicuAPIMenu);
  settingsMenu->addItem("Factory Reset", []() { clearSettings(); esp_restart(); });
  settingsMenu->addItem("Restart Device", []() { esp_restart(); });
  
  // main
  mainMenu = new TFTMenu(&tft, "Aicuflow IoT+AI");
  mainMenu->addItem("Start", []() { pageManager->openPage("measure"); });
  mainMenu->addSubmenu("IoT Tools", toolsMenu);
  mainMenu->addSubmenu("Preferences", settingsMenu);
  mainMenu->addItem("About Aicuflow", []() { pageManager->openPage("about"); });

  // add more menus here
  // mainMenu->addItem("Custom Page", []() { pageManager->openPage("[appname]"); });
  
  // keyboard
  keyboard = new TFTKeyboard(&tft, "Enter Text");
  keyboard->setButtonPins(LEFT_BUTTON, RIGHT_BUTTON);
  keyboard->setOnConfirm(onTextConfirmed);
  
  // after all propagate
  mainMenu->propagateButtonPins(LEFT_BUTTON, RIGHT_BUTTON);
}
void onMenuPageOpen() {
  tft.fillScreen(TFT_BLACK);
  TFTMenu* prevMenu = pageManager->getPreviousMenu();
  (prevMenu ? prevMenu : mainMenu)->begin();
}

// Keyboard
void onKeyboardPageOpen() {
  int context = pageManager->getContext<int>();
  if (context == 1) keyboard->setText(deviceName);          // 1 = CONTEXT_DEVICE_NAME
  else if (context == 2) keyboard->setText(streamFileName); // 2 = CONTEXT_FILE_NAME

  else if (context == 3) keyboard->setText(wlanSSID); // 3 = CONTEXT_WLAN_SSID
  else if (context == 4) keyboard->setText(wlanPass); // 4 = CONTEXT_WLAN_PASS
  else if (context == 5) keyboard->setText(aicuMail); // 5 = CONTEXT_AICU_USER
  else if (context == 6) keyboard->setText(aicuPass); // 6 = CONTEXT_AICU_PASS
  else if (context == 7) keyboard->setText(aicuFlow); // 7 = CONTEXT_AICU_FLOW
  
  tft.fillScreen(TFT_BLACK);
  keyboard->begin();
}
void onTextConfirmed(String text) {
  int context = pageManager->getContext<int>();
  if (context == 1) deviceName = text;          // 1 = CONTEXT_DEVICE_NAME
  else if (context == 2) streamFileName = text; // 2 = CONTEXT_FILE_NAME

  else if (context == 3) wlanSSID = text; // 3 = CONTEXT_WLAN_SSID
  else if (context == 4) wlanPass = text; // 4 = CONTEXT_WLAN_PASS
  else if (context == 5) aicuMail = text; // 5 = CONTEXT_AICU_USER
  else if (context == 6) aicuPass = text; // 6 = CONTEXT_AICU_PASS
  else if (context == 7) aicuFlow = text; // 7 = CONTEXT_AICU_FLOW

  saveSettings();
  applySettings();
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
      aicu.sendTimeseriesPoints(aicuFlow, streamFileName, *jb.doc);
      delete jb.doc; // free memory
    }
  }
}

// connecting
void connectWifiOrTimeout() {
  if (device->has_display) tft.print("Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(wlanSSID, wlanPass);

  // try to connect (until timeout)
  unsigned long start = millis();
  while (
    (WiFi.status() != WL_CONNECTED) // not connected
    && (!WIFI_TIMEOUT || (millis() - start < WIFI_TIMEOUT)) // timeout
  ) {
    delay(500);
    if (device->has_display) tft.print("."); // Loading progress
  }

  // no connection = off
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    wifiAvailable = false;
    tft.print("\n");
    tft.setTextColor(TFT_RED);
    tft.println("WiFi failed :/");
    tft.setTextColor(TFT_WHITE);
    return;
  } else { // connected = on
    wifiAvailable = true;
  }

  // set up wifi stuff
  client.setInsecure(); // TLS workaround
  aicu.setWiFiClient(&client);
  
  // print
  if (wifiAvailable && device->has_display) {
    tft.print("\n");
    tft.setTextColor(TFT_GREEN);
    tft.print("WiFi: "); 
    tft.println(WiFi.localIP());
    tft.setTextColor(TFT_WHITE);
  } else if (device->has_display) tft.print("\n");
  Serial.print("Wifi-Mac: "); Serial.println(WiFi.macAddress());
  delay(500);
}
void connectAPI() {
  if (!aicu.login(aicuMail, aicuPass)) {
    if (device->has_display){
      tft.setTextColor(TFT_RED);
      tft.println("API connection failed! :/");
      tft.setTextColor(TFT_WHITE);
    }
    Serial.println("Login failed!");
    return;
  } else {
    if (device->has_display){
      tft.setTextColor(TFT_GREEN);
      tft.println("API connected!");
      tft.setTextColor(TFT_WHITE);
    }
    Serial.println("Login success!");
  }
}

void registerAllSensors() {
  // Parameters: (key, label, readFunc, min, max, color, enabled, showGraph)
  
  sensors.registerSensor("left_button", "Button L", 
    [&]() { return digitalRead(LEFT_BUTTON) == 0 ? 1.0 : 0.0; },
    0, 1, TFT_PURPLE, LOG_SEND, SHOW_GRAPH);
  sensors.registerSensor("right_button", "Button R",
    [&]() { return digitalRead(RIGHT_BUTTON) == 0 ? 1.0 : 0.0; },
    0, 1, TFT_BLUE, LOG_SEND, SHOW_GRAPH);
  if (device->has_wifi && wifiAvailable) sensors.registerSensor("rssi", "RSSI (dBm)",
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
  
  // Want to add more sensors?
  //   Ex1) Custom analog sensor
  //      sensors.registerSensor("light", "Light", []() { return analogRead(35) / 4095.0 * 100; }, 0, 100, TFT_WHITE, LOG_SEND, SHOW_GRAPH);
  //   Ex2) Not graphed, deactivated sensor
  //      sensors.registerSensor("uptime", "Uptime (s)", []() { return millis() / 1000.0; }, 0, 86400, TFT_MAGENTA, LOG_NONE, HIDE_GRAPH);
  
  if (VERBOSE) {
    Serial.print("Registered sensors: ");
    Serial.println(sensors.getEnabledCount());
  }
}
void initSensorGraphs() {
  if (device->kind_slug == "lilygo-t-display-s3")
    sensors.setGraphSpacing(22, 5);
  else sensors.setGraphSpacing(14, 3);
  if (device->has_wifi && wifiAvailable){
    tft.setTextColor(TFT_CYAN);
    tft.print("Sending to "); tft.print(streamFileName); tft.println(".arrow");
    tft.setTextColor(TFT_WHITE);
  }
  else {
    tft.setTextColor(TFT_YELLOW);
    tft.println("Measuring Locally...");
    tft.setTextColor(TFT_WHITE);
  }
  sensors.initGraphs(&tft, screenWidth, screenHeight);
  if (VERBOSE) {
    Serial.print("Graphed sensors: ");
    Serial.println(sensors.getGraphCount());
  }
}

// Measurement page handlers
void onMeasurePageOpen() {
  if (device->has_display)  plotScreen(1000);
  if (device->has_wifi)     connectWifiOrTimeout();
  if (device->has_wifi && wifiAvailable) connectAPI();

  if (!sensors.getEnabledCount()) registerAllSensors();
  if (device->has_display)  initSensorGraphs();
  
  if (device->has_wifi) {
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
  if (VERBOSE && !device->has_display)    sensors.printValues(); // May be BLOCKING!
  if (pageManager->screenAwake && device->has_display) // save energy
                                         sensors.updateGraphs();
  if (wifiAvailable && device->has_wifi)  addSampleAndAutoSend();

  closePageIfBothLongPressed();
}

// initing
void initDeviceGPIOPins() {
  // specifics
  if (device->kind_slug == "esp32-ttgo-t1") {
    // voltage divider something
    pinMode(14, OUTPUT);
    digitalWrite(14, HIGH);

    // buttons
    LEFT_BUTTON = 0;
    RIGHT_BUTTON = 35;
    pinMode(LEFT_BUTTON, INPUT_PULLUP);
    pinMode(RIGHT_BUTTON, INPUT);
  } else if (device->kind_slug == "lilygo-t-display-s3") {
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
  Serial.print("Aicuflow booted on "); Serial.print(device->kind_slug); Serial.println("!");
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

/* Aicuflow x Arduino Setup & Loop */
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
  
  pageManager = new PageManager(&tft, LEFT_BUTTON, RIGHT_BUTTON, SCREEN_IDLE_MS);
  pageManager->registerPage("menu", onMenuPageOpen, []() { mainMenu->update(); });
  pageManager->registerPage("measure", onMeasurePageOpen, onMeasurePageUpdate, 0, false); // no delay!
  pageManager->registerPage("about", onAboutPageOpen, closePageIfAnyButtonIsPressed);
  pageManager->registerPage("random", nullptr, []() { onRandomPageUpdate(); closePageIfAnyButtonIsPressed(); });
  pageManager->registerPage("keyboard", onKeyboardPageOpen, []() { keyboard->update(); });
  pageManager->registerPage("wifiscan", onWifiPageOpen, onWifiPageUpdate);
  pageManager->registerPage("btscan", onBTPageOpen, onBTPageUpdate);
  pageManager->registerPage("colors", onColorTestPageOpen, onColorTestPageUpdate);
  pageManager->registerPage("colorwheel", onColorWheelTestPageOpen, onColorWheelTestPageUpdate);
  pageManager->registerPage("snake", onSnakeGamePageOpen, onSnakeGamePageUpdate);
  pageManager->registerPage("gol", onGameOfLifePageOpen, onGameOfLifePageUpdate);
  pageManager->registerPage("mandelbrot", onMandelbrotPageOpen, onMandelbrotPageUpdate);
  pageManager->registerPage("3d", onWireframe3dPageOpen, onWireframe3dPageUpdate);
  // add more apps here, see `library/apps/_expand.cpp`
  // pageManager->registerPage("[appname]", onPageOpen, onPageUpdate);

  pageManager->setDefaultPage(device->has_display ? "menu" : "measure");
  pageManager->begin();
}

void loop() {
  pageManager->update(); // render menus, pages / apps
}