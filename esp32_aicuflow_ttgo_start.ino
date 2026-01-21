/**
 *  Aicuflow AI Flow Builder + Arduino ESP32 Quickstart
 *
 *  Created: 20260121, by Finn Glas @ AICU GmbH
 *
 *  This sketch helps you stream data to the aicuflow platform easily.
 *  Just provide credentials, and all available sensor data is sent.
 *  If the device has a display, it is detected and filled with charts.
 *
 *  Tested devices:
 *  - ESP32 ttgo t1 (display);
 *  - liligo t3 s3 t-display (nerdminer case);
 *  - ESP32 devkit c (no display, 2020 build with hall sensor);
 *
 *  Check https://aicuflow.com/docs/library/arduino for more!
 */


#include <TFT_eSPI.h> // v2.5.43 Bodmer
#include <SPI.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_wifi.h>  // esp_wifi_set_max_tx_power(52); // ca 13dBm
#include <HTTPClient.h>
#include <ArduinoJson.h> // 7.4.2 by Benoit

#include "AicuClient.h"
#include "aicuflow_logo_64px.h" // aicuflow_logo
#include "aicuflow_logo_wide.h" // aicuflow_logo_wide
#include "ScrollingGraph.cpp"
#include "DeviceProps.h"

const char* WLAN_SSID = "your-wlan";
const char* WLAN_PASS = "your-pass";
const char* AICU_USER = "your-mail";
const char* AICU_PASS = "your-pass";
const char* PROJ_FLOW = "your-ai-cu-flow-uuid";
const char* PROJ_FILE = "esp32.arrow";

const char* DEVICE_ID = "ttt1"; // ttgo (t1), esp32 t display s3
const int POINTS_BATCH_SIZE = 256; // 64 always works
const int MEASURE_DELAY_MS = 100;
const int BUTTON_L = 0;
const int BUTTON_R = 35;

const auto& device = getDeviceProps();
TFT_eSPI tft = TFT_eSPI();
ScrollingGraph rightButtonGraph(&tft);
ScrollingGraph leftButtonGraph(&tft);
ScrollingGraph rssiGraph(&tft);
ScrollingGraph voltageGraph(&tft);
ScrollingGraph tempGraph(&tft);
ScrollingGraph heapGraph(&tft);
int screenWidth, screenHeight;
WiFiClientSecure client;
AicuClient aicu("https://prod-backend.aicuflow.com", true);

// json collect & stream
static DynamicJsonDocument points(384*POINTS_BATCH_SIZE*2); // est. data size + buffer
static JsonArray arr;
static uint16_t count = 0;
void initPoints() {
    points.clear();
    arr = points.to<JsonArray>();
    count = 0;
}
inline void addSampleAndAutoSend(int64_t t, double T, double U, int B_L, int B_R, int P, int n, int N, int i) {
    JsonObject o = arr.createNestedObject();
    o["millis"] = t;
    o["device"] = DEVICE_ID;
    o["temperature"] = T;
    o["voltage"] = U;
    o["left_button"] = B_L;
    o["right_button"] = B_R;
    o["rssi"] = P;
    o["heapfree"] = n;
    o["cpu_freq_hz"] = N;
    o["cpu_speed"] = i;

    if (++count >= POINTS_BATCH_SIZE) flushSamples();
}
void flushSamples() {
    if (count == 0) return;
    aicu.sendTimeseriesPoints(PROJ_FLOW, PROJ_FILE, points);
    points.clear();
    arr = points.to<JsonArray>();
    count = 0;
}

