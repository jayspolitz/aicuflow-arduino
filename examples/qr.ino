// https://github.com/yoprogramo/QRcode_eSPI/blob/c0c5827f772e3a6e9b93db44321ec0009feab919/src/qrcode_espi.cpp

#include <SPI.h>
#include "imports/TFT_eSPI/TFT_eSPI.cpp"
#include <qrcode_espi.h>
// needs QRcodeDisplay 2.1.0
// needs QRcode_eSPI 2.0.0

TFT_eSPI display = TFT_eSPI();
QRcode_eSPI qrcode (&display);


void setup() {

   display.fillScreen(TFT_BLACK);
   display.begin();
   qrcode.init();
   qrcode.create("https://example.com");
}

// Add this function to resolve the "undefined reference to loopv" error
void loop() {
   // Empty loop
}