#include "aicuflow_tutorial.h"

static int aicuflowTutorialPage = 0;
static const int aicuflowTutorialPageCount = 4;
static unsigned long aicuflowTutorialLastInput = 0;
static bool aicuflowTutorialLeftWasDown = false;
static bool aicuflowTutorialRightWasDown = false;

static int aicuflowTutorialSidePadding() {
  return screenWidth < 200 ? 12 : 20;
}

static void aicuflowTutorialDrawDots() {
  int dotCount = aicuflowTutorialPageCount;
  int r = 5;
  int spacing = 12;
  int totalW = dotCount * (2 * r) + (dotCount - 1) * spacing;
  int x0 = (screenWidth - totalW) / 2;
  int y = screenHeight - 14;

  for (int i = 0; i < dotCount; i++) {
    int x = x0 + i * (2 * r + spacing) + r;
    if (i == aicuflowTutorialPage)
      tft.fillCircle(x, y, r, TFT_CYAN);
    else
      tft.drawCircle(x, y, r, TFT_DARKGREY);
  }
}

static void aicuflowTutorialDrawHint() {
  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  if (aicuflowTutorialPage == 0)
    tft.drawString(en_de("Press right button", "Drücke rechten Knopf"),
                 screenWidth / 2,
                 screenHeight - 26);
  else if (aicuflowTutorialPage == aicuflowTutorialPageCount - 1)
    tft.drawString(en_de("Finish tutorial", "Tutorial beenden"),
                 screenWidth / 2,
                 screenHeight - 26);
  else tft.drawString(en_de("< Left | Right >", "< Links | Rechts >"),
                 screenWidth / 2,
                 screenHeight - 26);
  tft.setTextDatum(TL_DATUM);
}

static void aicuflowTutorialTitle(const char* txt) {
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(txt, screenWidth / 2, 22);
  tft.setTextDatum(TL_DATUM);
}

static int aicuflowTutorialBodyStartY(int lineCount) {
  int h = lineCount * (tft.fontHeight() + 2);
  int y = (screenHeight - h) / 2;
  if (y < 60) y = 60;
  return y;
}

static void aicuflowTutorialBodyLine(const char* txt, int y) {
  int pad = aicuflowTutorialSidePadding();
  tft.setCursor(pad, y);
  tft.print(txt);
}

static void aicuflowTutorialPage0() {
  aicuflowTutorialTitle(en_de("Welcome!", "Willkommen"));
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  int y = aicuflowTutorialBodyStartY(7);
  aicuflowTutorialBodyLine(en_de("Welcome to Aicuflow!", "Willkommen bei AICU!"), y); y += 16;
  aicuflowTutorialBodyLine("", y); y += 16;
  aicuflowTutorialBodyLine(en_de("This is your IoT+AI PoC!", "Hier ist dein IoT+KI PoC!"), y); y += 16;
  aicuflowTutorialBodyLine(en_de("Measure sensor data,", "Sensordaten messen,"), y); y += 16;
  aicuflowTutorialBodyLine(en_de("Stream to the cloud,", "In der Cloud sammeln,"), y);y += 16;
  aicuflowTutorialBodyLine(en_de("Plot and analyze it,", "Plotten, analysieren,"), y);y += 16;
  aicuflowTutorialBodyLine(en_de("or train your AI!", "oder KI trainieren!"), y);y += 16;
}

static void aicuflowTutorialPage1() {
  aicuflowTutorialTitle(en_de("Controls", "Steuerung"));
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  int y = aicuflowTutorialBodyStartY(3);
  aicuflowTutorialBodyLine(en_de("Left = Up", "L. = Hoch"), y); y += 24;
  aicuflowTutorialBodyLine(en_de("Right = Down", "R. = Runter"), y); y += 24;
  aicuflowTutorialBodyLine(en_de("Both = Ok", "Beide = Ok"), y); y += 24;
}

static void aicuflowTutorialPage2() {
  aicuflowTutorialTitle(en_de("Setup", "Setup"));
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  int y = aicuflowTutorialBodyStartY(8);
  aicuflowTutorialBodyLine(en_de("How to start:", "So startest du:"), y); y += 16;
  aicuflowTutorialBodyLine(en_de("1. Setup > Wifi", "1. Setup > Wlan"), y); y += 16;
  aicuflowTutorialBodyLine(en_de("2. Setup > API", "2. Setup > API"), y); y += 16;
  aicuflowTutorialBodyLine(en_de("3. Start", "3. Start"), y); y += 16;
  aicuflowTutorialBodyLine(en_de("4. Data is sent", "4. Es sendet Daten"), y); y += 16;
  aicuflowTutorialBodyLine(en_de("5. Check your flow", "5. Flow ansehen"), y); y += 16;
  aicuflowTutorialBodyLine(en_de("6. Analyze data", "6. Daten analysieren"), y); y += 16;
}

static void aicuflowTutorialPage3() {
  aicuflowTutorialTitle(en_de("Let's go", "Los gehts"));
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  int y = aicuflowTutorialBodyStartY(4);
  aicuflowTutorialBodyLine(en_de("You are ready!", "Du bist ready!"), y); y += 16;
  aicuflowTutorialBodyLine(en_de("Check out tools!", "Teste die Tools!"), y); y += 16;
  aicuflowTutorialBodyLine("-Finn @AICU GmbH", y); y += 16;
}

static void aicuflowTutorialRender() {
  tft.fillScreen(TFT_BLACK);

  switch (aicuflowTutorialPage) {
    case 0: aicuflowTutorialPage0(); break;
    case 1: aicuflowTutorialPage1(); break;
    case 2: aicuflowTutorialPage2(); break;
    case 3: aicuflowTutorialPage3(); break;
  }

  aicuflowTutorialDrawHint();
  aicuflowTutorialDrawDots();
}

void onAicuflowTutorialPageOpen() {
  aicuflowTutorialPage = 0;
  aicuflowTutorialRender();
}
void onAicuflowTutorialPageUpdate() {
  bool leftDown  = (digitalRead(LEFT_BUTTON)  == LOW);
  bool rightDown = (digitalRead(RIGHT_BUTTON) == LOW);

  // LEFT: previous page (on release)
  if (!leftDown && aicuflowTutorialLeftWasDown) {
    if (aicuflowTutorialPage > 0) {
      aicuflowTutorialPage--;
      aicuflowTutorialRender();
    }
  }

  // RIGHT: immediate exit on last page (on press)
  if (rightDown && !aicuflowTutorialRightWasDown &&
      aicuflowTutorialPage == aicuflowTutorialPageCount - 1) {
    closePageIfAnyButtonIsPressed();
    return;
  }

  // RIGHT: next page (on release)
  if (!rightDown && aicuflowTutorialRightWasDown) {
    if (aicuflowTutorialPage < aicuflowTutorialPageCount - 1) {
      aicuflowTutorialPage++;
      aicuflowTutorialRender();
    }
  }

  aicuflowTutorialLeftWasDown  = leftDown;
  aicuflowTutorialRightWasDown = rightDown;
}
