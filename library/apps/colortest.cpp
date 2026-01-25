#include "colortest.h"
bool colorTestDrawn = false;

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Convert HSV to RGB (H: 0-360, S: 0-100, V: 0-100)
void hsvToRgb(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b) {
  s /= 100.0;
  v /= 100.0;
  
  float c = v * s;
  float x = c * (1 - abs(fmod(h / 60.0, 2) - 1));
  float m = v - c;
  
  float r1, g1, b1;
  if (h < 60) { r1 = c; g1 = x; b1 = 0; }
  else if (h < 120) { r1 = x; g1 = c; b1 = 0; }
  else if (h < 180) { r1 = 0; g1 = c; b1 = x; }
  else if (h < 240) { r1 = 0; g1 = x; b1 = c; }
  else if (h < 300) { r1 = x; g1 = 0; b1 = c; }
  else { r1 = c; g1 = 0; b1 = x; }
  
  r = (r1 + m) * 255;
  g = (g1 + m) * 255;
  b = (b1 + m) * 255;
}

void onColorTestPageOpen() {
  tft.fillScreen(TFT_BLACK);
  colorTestDrawn = false;
}

void onColorTestPageUpdate() {
  if (!colorTestDrawn) {
    for (int y = 0; y < screenHeight; y++) {
      float v = map(y, 0, screenHeight - 1, 100, 0); // brightness: top=bright, bottom=dark
      for (int x = 0; x < screenWidth; x++) {
        float h = map(x, 0, screenWidth - 1, 0, 360); // hue: full spectrum left to right
        uint8_t r, g, b;
        hsvToRgb(h, 100, v, r, g, b); // full saturation
        tft.drawPixel(x, y, rgb565(r, g, b));
      }
    }
    colorTestDrawn = true;
  }
  closePageIfAnyButtonIsPressed();
}