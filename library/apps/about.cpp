#include "about.h"

// About Page
void onAboutPageOpen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  int w = tft.width();
  int h = tft.height();

  int margin = device->kind_slug == "esp32-ttgo-t1" ? 0 : 12;

  tft.setCursor(margin, margin);

  int titleSize = (w >= 240) ? 2 : 1;

  // Title
  tft.setTextSize(titleSize);
  tft.println("Aicuflow");

  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.println("by AICU GmbH");
  tft.println();

  // Divider
  tft.drawFastHLine(margin, tft.getCursorY(), w - margin * 2, TFT_DARKGREY);
  tft.println();
  tft.println();

  // Description
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Create big data workflows &");
  tft.println("automations by chatting with AI.");
  tft.println("Train custom AI models.");
  tft.println("Deploy & scale with one click.");
  tft.println();

  // Divider
  tft.drawFastHLine(margin, tft.getCursorY(), w - margin * 2, TFT_DARKGREY);
  tft.println();
  tft.println();

  // Company details
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.println("AICU GmbH, Heilbronn");
  tft.println("HRB 794842 · Amtsgericht Stuttgart");
  tft.println("VAT: DE368976811");
  tft.println("CEO: Julia C. Yukovich");

  // Footer (explicit positioning only here)
  tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
  tft.setCursor(margin, h - 22);
  tft.print("Docs: aicuflow.com");

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(w - 90, h - 10);
  tft.print("Press any key");
}