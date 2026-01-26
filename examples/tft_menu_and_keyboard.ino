/**
 *  Aicuflow ESP32 TFTMenu + TFTKeyboard Demo
 *
 *  Created: 20260125, by Finn Glas @ AICU GmbH
 *  
 *  This example shows how to combine TFTMenu with TFTKeyboard:
 *  Navigate through a menu and open the keyboard to enter text!
 *  
 *  Controls:
 *  - Left Button: Previous item / Previous key
 *  - Right Button: Next item / Next key
 *  - Both Buttons: Select / Confirm
 *  
 *  Check https://aicuflow.com/docs/library/arduino for more!
 */

#include "imports/TFT_eSPI/TFT_eSPI.cpp"
#include "library/graphics/TFTMenuModern.cpp"
#include "library/graphics/TFTKeyboard.cpp"

TFT_eSPI tft = TFT_eSPI();

#define LEFT_BUTTON  0
#define RIGHT_BUTTON 35

// Menu and Keyboard objects
TFTMenu* mainMenu;
TFTKeyboard* keyboard;

// Page management
enum Page {
  PAGE_MENU,
  PAGE_KEYBOARD,
  PAGE_CUSTOM
};

Page currentPage = PAGE_MENU;

// Simple flag to block input after transitions
bool blockInput = false;
unsigned long blockInputUntil = 0;

// Store entered text
String userName = "";
String userMessage = "";

// Track which keyboard is currently active
enum KeyboardContext {
  CONTEXT_NAME,
  CONTEXT_MESSAGE
};

KeyboardContext currentKeyboardContext;

// ========================================
// INPUT BLOCKING HELPERS
// ========================================

void blockInputFor(unsigned long ms) {
  blockInput = true;
  blockInputUntil = millis() + ms;
}

bool isInputBlocked() {
  if (blockInput && millis() >= blockInputUntil) {
    blockInput = false;
  }
  return blockInput;
}

// ========================================
// KEYBOARD HANDLERS
// ========================================

void openNameKeyboard() {
  currentKeyboardContext = CONTEXT_NAME;
  currentPage = PAGE_KEYBOARD;
  keyboard->setText(userName);
  tft.fillScreen(TFT_BLACK);
  keyboard->begin();
  blockInputFor(300);
}

void openMessageKeyboard() {
  currentKeyboardContext = CONTEXT_MESSAGE;
  currentPage = PAGE_KEYBOARD;
  keyboard->setText(userMessage);
  tft.fillScreen(TFT_BLACK);
  keyboard->begin();
  blockInputFor(300);
}

void onTextConfirmed(String text) {
  // Store the text
  if (currentKeyboardContext == CONTEXT_NAME) {
    userName = text;
    Serial.println("Saved name: " + userName);
  } else if (currentKeyboardContext == CONTEXT_MESSAGE) {
    userMessage = text;
    Serial.println("Saved message: " + userMessage);
  }
  
  // Return to menu
  currentPage = PAGE_MENU;
  tft.fillScreen(TFT_BLACK);
  mainMenu->draw();
  blockInputFor(300);
}

// ========================================
// MENU ACTIONS
// ========================================

void viewStoredData() {
  currentPage = PAGE_CUSTOM;
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TL_DATUM);
  
  tft.drawString("STORED DATA", 10, 20, 2);
  tft.drawLine(0, 40, tft.width(), 40, TFT_DARKGREY);
  
  tft.drawString("Name:", 10, 50, 2);
  tft.drawString(userName.isEmpty() ? "(none)" : userName, 10, 70, 2);
  
  tft.drawString("Message:", 10, 100, 2);
  
  // Wrap message text
  int y = 120;
  String msg = userMessage.isEmpty() ? "(none)" : userMessage;
  
  int start = 0;
  while (start < msg.length()) {
    int end = start + 25;
    if (end > msg.length()) end = msg.length();
    
    if (end < msg.length()) {
      int lastSpace = msg.lastIndexOf(' ', end);
      if (lastSpace > start) end = lastSpace;
    }
    
    tft.drawString(msg.substring(start, end), 10, y, 2);
    y += 20;
    start = end + 1;
    
    if (y > tft.height() - 40) break;
  }
  
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Press any button to return", tft.width() / 2, tft.height() - 20, 1);
  
  blockInputFor(300);
}

