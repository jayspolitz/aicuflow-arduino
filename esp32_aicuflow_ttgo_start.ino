#define USE_DISPLAY 0
#define USE_HALL 0 // google says its only in v2.x?

#pragma region imports // ===
#include <TFT_eSPI.h> // Graphics and font library by "Bodmer" install v2.5.43 before anything, maybe needs to be ported in
// TFT_eSPI: Needs modifications under Arduino/libraries/TFT_eSPI/User_Setup_Select.h:
//   UNCOMMENT THIS in that file:
//   // #include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT
//   COMMENT THIS out in that file:
//   #include <User_Setup.h>           // Default setup is root library folder
#include <SPI.h>
// wifi imports
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // 7.4.2 by Benoit
// graphic imports
#include "aicuflow_logo_64px.h" // aicuflow_logo
#include "aicuflow_logo_wide.h" // aicuflow_logo_wide
//
//#include "esp_clk.h"
#include "ScrollingGraph.cpp"
#pragma endregion // ===
#include <esp_wifi.h>
// After WiFi.mode(WIFI_STA) but before WiFi.begin()
/**
 *
 *  Compiling for different devices!

ttgo t1

liligo t3 s3

 *
 */

#pragma region defines // ===

#define WLAN_SSID "aicuflow"
#define WLAN_PASS "#aicuties"

#define AICU_USER "your_user"
#define AICU_PASSWORD "your_user"

String accessToken; // to store the token

const int BUTTON_L = 0;
const int BUTTON_R = 35;

#if USE_DISPLAY
TFT_eSPI tft = TFT_eSPI();
ScrollingGraph rssiGraph(&tft);
ScrollingGraph tempGraph(&tft);
ScrollingGraph voltageGraph(&tft);
ScrollingGraph heapGraph(&tft);
ScrollingGraph leftButtonGraph(&tft);
ScrollingGraph rightButtonGraph(&tft);

const int G_WIDTH = 135;  // Match your screen width
const int G_HEIGHT = 20;  // 20 pixels high
float values[G_WIDTH];    // Buffer for auto-scaling
int head = 0;
#endif


#define SEND_SAMPLES 64 // ping it all 25,6 seconds if 10/sec are recorded;
// 512 would be about every min
// 1024 would be about every 2 min, the thing can still easily handle
// 2048 would be about every 4 min, the thing can still easily handle
#define MAX_SAMPLES (SEND_SAMPLES*2)

struct Sample {
  unsigned long millis;
  float temperature;
  float voltage;
  int left_button;
  int right_button;
  float rssi;
  int heapfree;
  uint32_t cpu_freq_hz;
  unsigned long counter;
};

Sample samples[MAX_SAMPLES];
uint32_t sampleCount = 0;
uint32_t loopCounter = 0;

const char* BASE_URL = "https://dev-backend.aicuflow.com";
const char* POST_URL = "/data/write-values/?filename=";
const char* FILE_NAME = "esp32.arrow";
const char* FLOW_ID = "9dd5d0a6-25a3-4e65-8aa6-04ec531de88c";
const char* DEVICE_ID = "dev"; // ttgo (t1), esp32 t display s3
const char* TEST_URL = "/subscriptions/plans/";

#pragma endregion // ===

void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  static uint32_t lastAttempt = 0;
  if (millis() - lastAttempt < 5000) return;

  lastAttempt = millis();
  WiFi.disconnect();
  WiFi.begin(WLAN_SSID, WLAN_PASS);
}

bool login() {
  HTTPClient http;
  String url = String(BASE_URL) + "/user/auth/login/";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<256> payload;
  payload["email"] = AICU_USER;
  payload["password"] = AICU_PASSWORD;
  payload["keep_me_logged_in"] = true;

  String body;
  serializeJson(payload, body);

  int code = http.POST(body);
  if (code != 200) {
    Serial.print("Login failed, code: ");
    Serial.println(code);
    http.end();
    return false;
  }

  String response = http.getString();
  http.end();

  StaticJsonDocument<1024> doc;
  DeserializationError err = deserializeJson(doc, response);
  if (err) {
    Serial.println("JSON parse error (login)");
    return false;
  }

  accessToken = doc["data"]["accesstoken"].as<String>();
  Serial.println("Login OK");
  return true;
}

