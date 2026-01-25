#include "colorwheeltest.h"
#include "colortest.h"

// this one looks kinda cool, so finn kept both.

bool wheelColorTestDrawn = false;

void onColorWheelTestPageOpen() {
  tft.fillScreen(TFT_BLACK);
  wheelColorTestDrawn = false;
}

void onColorWheelTestPageUpdate() {
  if (!wheelColorTestDrawn) {
    for (int y = 0; y < screenHeight; y++) {
      uint8_t g = map(y, 0, screenHeight - 1, 0, 255);
      for (int x = 0; x < screenWidth; x++) {
        uint8_t r = map(x, 0, screenWidth - 1, 0, 255);
        uint8_t b = 128; // fixed blue for clean RG gradient
        tft.drawPixel(x, y, rgb565(r, g, b));
      }
    }
    wheelColorTestDrawn = true;
  }
  closePageIfAnyButtonIsPressed();
}