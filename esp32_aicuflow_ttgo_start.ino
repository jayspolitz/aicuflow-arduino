#include <TFT_eSPI.h> // Graphics and font library by "Bodmer" install v2.5.43 before anything, maybe needs to be ported in
#include <SPI.h>
#include "aicuflow_logo_64px.h" // aicuflow_logo
#include "aicuflow_logo_wide.h" // aicuflow_logo_wide

TFT_eSPI tft = TFT_eSPI();

void setup() {
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
  
  delay(5000);

  // = boot screen 2
  tft.setRotation(0); // Set vertical orientation
  tft.setCursor(0, 0);
  tft.fillScreen(TFT_BLACK);
  tft.pushImage(screenWidth/2-32, screenHeight/2-32, 64, 64, aicuflow_logo);
  
  delay(5000);

  // = screen information
  tft.setRotation(0); // Set vertical orientation
  tft.setCursor(0, 0);
  tft.fillScreen(TFT_BLACK);
  tft.pushImage(screenWidth/2-32, 6, 64, 64, aicuflow_logo);
  tft.setCursor(0, 75);
  tft.setTextColor(TFT_AICU_TEAL, TFT_BLACK); 
  tft.setTextSize(2);
  // tft.invertDisplay(true);
  tft.println("#aicuflow32");
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("ESP32 IoT-AI Endpoint");
  tft.println("https://aicuflow.com");
  tft.println("");
  tft.println("");
  tft.println("Device started...");


  // === TODO WIFI Setup ===
}

void loop() {
  Serial.print("Millis: ");
  Serial.println(millis());
  delay(1000);
  // Add dynamic content here
}