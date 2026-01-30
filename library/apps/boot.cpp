#include "boot.h"

Preferences bprefs;

void trackForAutoReset() {
  bprefs.begin("bootTrack", false);
  int counter = bprefs.getInt("counter", 0);
  counter++;  
  bprefs.putInt("counter", counter);
  bprefs.end();
  if (counter >= 3) { // tripple reboot detected
    bprefs.begin("bootTrack", false);
    bprefs.putInt("counter", 0);
    bprefs.end();
    // react by resetting settings to factory
    clearSettings();
    esp_restart();
  }
}

void clearAutoReset() {
  bprefs.begin("bootTrack", false);
  bprefs.putInt("counter", 0);
  bprefs.end();
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
  int topoffset = (device->kind_slug == "lilygo-t-display-s3") ? 6 : 0;
  tft.pushImage(screenWidth/2-126/2, topoffset, 126, 28, aicuflow_logo_wide);
  tft.setCursor(0, 32);
  if (device->kind_slug == "lilygo-t-display-s3") tft.println("");
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // details
  tft.println(en_de("Aicu IoT+AI Endpoint", "Aicu IoT+KI Endpoint"));
  tft.print(en_de("Board: ", "Board: ")); tft.println(device->kind_slug);
  tft.println("https://aicuflow.com");
  tft.println("");
  delay(duration);
}

void initSerial() {
  Serial.begin(115200); // Serial.begin(0); would try to detect
  // if (Serial.baudRate() == 0) // no rate detected
  while (!Serial && millis() < 200) delay(10);
  #if VERBOSE
    Serial.println("Hello World!");
    delay(500);
    Serial.print("Aicuflow booted on "); Serial.print(device->kind_slug); Serial.println("!");
  #endif
}

void initTFTScreen() {
  screenWidth  = tft.width();
  screenHeight = tft.height();
  #if VERBOSE
    Serial.print("Width: "); Serial.println(screenWidth);   // ttgo-t1: 135 
    Serial.print("Height: "); Serial.println(screenHeight); // ttgo-t1: 240
  #endif
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
    VOLTAGE_PIN = 34;
    pinMode(LEFT_BUTTON, INPUT_PULLUP);
    pinMode(RIGHT_BUTTON, INPUT);
  } else if (device->kind_slug == "lilygo-t-display-s3") {
    // voltage divider for measurement of U ? -> don't think

    // buttons
    LEFT_BUTTON = 0;
    RIGHT_BUTTON = 14;
    VOLTAGE_PIN = 4; // GPIO 4 for battery voltage; might need display on
    pinMode(LEFT_BUTTON, INPUT_PULLUP);
    pinMode(RIGHT_BUTTON, INPUT);

    // power up
    esp_wifi_set_max_tx_power(52); // ca 13dBm
  }
}

// boot fkts related to measurement displaying
void initSensorGraphs() {
  if (device->kind_slug == "lilygo-t-display-s3")
    sensors.setGraphSpacing(22, 5);
  else sensors.setGraphSpacing(14, 3);
  if (device->has_wifi && wifiAvailable){
    tft.setTextColor(TFT_CYAN);
    tft.print(en_de("Sending to ", "Sende an ")); tft.print(streamFileName); tft.println(".arrow");
    tft.setTextColor(TFT_WHITE);
  }
  else {
    tft.setTextColor(TFT_YELLOW);
    tft.println(en_de("Measuring without sending...", "Messung ohne zu senden..."));
    tft.setTextColor(TFT_WHITE);
  }
  sensors.initGraphs(&tft, screenWidth, screenHeight);
  #if VERBOSE
    Serial.print("Graphed sensors: ");
    Serial.println(sensors.getGraphCount());
  #endif
}