#include "about.h"

// Random Colors Page
void onRandomPageUpdate() {
  int w = tft.width(), h = tft.height();
  // Seed randomness (ESP32 has HW RNG, this mixes it)
  randomSeed(esp_random());
  tft.startWrite();
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      tft.drawPixel(x, y, random(0xFFFF)); // full 16-bit RGB565 space
    }
  }
  tft.endWrite();
  // Footer (explicit positioning)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(4, h - 10);
  tft.print("Press any key");
}