// graphing
void initGraphs() {
  tft.println("Measuring...");
  int barheight = 14;
  int bargap = 3;
  int fullheight = barheight + bargap;
  // .begin(x, y, width, height, min, max, color, label)
  rightButtonGraph.begin(0, screenHeight-fullheight*6,  135, barheight, 0, 1, TFT_BLUE, "Button");
  leftButtonGraph.begin(0, screenHeight-fullheight*5,  135, barheight, 0, 1, TFT_PURPLE, "Button");
  rssiGraph.begin(0, screenHeight-fullheight*4,  135, barheight, -100, -30, TFT_GREEN, "RSSI");
  voltageGraph.begin(0, screenHeight-fullheight*3,  135, barheight, 20, 60,   TFT_RED, "Voltage (V)");
  tempGraph.begin(0, screenHeight-fullheight*2,  135, barheight, 20, 60,   TFT_ORANGE, "Temp (C)");
  heapGraph.begin(0, screenHeight-fullheight*1,  135, barheight, 100, 300, TFT_CYAN, "Heap (KB)");
}
void updateGraphs(int B_R, int B_L, float P, float U, float T, int n) {
    rightButtonGraph.update(B_R);
    leftButtonGraph.update(B_L);
    rssiGraph.update(P);
    voltageGraph.update(U);
    tempGraph.update(T);
    heapGraph.update(n);
}
inline void printSampledValues(int64_t t, double T, double U, int B_L, int B_R, int P, int n, int N, int i) {
  Serial.print("Millis start: "); Serial.println(t);
  Serial.print("Right Button: "); Serial.println(B_R);
  Serial.print("Left Button: "); Serial.println(B_L);
  Serial.print("Wifi RSSI: "); Serial.println(P);
  Serial.print("Chip Voltage: "); Serial.println(U);
  Serial.print("Chip Temp (°C): "); Serial.println(T);
  Serial.print("Free Memory: "); Serial.println(n);
  Serial.print("CPU Freq Hz: "); Serial.println(N);
  Serial.print("Loops/10ms: "); Serial.println(1.0*i/10);
  Serial.println("========");
}

// initing
void initGPIOPins() {
  pinMode(BUTTON_L, INPUT_PULLUP);
  pinMode(BUTTON_R, INPUT);
  pinMode(14, OUTPUT);
  digitalWrite(14, HIGH);
}
void initSerial() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Hello World!");
  delay(500);
  Serial.print("Aicuflow booted on "); Serial.print(device.kind_slug); Serial.println("!");
}
void initTFTScreen() {
  screenWidth  = tft.width();
  screenHeight = tft.height();
  Serial.print("Width: "); Serial.println(screenWidth);   // ttgo-t1: 135 
  Serial.print("Height: "); Serial.println(screenHeight); // ttgo-t1: 240
  tft.init();
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
    tft.pushImage(screenWidth/2-126/2, 0, 126, 28, aicuflow_logo_wide);
    tft.setCursor(0, 32);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK); // details
    tft.println("ESP32 IoT-AI Endpoint");
    tft.println("https://aicuflow.com");
    tft.println("");
    tft.println("");
    tft.println("Device started...");
    delay(duration);
}

void connectWifiBlocking() {
  if (device.has_display) tft.print("Connecting to wifi");
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (device.has_display) tft.print("."); // Loading progress
  }

  client.setInsecure(); // TLS workaround
  aicu.setWiFiClient(&client);
  
  if (device.has_display) {
    tft.print("\n");
    tft.println("WiFi Connected!");
    tft.print("IP: "); 
    tft.println(WiFi.localIP());
    tft.println("");
  }
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

void setup() {
  initPoints();
  initGPIOPins();
  initSerial();
  
  if (!device.has_display) delay(1000);
  if (device.has_display)  initTFTScreen();
  if (device.has_display)  bootScreen(1000);
  if (device.has_display)  plotScreen(1000);
  if (device.has_wifi)     connectWifiBlocking();
  if (device.has_wifi)     connectAPI();
  if (device.has_display)  initGraphs();
}

void loop() {
  delay(MEASURE_DELAY_MS); // Buffer

  // Measure Data
  float U = (analogRead(34) / 4095.0) * 2.0 * 3.3 * 1.1; // 2=ResDiv;3.3RefV;4095-12-bitADCres
  float T = (temperatureRead());
  int B_L = digitalRead(BUTTON_L) == 0;
  int B_R = digitalRead(BUTTON_R) == 0;
  float P = WiFi.RSSI();
  int n = ESP.getFreeHeap();
  uint32_t N = getCpuFrequencyMhz(); 
  unsigned long t = millis();
  unsigned long i = 0;
  while (millis() - t < 10) i++;

  // Forward Data
  if (device.has_display) updateGraphs(B_R, B_L, P, U, T, n);
  if (device.has_wifi) addSampleAndAutoSend(t, T, U, B_L, B_R, P, n, N, i);
}
