#include "qrcode.h"

QRCode qrCodeQR;
uint8_t* qrCodeData = nullptr; // dynamically allocated
const char* qrCodeText = "https://aicuflow.com";
const char* qrCodeLabel = "aicuflow.com";

bool qrCodeInverted = false;

void drawQRCodeScreen() {
  // Set screen background and text colors based on inversion
  uint32_t bgColor = qrCodeInverted ? TFT_WHITE : TFT_BLACK;
  uint32_t fgColor = qrCodeInverted ? TFT_BLACK : TFT_WHITE;

  tft.fillScreen(bgColor);
  tft.setTextColor(fgColor, bgColor);
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.println(en_de("Generating QR Code...", "QR-Code wird erstellt..."));

  // Allocate buffer dynamically if not yet allocated
  if (!qrCodeData) {
    qrCodeData = (uint8_t*)malloc(qrcode_getBufferSize(3));
    if (!qrCodeData) {
      tft.println(en_de("Memory allocation failed", "Speicherallokierung fehlgeschlagen"));
      return;
    }
  }

  // Initialize QR code
  qrcode_initText(&qrCodeQR, qrCodeData, 3, 0, qrCodeText);

  // Scale QR code to nearly fill screen with ~2% padding
  int maxQRSize = min(screenWidth, screenHeight) * 0.96;
  int scale = maxQRSize / qrCodeQR.size;
  int qrPixelSize = qrCodeQR.size * scale;
  int xStart = (screenWidth - qrPixelSize) / 2;
  int yStart = (screenHeight - qrPixelSize) / 2 - 10;

  // Draw QR code with color inverted or normal
  for (uint8_t y = 0; y < qrCodeQR.size; y++) {
    for (uint8_t x = 0; x < qrCodeQR.size; x++) {
      uint32_t color = qrcode_getModule(&qrCodeQR, x, y) ? fgColor : bgColor;
      tft.fillRect(xStart + x * scale, yStart + y * scale, scale, scale, color);
    }
  }

  // Draw centered label below QR code
  tft.setTextDatum(TC_DATUM);
  tft.drawString(qrCodeLabel, screenWidth / 2, yStart + qrPixelSize + 5);
}

void onQRCodePageOpen() {
  drawQRCodeScreen();
}

void onQRCodePageUpdate() {
  if (digitalRead(LEFT_BUTTON) == LOW) {
    qrCodeInverted = !qrCodeInverted;
    drawQRCodeScreen(); // redraw whole screen inverted
    delay(200); // debounce
  }

  if (digitalRead(RIGHT_BUTTON) == LOW) {
    closePageIfAnyButtonIsPressed(); // dismiss page
  }
}
