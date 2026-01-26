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

#include "library/device/DeviceProps.cpp"        // device detection
#include "library/device/Settings.cpp"           // persistent settings
#include "library/aicuflow/AicuClient.cpp"       // --> client library you can use <--
#include "library/sensors/SensorMeasurement.cpp" // sensor registry for measuring, plots and saving
#include "library/graphics/TFTKeyboard.cpp"      // enter strings using buttons (cool)
#include "library/graphics/PageManager.cpp"      // multi-page app on screens
#include "library/graphics/closeFunctions.cpp"   // for close timing & recognition

#include "library/apps/boot.cpp"    // page: boot screen & setup
#include "library/apps/measurement.cpp"  // page: measurement helpers
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
bool wifiAvailable = false;

// Menu & UI
TFTMenu *mainMenu, *toolsMenu, *gamesMenu, *graphicsMenu, *connectToolsMenu;
TFTMenu *settingsMenu, *wifiMenu, *aicuAPIMenu;
TFTKeyboard* keyboard;
PageManager* pageManager;
void setupMenus() {
  const char* s_back = en_de("Back", "Hoch");
  // connection tools
  connectToolsMenu = new TFTMenu(&tft, en_de("Wireless", "Funk"));
  connectToolsMenu->addBackItem(s_back);
  connectToolsMenu->addItem(en_de("Wifi Scan", "Wlan Suche"), []() { pageManager->openPage("wifiscan"); });
  connectToolsMenu->addItem(en_de("BT Scan", "BT Suche"), []() { pageManager->openPage("btscan"); });

  // graphicstest
  graphicsMenu = new TFTMenu(&tft, en_de("Graphics", "Grafik"));
  graphicsMenu->addBackItem(s_back);
  graphicsMenu->addItem("Mandelbrot", []() { pageManager->openPage("mandelbrot"); });
  graphicsMenu->addItem(en_de("3D Objects", "3D Objekte"), []() { pageManager->openPage("3d"); });
  graphicsMenu->addItem(en_de("Color Test", "Farbtest"), []() { pageManager->openPage("colors"); });
  graphicsMenu->addItem(en_de("Color Wheel", "Farbrad"), []() { pageManager->openPage("colorwheel"); });
  graphicsMenu->addItem(en_de("Random Color", "Farbrauschen"), []() { pageManager->openPage("random"); });

  // games
  gamesMenu = new TFTMenu(&tft, en_de("Games","Spiele"));
  gamesMenu->addBackItem(s_back);
  gamesMenu->addItem(en_de("Snake Game", "Snake Spiel"), []() { pageManager->openPage("snake"); });
  gamesMenu->addItem("Game of Life", []() { pageManager->openPage("gol"); });

  // tools
  toolsMenu = new TFTMenu(&tft, "Tools");
  toolsMenu->addBackItem(s_back);
  toolsMenu->addSubmenu(en_de("Wireless", "Funk"), connectToolsMenu);
  toolsMenu->addSubmenu(en_de("Graphics", "Grafik"), graphicsMenu);
  toolsMenu->addSubmenu(en_de("Games","Spiele"), gamesMenu);
  
  // wifi settings
  wifiMenu = new TFTMenu(&tft, en_de("WiFi Settings", "Wlan Zugang"));
  wifiMenu->addBackItem(s_back);
  wifiMenu->addItem(en_de("WiFi SSID", "Wlan Name"), []() { pageManager->openPage("keyboard", (void*)3); });
  wifiMenu->addItem(en_de("WiFi Passwd", "Wlan Passwd"), []() { pageManager->openPage("keyboard", (void*)4); });
  
  // aicuflow settings
  aicuAPIMenu = new TFTMenu(&tft, "Aicuflow API");
  aicuAPIMenu->addBackItem(s_back);
  aicuAPIMenu->addItem(en_de("User Mail", "Login Mail"), []() { pageManager->openPage("keyboard", (void*)5); });
  aicuAPIMenu->addItem(en_de("User Passwd.", "Login Passwd."), []() { pageManager->openPage("keyboard", (void*)6); });
  aicuAPIMenu->addItem(en_de("Flow ID", "Flow Id"), []() { pageManager->openPage("keyboard", (void*)7); });
  aicuAPIMenu->addItem(en_de("Device Name", "Geraetename"), []() { pageManager->openPage("keyboard", (void*)1); }); // 1 = device name context
  aicuAPIMenu->addItem(en_de("File Name", "Dateiname"), []() { pageManager->openPage("keyboard", (void*)2); }); // 2 = streamfilename context
  
  // settings
  settingsMenu = new TFTMenu(&tft, en_de("Settings", "Einstellungen"));
  settingsMenu->addBackItem(s_back);
  settingsMenu->addSubmenu(en_de("WiFi Settings", "Wlan Zugang"), wifiMenu);
  settingsMenu->addSubmenu("Aicuflow API", aicuAPIMenu);
  settingsMenu->addItem(en_de("Language: EN", "Language: DE"), []() { locale = locale == "en" ? "de" : "en"; saveSettings(); esp_restart(); });
  settingsMenu->addItem(en_de("Factory Reset", "Zurucksetzen"), []() { clearSettings(); esp_restart(); });
  settingsMenu->addItem(en_de("Restart Device", "Neu starten"), []() { esp_restart(); });
  
  // main
  mainMenu = new TFTMenu(&tft, en_de("Aicuflow IoT+AI", "Aicuflow IoT+KI"));
  mainMenu->addItem(en_de("Start", "Start"), []() { pageManager->openPage("measure"); });
  mainMenu->addSubmenu(en_de("Tools", "Tools"), toolsMenu);
  mainMenu->addSubmenu(en_de("Setup", "Setup"), settingsMenu);
  mainMenu->addItem(en_de("About", "Infos"), []() { pageManager->openPage("about"); });

  // add more menus here
  // mainMenu->addItem("Custom Page", []() { pageManager->openPage("[appname]"); });
  
  // keyboard
  keyboard = new TFTKeyboard(&tft, en_de("Enter Text", "Text eingeben"));
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

// Measurement page handlers
void measurementSetup() {
  if (device->has_display)               plotScreen(1000);
  if (device->has_wifi)                  connectWifiOrTimeout();
  if (device->has_wifi && wifiAvailable) connectAPI();
  if (!sensors.getEnabledCount())        registerAllSensors();
  if (device->has_display)               initSensorGraphs();
  if (device->has_wifi)                  startCPUSendTask();
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
  pageManager->registerPage("measure", measurementSetup, onMeasurePageUpdate, 0, false); // no delay!
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