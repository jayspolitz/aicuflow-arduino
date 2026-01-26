#include "aicuflow_tutorial.h"

static int aicuflowTutorialPage = 0;
static const int aicuflowTutorialPageCount = 7;
static unsigned long aicuflowTutorialLastInput = 0;

static int aicuflowTutorialSidePadding() {
  return screenWidth < 200 ? 12 : 20;
}

static void aicuflowTutorialDrawDots() {
  int dotCount = aicuflowTutorialPageCount;
  int r = 3;
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
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(en_de("Links / Rechts", "Links / Rechts"),
                 screenWidth / 2,
                 screenHeight - 26);
  tft.setTextDatum(TL_DATUM);
}

static void aicuflowTutorialTitle(const char* txt) {
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
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
  aicuflowTutorialTitle("Aicuflow IoT");

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  int y = aicuflowTutorialBodyStartY(3);

  aicuflowTutorialBodyLine(
    en_de("Open-source data pipelines",
          "Open-Source Datenpipelines"), y);
  y += 16;

  aicuflowTutorialBodyLine(
    en_de("From devices to insight",
          "Von Geraeten zu Erkenntnis"), y);
}

static void aicuflowTutorialPage1() {
  aicuflowTutorialTitle(en_de("Use-Cases", "Anwendungsfaelle"));

  tft.setTextSize(1);
  int y = aicuflowTutorialBodyStartY(5);

  aicuflowTutorialBodyLine("- Sensor Streaming", y); y += 14;
  aicuflowTutorialBodyLine("- Proof of Concepts", y); y += 14;
  aicuflowTutorialBodyLine("- IoT Mess-Tools", y);
}

static void aicuflowTutorialPage2() {
  aicuflowTutorialTitle(en_de("Data Flow", "Datenfluss"));

  tft.setTextSize(1);
  int y = aicuflowTutorialBodyStartY(4);

  aicuflowTutorialBodyLine(
    en_de("Device sends live data",
          "Geraet sendet Live-Daten"), y); y += 14;

  aicuflowTutorialBodyLine(
    en_de("Aicuflow stores streams",
          "Aicuflow speichert Streams"), y); y += 14;

  aicuflowTutorialBodyLine(
    en_de("AI can be trained",
          "KI kann trainiert werden"), y);
}

static void aicuflowTutorialPage3() {
  aicuflowTutorialTitle(en_de("Measurement", "Messung"));

  tft.setTextSize(1);
  int y = aicuflowTutorialBodyStartY(4);

  aicuflowTutorialBodyLine(
    en_de("Main application",
          "Hauptanwendung"), y); y += 14;

  aicuflowTutorialBodyLine(
    en_de("All sensors & WiFi stats",
          "Alle Sensor- & WLAN-Daten"), y); y += 14;

  aicuflowTutorialBodyLine(
    en_de("Sent in batches",
          "Gebuendelt gesendet"), y);
}

static void aicuflowTutorialPage4() {
  aicuflowTutorialTitle(en_de("Setup", "Einrichtung"));

  tft.setTextSize(1);
  int y = aicuflowTutorialBodyStartY(5);

  aicuflowTutorialBodyLine("Settings > WiFi", y); y += 14;
  aicuflowTutorialBodyLine("Settings > API", y); y += 14;

  aicuflowTutorialBodyLine(
    en_de("User, password, flow id",
          "Benutzer, Passwort, Flow-ID"), y);
}

static void aicuflowTutorialPage5() {
  aicuflowTutorialTitle(en_de("Controls", "Bedienung"));

  tft.setTextSize(1);
  int y = aicuflowTutorialBodyStartY(6);

  aicuflowTutorialBodyLine(
    en_de("Left: up / previous",
          "Links: hoch / zurueck"), y); y += 14;

  aicuflowTutorialBodyLine(
    en_de("Right: down / next",
          "Rechts: runter / weiter"), y); y += 14;

  aicuflowTutorialBodyLine(
    en_de("Press both to confirm",
          "Beide druecken = bestaetigen"), y);
}

static void aicuflowTutorialPage6() {
  aicuflowTutorialTitle(en_de("More", "Mehr"));

  tft.setTextSize(1);
  int y = aicuflowTutorialBodyStartY(5);

  aicuflowTutorialBodyLine(
    en_de("WiFi & Bluetooth tools",
          "WLAN- & Bluetooth-Tools"), y); y += 14;

  aicuflowTutorialBodyLine(
    en_de("Games and test screens",
          "Spiele und Testanzeigen"), y); y += 18;

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  aicuflowTutorialBodyLine("Finn Glas @AICU GmbH", y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

static void aicuflowTutorialRender() {
  tft.fillScreen(TFT_BLACK);

  switch (aicuflowTutorialPage) {
    case 0: aicuflowTutorialPage0(); break;
    case 1: aicuflowTutorialPage1(); break;
    case 2: aicuflowTutorialPage2(); break;
    case 3: aicuflowTutorialPage3(); break;
    case 4: aicuflowTutorialPage4(); break;
    case 5: aicuflowTutorialPage5(); break;
    case 6: aicuflowTutorialPage6(); break;
  }

  aicuflowTutorialDrawHint();
  aicuflowTutorialDrawDots();
}

void onAicuflowTutorialPageOpen() {
  aicuflowTutorialPage = 0;
  aicuflowTutorialRender();
}

void onAicuflowTutorialPageUpdate() {
  if (millis() - aicuflowTutorialLastInput < 180) return;

  if (digitalRead(LEFT_BUTTON) == LOW) {
    if (aicuflowTutorialPage > 0) {
      aicuflowTutorialPage--;
      aicuflowTutorialRender();
    }
    aicuflowTutorialLastInput = millis();
  }

  if (digitalRead(RIGHT_BUTTON) == LOW) {
    if (aicuflowTutorialPage < aicuflowTutorialPageCount - 1) {
      aicuflowTutorialPage++;
      aicuflowTutorialRender();
    } else {
      closePageIfAnyButtonIsPressed();
    }
    aicuflowTutorialLastInput = millis();
  }
}
