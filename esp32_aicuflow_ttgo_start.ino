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
// graphic imports
#include "aicuflow_logo_64px.h" // aicuflow_logo
#include "aicuflow_logo_wide.h" // aicuflow_logo_wide
//
//#include "esp_clk.h"
#pragma endregion // ===

#pragma region defines // ===

#define WLAN_SSID "aicuflow"
#define WLAN_PASS "#aicuties"

#define AICU_USER "YOUR_EMAIL"
#define AICU_PASSWORD "YOUR_PASSWORD"

const int BUTTON_L = 0;
const int BUTTON_R = 35;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite graph = TFT_eSprite(&tft);

const int G_WIDTH = 135;  // Match your screen width
const int G_HEIGHT = 20;  // 20 pixels high
float values[G_WIDTH];    // Buffer for auto-scaling
int head = 0;

#pragma defines // ===

void setup() {
  // === Pin Setup ===
  pinMode(BUTTON_L, INPUT_PULLUP); // Button 1
  pinMode(BUTTON_R, INPUT);        // Button 2 (GPIO 35 is input-only)
  pinMode(14, OUTPUT); // voltage?
  digitalWrite(14, HIGH);

  // === Serial Boot ===
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Aicuflow ESP32 booted");
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
  
  delay(1000);

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


  // === WIFI Setup ===
    // 1. Connect to WiFi
  tft.print("Connecting to wifi");
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print("."); // Loading progress
  }
  tft.print("\n");
  tft.println("WiFi Connected!");
  tft.print("IP: "); 
  tft.println(WiFi.localIP());
  tft.println("");
  Serial.print("Mac: "); Serial.println(WiFi.macAddress());
  delay(500);

  // 2. Send HTTP Request
  tft.println("Fetching Data...");

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // Example: Fetching a test JSON object
    http.begin("https://prod-backend.aicuflow.com/subscriptions/plans/"); 
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      tft.setTextColor(TFT_CYAN);
      tft.printf("Code: %d\n", httpCode);
      tft.setTextSize(1);
      tft.setTextColor(TFT_WHITE);
      tft.println("API connected!");
      //tft.println(payload.substring(0, 150)); // Show snippet
    } else {
      tft.setTextColor(TFT_RED);
      tft.println("HTTP Error");
    }
    http.end();
  }

  graph.createSprite(G_WIDTH, G_HEIGHT);
  graph.fillSprite(TFT_BLACK);
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
  // 1. Update buffer for auto-scaling
  values[head] = rssi;
  head = (head + 1) % G_WIDTH;
  
  // 2. Find min/max for auto-scale
  float vMin = 100, vMax = -100;
  for(int i=0; i<G_WIDTH; i++) {
    if(values[i] < vMin) vMin = values[i];
    if(values[i] > vMax) vMax = values[i];
  }
  if (vMax == vMin) vMax = vMin + 1; // Prevent division by zero

  // 3. Scroll graph and draw new point
  graph.scroll(-1); // Shift left
  
  // Map current value to 0-20 pixel height
  int yPos = map(rssi, vMin, vMax, G_HEIGHT - 1, 0); 
  
  // 4. Color logic
  uint16_t color = TFT_GREEN;
  if (rssi < -70) color = TFT_YELLOW;
  if (rssi < -85) color = TFT_RED;

  // Draw the new point on the rightmost edge
  graph.drawFastVLine(G_WIDTH - 1, 0, G_HEIGHT, TFT_BLACK); // Clear column
  graph.drawPixel(G_WIDTH - 1, yPos, color);
  graph.pushSprite(0, tft.height()-20);

  // === TODO Stream to Aicuflow Backend ===

}