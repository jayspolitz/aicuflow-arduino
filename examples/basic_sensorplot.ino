/**
 *  Aicuflow AI Flow Builder + Arduino ESP32 Quickstart
 *  Created: 20260122, by Finn Glas @ AICU GmbH
 *  Simple sensor graphing example.
 *  Uses ESPI v2.5.43 Bodmer, which needs customisation.
 *  Check https://aicuflow.com/docs/library/arduino for more!
 */

#include <TFT_eSPI.h>
#include "sensors/SensorMeasurement.cpp"

bool isTDisplayS3 = true;
int leftButton = 0;
int rightButton = isTDisplayS3 ? 14 : 35;

TFT_eSPI tft = TFT_eSPI();
SensorMeasurement sensors("dev");

void setup() {
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(1);
  tft.setCursor(10, 10);
  tft.println("Aicuflow Sensor Plot (left & right button)");

  pinMode(leftButton, INPUT_PULLUP);
  sensors.registerSensor("left_button", "Button L", [&]() { return digitalRead(leftButton) == 0 ? 1.0 : 0.0; }, 0, 1, TFT_PURPLE, LOG_SEND, SHOW_GRAPH);
  
  pinMode(rightButton, INPUT);
  sensors.registerSensor("right_button", "Button R", [&]() { return digitalRead(rightButton) == 0 ? 1.0 : 0.0; }, 0, 1, TFT_BLUE, LOG_SEND, SHOW_GRAPH);

  sensors.setGraphSpacing(tft.height()/2-2*tft.height()/10, tft.height()/10);
  sensors.initGraphs(&tft, tft.width(), tft.height());
}

void loop() {
  delay(100);
  sensors.measure();
  sensors.updateGraphs();
}