int listFlowsAndCount() {
  HTTPClient http;
  String url = String(BASE_URL) + "/flows/?page=1&page_size=100";

  http.begin(url);
  http.addHeader("Authorization", "Bearer " + accessToken);

  int code = http.GET();
  if (code != 200) {
    Serial.print("List flows failed, code: ");
    Serial.println(code);
    http.end();
    return -1;
  }

  String response = http.getString();
  http.end();

  StaticJsonDocument<8192> doc;
  DeserializationError err = deserializeJson(doc, response);
  if (err) {
    Serial.println("JSON parse error (flows)");
    return -1;
  }

  JsonArray flows = doc["data"].as<JsonArray>();
  return flows.size();
}


void postSamples() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (sampleCount == 0) return;

  String json = "[";
  for (int i = 0; i < sampleCount; i++) {
    json += "{";
    json += "\"millis\":" + String(samples[i].millis) + ",";
    json += "\"device\":\"" + String(DEVICE_ID) + "\",";
    json += "\"temperature\":" + String(samples[i].temperature) + ",";
    json += "\"voltage\":" + String(samples[i].voltage) + ",";
    json += "\"left_button\":" + String(samples[i].left_button) + ",";
    json += "\"right_button\":" + String(samples[i].right_button) + ",";
    json += "\"rssi\":" + String(samples[i].rssi) + ",";
    json += "\"heapfree\":" + String(samples[i].heapfree) + ",";
    json += "\"cpu_freq_hz\":" + String(samples[i].cpu_freq_hz) + ",";
    json += "\"cpu_speed\":" + String(samples[i].counter);
    json += "}";

  float temperature;
  float voltage;
  int left_button;
  int right_button;
  float rssi;
  int heapfree;
  uint32_t cpu_freq_hz;
  unsigned long counter;
    if (i < sampleCount - 1) json += ",";
  }
  json += "]";

  HTTPClient http;
  http.begin(String(BASE_URL) + POST_URL + FILE_NAME + "&flow=" + FLOW_ID);
  http.addHeader("Authorization", "Bearer " + accessToken);
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(json);

  // optional debug
  Serial.printf("POST %d (%d samples)\n", code, sampleCount);

  http.end();

  if (code > 0 && code < 300) {
    sampleCount = 0; // clear only on success
  }
}

void setup() {
  esp_wifi_set_max_tx_power(52); // Approximately 13dBm

  // === Pin Setup ===
  pinMode(BUTTON_L, INPUT_PULLUP); // Button 1
  pinMode(BUTTON_R, INPUT);        // Button 2 (GPIO 35 is input-only)
  pinMode(14, OUTPUT); // voltage?
  digitalWrite(14, HIGH);

  // === Serial Boot ===
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Aicuflow ESP32 booted");

  #if USE_DISPLAY
    int screenWidth  = tft.width();
    int screenHeight = tft.height();

    Serial.print("Width: "); Serial.println(screenWidth); // 135
    Serial.print("Height: "); Serial.println(screenHeight); // 240

    // === Display Startup ===
    uint16_t TFT_AICU_TEAL = tft.color565(101, 195, 186);
    tft.init();

    // = boot screen 1
    tft.setRotation(0); // Set vert orientation
    tft.setCursor(0, 0);
    tft.fillScreen(TFT_BLACK);
    tft.pushImage(screenWidth/2-126/2, screenHeight/2-14, 126, 28, aicuflow_logo_wide);
  #endif

  delay(1000);

  #if USE_DISPLAY
    // = screen information
    tft.setRotation(0); // Set vertical orientation
    tft.setCursor(0, 0);
    tft.fillScreen(TFT_BLACK);
    tft.pushImage(screenWidth/2-126/2, 0, 126, 28, aicuflow_logo_wide);
    tft.setCursor(0, 32);
    // tft.setTextColor(TFT_AICU_TEAL, TFT_BLACK); 
    // tft.setTextSize(2);
    // tft.invertDisplay(true);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("ESP32 IoT-AI Endpoint");
    tft.println("https://aicuflow.com");
    tft.println("");
    tft.println("");
    tft.println("Device started...");
  #endif


  // === WIFI Setup ===
    // 1. Connect to WiFi
  #if USE_DISPLAY
    tft.print("Connecting to wifi");
  #endif
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #if USE_DISPLAY
      tft.print("."); // Loading progress
    #endif
  }
  #if USE_DISPLAY
    tft.print("\n");
    tft.println("WiFi Connected!");
    tft.print("IP: "); 
    tft.println(WiFi.localIP());
    tft.println("");
  #endif

  Serial.print("Mac: "); Serial.println(WiFi.macAddress());
  delay(500);

  // 2. Send HTTP Request
  #if USE_DISPLAY
    tft.println("Fetching Data...");
  #endif

  if (login()) {
    int flowCount = listFlowsAndCount();
    #if USE_DISPLAY
      tft.println("API connected!");
      tft.print("Found "); tft.print(flowCount); tft.println(" Flows!");
    #endif
  } else {
    #if USE_DISPLAY
      tft.println("Auth failed! :/");
    #endif
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // Example: Fetching a test JSON object
    http.begin(String(BASE_URL) + TEST_URL); 
    int httpCode = http.GET();

    #if USE_DISPLAY
      if (httpCode > 0) {
        String payload = http.getString();
        tft.setTextColor(TFT_CYAN);
        tft.printf("Code: %d\n", httpCode);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE);
        //tft.println(payload.substring(0, 150)); // Show snippet
      } else {
        tft.setTextColor(TFT_RED);
        tft.println("HTTP Error");
      }
    #endif

    http.end();
  }
  #if USE_DISPLAY
    tft.println("Transmitting...");
  #endif

  #if USE_DISPLAY
    // Stack them from top to bottom
    // .begin(x, y, width, height, min, max, color, label)
    int barheight = 14;
    int bargap = 3;
    int fullheight = barheight + bargap;
    heapGraph.begin(0, screenHeight-fullheight*1,  135, barheight, 100, 300, TFT_CYAN, "Heap (KB)");
    tempGraph.begin(0, screenHeight-fullheight*2,  135, barheight, 20, 60,   TFT_ORANGE, "Temp (C)");
    voltageGraph.begin(0, screenHeight-fullheight*3,  135, barheight, 20, 60,   TFT_ORANGE, "Voltage (V)");
    rssiGraph.begin(0, screenHeight-fullheight*4,  135, barheight, -100, -30, TFT_GREEN, "RSSI");
    leftButtonGraph.begin(0, screenHeight-fullheight*5,  135, barheight, 0, 1, TFT_PURPLE, "Button");
    rightButtonGraph.begin(0, screenHeight-fullheight*6,  135, barheight, 0, 1, TFT_RED, "Button");
  #endif
}

