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
  tft.println("Aicu IoT-AI Endpoint");
  tft.println("https://aicuflow.com");
  tft.println("");
  tft.println("");
  tft.println("Device started...");
  delay(duration);
}