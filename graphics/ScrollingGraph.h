#pragma once
#include <TFT_eSPI.h>

// fallback to have tft backlight pin even if stuff is not defined
#ifndef TFT_BL
#define TFT_BL 42
#endif

class ScrollingGraph {
private:
  TFT_eSprite spr;
  int _x, _y, _w, _h;
  uint16_t _color;
  const char* _label;
  float* _values;
  int _count;
  
public:
  ScrollingGraph(TFT_eSPI* tft);
  void begin(int x, int y, int w, int h, float minVal, float maxVal, uint16_t color, const char* label);
  void update(float val);
};