void clearStoredData() {
  userName = "";
  userMessage = "";
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Data Cleared!", tft.width() / 2, tft.height() / 2, 2);
  
  delay(1000);
  
  currentPage = PAGE_MENU;
  tft.fillScreen(TFT_BLACK);
  mainMenu->draw();
  blockInputFor(500);
}

void aboutScreen() {
  currentPage = PAGE_CUSTOM;
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  
  tft.drawString("TFTMenu + Keyboard", tft.width() / 2, 30, 2);
  tft.drawString("Demo v1.0", tft.width() / 2, 55, 2);
  tft.drawLine(10, 75, tft.width() - 10, 75, TFT_DARKGREY);
  
  tft.setTextDatum(TL_DATUM);
  tft.drawString("by AICU GmbH", 10, 90, 1);
  tft.drawString("aicuflow.com", 10, 105, 1);
  
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Press any button", tft.width() / 2, tft.height() - 20, 1);
  
  blockInputFor(300);
}

// ========================================
// CUSTOM PAGE UPDATE
// ========================================

void updateCustomPage() {
  if (isInputBlocked()) return;
  
  if (digitalRead(LEFT_BUTTON) == LOW || digitalRead(RIGHT_BUTTON) == LOW) {
    // Wait for release
    while (digitalRead(LEFT_BUTTON) == LOW || digitalRead(RIGHT_BUTTON) == LOW) {
      delay(10);
    }
    
    currentPage = PAGE_MENU;
    tft.fillScreen(TFT_BLACK);
    mainMenu->draw();
    blockInputFor(300);
  }
}

// ========================================
// SETUP
// ========================================

void setup() {
  Serial.begin(115200);
  
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  Serial.println("======================");
  Serial.println("TFTMenu + Keyboard Demo");
  Serial.println("======================");
  
  // Create main menu
  mainMenu = new TFTMenu(&tft, "Main Menu");
  mainMenu->addItem("Enter Name", openNameKeyboard);
  mainMenu->addItem("Enter Message", openMessageKeyboard);
  mainMenu->addItem("View Data", viewStoredData);
  mainMenu->addItem("Clear Data", clearStoredData);
  mainMenu->addItem("About", aboutScreen);
  mainMenu->propagateButtonPins(LEFT_BUTTON, RIGHT_BUTTON);
  
  // Create keyboard
  keyboard = new TFTKeyboard(&tft, "Enter Text");
  keyboard->setButtonPins(LEFT_BUTTON, RIGHT_BUTTON);
  keyboard->setOnConfirm(onTextConfirmed);
  
  mainMenu->begin();
  
  Serial.println("Ready!");
}

// ========================================
// MAIN LOOP
// ========================================

void loop() {
  if (isInputBlocked()) {
    delay(20);
    return;
  }
  
  if (currentPage == PAGE_MENU) {
    mainMenu->update();
  } else if (currentPage == PAGE_KEYBOARD) {
    keyboard->update();
  } else if (currentPage == PAGE_CUSTOM) {
    updateCustomPage();
  }
  
  delay(20);
}
/**
 * USAGE FLOW:
 * 
 * 1. Navigate Main Menu:
 *    - Use Left/Right buttons to browse menu items
 *    - Press Both buttons to select
 * 
 * 2. Enter Name:
 *    - Opens keyboard
 *    - Navigate with Left/Right, select with Both
 *    - Use ABC for uppercase, 123 for special chars
 *    - Press OK when done
 * 
 * 3. Enter Message:
 *    - Same keyboard interface
 *    - Can enter longer text
 * 
 * 4. View Data:
 *    - Shows stored name and message
 *    - Press any button to return to menu
 * 
 * 5. Clear Data:
 *    - Erases stored name and message
 *    - Returns to menu automatically
 * 
 * 6. About:
 *    - Shows app information
 *    - Press any button to return
 */