void loop() {
  // === Measure and print time series ===
  uint16_t raw = analogRead(34);
  // 2.0 = Divider ratio (100k/100k resistors)
  // 3.3 = Reference voltage
  // 4095 = 12-bit ADC resolution
  float voltage = (raw / 4095.0) * 2.0 * 3.3 * 1.1; 
  //float voltage = (analogRead(35) / 4095.0) * 2.0 * 3.3 * 1.1;
  float temperature = (temperatureRead());
  int left_button = digitalRead(BUTTON_L) == 0;
  int right_button = digitalRead(BUTTON_R) == 0;
  float rssi = WiFi.RSSI();
  int heapfree = ESP.getFreeHeap();
  uint32_t cpu_freq_hz = getCpuFrequencyMhz(); 
  unsigned long start = millis();
  unsigned long counter = 0;
  while (millis() - start < 10) {
      counter++; // Simple loop to count iterations per second
  }

  Serial.print("Millis since start: ");
  Serial.println(millis());
  Serial.print("Chip Voltage: ");
  Serial.println(voltage);
  Serial.print("Chip Temperature (°C): ");
  Serial.println(temperature);
  Serial.print("Left Button: ");
  Serial.println(left_button);
  Serial.print("Right Button: ");
  Serial.println(right_button);
  Serial.print("Wifi RSSI: ");
  Serial.println(rssi);
  Serial.print("Free Memory: ");
  Serial.println(heapfree);
  Serial.print("CPU Freq Hz: ");
  Serial.println(cpu_freq_hz);
  Serial.print("Iterations per ms: ");
  Serial.println(1.0*counter/10);

  Serial.println("========");
  delay(100);

  // === Show sprite graphs ===
  #if USE_DISPLAY
    rssiGraph.update(rssi);
    voltageGraph.update(voltage);
    tempGraph.update(temperature);
    heapGraph.update(heapfree);
    leftButtonGraph.update(left_button);
    rightButtonGraph.update(right_button);
  #endif

  // === TODO Stream to Aicuflow Backend ===
  // "https://www.postb.in/1768855487559-8489910799544"

  if (sampleCount < MAX_SAMPLES) {
    samples[sampleCount++] = {
      millis(),
      temperature,
      voltage,
      left_button,
      right_button,
      rssi,
      heapfree,
      cpu_freq_hz,
      counter,
    };
  }

  loopCounter++;
  
  if (loopCounter % SEND_SAMPLES == 0) {
    ensureWiFi();
    postSamples();
  }
}
