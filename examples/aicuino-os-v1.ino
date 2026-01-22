/**
 *  Aicuflow AI Flow Builder + Arduino ESP32 Quickstart
 *
 *  Created: 20260121, by Finn Glas @ AICU GmbH
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


#include "DeviceProps.h"
#include <TFT_eSPI.h> // v2.5.43 Bodmer, needs customisation
#include <SPI.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_wifi.h>  // esp_wifi_set_max_tx_power(52); // ca 13dBm
#include <HTTPClient.h>
#include <ArduinoJson.h> // 7.4.2 by Benoit

#include "AicuClient.h"
#include "aicuflow_logo_64px.h" // aicuflow_logo
#include "aicuflow_logo_wide.h" // aicuflow_logo_wide
#include "ScrollingGraph.h"
#include "SensorMeasurement.h"

const char* WLAN_SSID = "your-wlan";
const char* WLAN_PASS = "your-pass";
const char* AICU_USER = "your-mail";
const char* AICU_PASS = "your-pass";
const char* PROJ_FLOW = "your-ai-cu-flow-uuid";
const char* PROJ_FILE = "esp32.arrow";

const int VERBOSE = true;
const char* DEVICE_ID = "ttt1"; // ttgo (t1), esp32 t display s3
const int POINTS_BATCH_SIZE = 64; // 64 always works, 256 sometimes did, but may be too large.
const int MEASURE_DELAY_MS = 100;
int BUTTON_L, BUTTON_R;
const int SCREEN_IDLE_MS = 30000; // also needs TFT_BL eg 38
static uint32_t lastInputMs = 0;
const int WIFI_TIMEOUT = 10000; // 10s, 0 -> blocking till wifi
static bool screenAwake = true;
static bool wifiAvailable = false;

const auto& device = getDeviceProps();
TFT_eSPI tft = TFT_eSPI();
int screenWidth, screenHeight;

WiFiClientSecure client;
AicuClient aicu("https://dev-backend.aicuflow.com", VERBOSE);
SensorMeasurement sensors(DEVICE_ID);

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
    BUTTON_L = 0;
    BUTTON_R = 35;
    pinMode(BUTTON_L, INPUT_PULLUP);
    pinMode(BUTTON_R, INPUT);
  } else if (device.kind_slug == "lilygo-t-display-s3") {
    // buttons
    BUTTON_L = 0;
    BUTTON_R = 14;
    pinMode(BUTTON_L, INPUT_PULLUP);
    pinMode(BUTTON_R, INPUT);

    // power up
    esp_wifi_set_max_tx_power(52); // ca 13dBm
  }
}
void initSerial() {
  Serial.begin(115200);
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
    [&]() { return digitalRead(BUTTON_L) == 0 ? 1.0 : 0.0; },
    0, 1, TFT_PURPLE, LOG_SEND, SHOW_GRAPH);
  sensors.registerSensor("right_button", "Button R",
    [&]() { return digitalRead(BUTTON_R) == 0 ? 1.0 : 0.0; },
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

void loop() {
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
