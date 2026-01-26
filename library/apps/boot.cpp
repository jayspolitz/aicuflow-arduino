#include "boot.h"

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
  int topoffset = (device->kind_slug == "lilygo-t-display-s3") ? 6 : 0;
  tft.pushImage(screenWidth/2-126/2, topoffset, 126, 28, aicuflow_logo_wide);
  tft.setCursor(0, 32);
  if (device->kind_slug == "lilygo-t-display-s3") tft.println("");
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // details
  tft.println(en_de("Aicu IoT+AI Endpoint", "Aicu IoT+KI Endpoint"));
  tft.print(en_de("Hardware: ", "Hardware: ")); tft.println(device->kind_slug);
  tft.println("https://aicuflow.com");
  tft.println("");
  delay(duration);
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