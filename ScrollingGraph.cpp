#include "ScrollingGraph.h"

ScrollingGraph::ScrollingGraph(TFT_eSPI* tft) : spr(tft), _count(0) {}

void ScrollingGraph::begin(int x, int y, int w, int h,
            float /*minVal*/, float /*maxVal*/,
            uint16_t color, const char* label) {
  _x = x; _y = y; _w = w; _h = h;
  _color = color; _label = label;

  spr.createSprite(w, h);
  spr.fillSprite(TFT_BLACK);

  _values = (float*)malloc(sizeof(float) * w);
  _count = 0;
}

void ScrollingGraph::update(float val) {
  // shift value buffer
  if (_count < _w) {
    _values[_count++] = val;
  } else {
    memmove(_values, _values + 1, sizeof(float) * (_w - 1));
    _values[_w - 1] = val;
  }

  // compute autoscale from buffer
  float minV = _values[0];
  float maxV = _values[0];
  for (int i = 1; i < _count; i++) {
    if (_values[i] < minV) minV = _values[i];
    if (_values[i] > maxV) maxV = _values[i];
  }

  float range = maxV - minV;
  if (range < 1e-6f) range = 1e-6f;

  // redraw everything
  spr.fillSprite(TFT_BLACK);

  for (int x = 1; x < _count; x++) {
    int y0 = map(
      _values[x - 1] * 1000,
      minV * 1000,
      maxV * 1000,
      _h - 1,
      0
    );
    int y1 = map(
      _values[x] * 1000,
      minV * 1000,
      maxV * 1000,
      _h - 1,
      0
    );

    if (y0 < 0) y0 = 0;
    if (y0 >= _h) y0 = _h - 1;
    if (y1 < 0) y1 = 0;
    if (y1 >= _h) y1 = _h - 1;

    spr.drawLine(x - 1, y0, x, y1, _color);
  }

  spr.pushSprite(_x, _y);
}
