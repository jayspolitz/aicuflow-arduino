/**
 *  Aicuflow ESP32 TFTKeyboard Demo
 *
 *  Created: 20260125, by Finn Glas @ AICU GmbH
 *  
 *  This example shows how to use TFTKeyboard:
 *  a beautiful on-screen keyboard for text input
 *  with your ESP32 and TFT display!
 *  
 *  Controls:
 *  - Left Button: Previous key
 *  - Right Button: Next key
 *  - Both Buttons: Select/Confirm
 *  
 *  Check https://aicuflow.com/docs/library/arduino for more!
 */

#include "imports/TFT_eSPI/TFT_eSPI.cpp"
#include "library/graphics/TFTKeyboard.cpp"

TFT_eSPI tft = TFT_eSPI();

#define LEFT_BUTTON  0 // these are for ttgo-t1
#define RIGHT_BUTTON 35

TFTKeyboard* keyboard;

// Callback function when text is confirmed
void onTextConfirmed(String text) {
  Serial.println(text);
  
  // You can do whatever you want with the confirmed text here
  // For example: send it over WiFi, save to SD card, etc.
}

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  tft.init();
  tft.setRotation(0);  // Adjust rotation as needed
  tft.fillScreen(TFT_BLACK);
  
  // Create keyboard
  keyboard = new TFTKeyboard(&tft, "Your Name");
  
  // Set button pins
  keyboard->setButtonPins(LEFT_BUTTON, RIGHT_BUTTON);
  
  // Optional: set custom debounce delay (default is 200ms)
  // keyboard->setDebounceDelay(150);
  
  // Set callback for when user presses OK/Confirm
  keyboard->setOnConfirm(onTextConfirmed);
  
  // Start keyboard
  keyboard->begin();
  
  Serial.println("TFTKeyboard initialized!");
  Serial.println("Use buttons to navigate and enter text.");
}

void loop() {
  // Update keyboard (handles button presses)
  keyboard->update();
  
  delay(20);
}

/**
 * USAGE TIPS:
 * 
 * 1. Navigate through keys:
 *    - Press LEFT button to go to previous key
 *    - Press RIGHT button to go to next key
 *    - Keys wrap around (last -> first, first -> last)
 * 
 * 2. Select a key:
 *    - Press BOTH buttons simultaneously
 * 
 * 3. Keyboard modes:
 *    - Default: lowercase letters (a-z)
 *    - Press "ABC" to toggle UPPERCASE mode
 *    - Press "123" to toggle SPECIAL characters mode
 *    - Active modes are shown with white background
 * 
 * 4. Special keys:
 *    - SPACE: adds a space
 *    - DEL: deletes last character
 *    - CLR: clears entire text
 *    - OK: confirms and prints text to Serial
 * 
 * 5. Get/Set text programmatically:
 *    String text = keyboard->getText();
 *    keyboard->setText("Hello");
 */