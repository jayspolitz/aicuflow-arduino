#include "wifiscan.h"

int lastScanTime = 0;
int scanInterval = 5000; // scan every 5 seconds

void onWifiPageOpen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.println("Scanning Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

void onWifiPageUpdate() {
  if (millis() - lastScanTime > scanInterval) {
    int n = WiFi.scanNetworks();
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    if (n == 0) {
      tft.println("No networks found.");
    } else {
      for (int i = 0; i < n && i < 10; i++) {
        tft.printf("%d: %s (%d)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
      }
    }
    lastScanTime = millis();
  }
  closePageIfAnyButtonIsPressed();
}
