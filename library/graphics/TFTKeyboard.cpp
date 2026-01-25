#include "TFTKeyboard.h"

TFTKeyboard::TFTKeyboard(TFT_eSPI* display, String keyboardTitle) {
  tft = display;
  title = keyboardTitle;
  selectedIndex = 0;
  currentMode = MODE_LOWERCASE;
  currentText = "";
  
  // Colors - minimal black & white
  bgColor = TFT_BLACK;
  textColor = TFT_WHITE;
  selectedBorderColor = TFT_WHITE;
  activeModeColor = TFT_WHITE;
  
  // Display settings
  screenWidth = tft->width();
  screenHeight = tft->height();
  
  // Adaptive sizing based on screen height
  if (screenHeight <= 128) {
    // Very small screens (e.g., 128x128)
    headerHeight = 20;
    textDisplayHeight = 25;
    keyHeight = 18;
    keyPadding = 3;
  } else if (screenHeight <= 160) {
    // Small screens (e.g., 135x240 TTGO T1)
    headerHeight = 25;
    textDisplayHeight = 30;
    keyHeight = 22;
    keyPadding = 4;
  } else if (screenHeight <= 240) {
    // Medium screens
    headerHeight = 30;
    textDisplayHeight = 35;
    keyHeight = 28;
    keyPadding = 5;
  } else {
    // Large screens (320x240 and above)
    headerHeight = 35;
    textDisplayHeight = 40;
    keyHeight = 32;
    keyPadding = 6;
  }
  
  keysPerRow = 5;
  
  // Calculate key dimensions based on screen size
  int availableWidth = screenWidth - (keyPadding * (keysPerRow + 1));
  keyWidth = availableWidth / keysPerRow;
  
  // Calculate how many rows we can fit
  int keyboardStartY = headerHeight + textDisplayHeight;
  int availableHeight = screenHeight - keyboardStartY - keyPadding;
  int maxRows = availableHeight / (keyHeight + keyPadding);
  
  // Total keys we need: 26 letters + space + 2 mode toggles + 3 actions = 32 keys
  // That's 7 rows with 5 keys per row (35 total slots, some empty)
  // If we can't fit that, reduce key height further
  int neededRows = 7;
  if (maxRows < neededRows) {
    keyHeight = (availableHeight / neededRows) - keyPadding;
    // Minimum key height
    if (keyHeight < 16) {
      keyHeight = 16;
    }
  }
  
  // Button settings
  leftButtonPin = -1;
  rightButtonPin = -1;
  lastDebounceTime = 0;
  debounceDelay = 200;
  bothPressStartTime = 0;
  actionExecuted = false;
  
  initKeys();
}

void TFTKeyboard::initKeys() {
  // Clear existing keys
  for (auto key : keys) {
    delete key;
  }
  keys.clear();
  
  // Lowercase letters
  const char* lowercase = "abcdefghijklmnopqrstuvwxyz";
  for (int i = 0; i < 26; i++) {
    String c = String(lowercase[i]);
    keys.push_back(new Key(c, c));
  }
  
  // Space
  keys.push_back(new Key("SPC", " "));
  
  // Mode toggles
  keys.push_back(new Key("123", KEY_MODE_TOGGLE, MODE_SPECIAL));
  keys.push_back(new Key("ABC", KEY_MODE_TOGGLE, MODE_UPPERCASE));
  
  // Actions
  keys.push_back(new Key("DEL", KEY_DELETE));
  keys.push_back(new Key("CLR", KEY_CLEAR));
  keys.push_back(new Key("OK", KEY_CONFIRM));
}

void TFTKeyboard::setButtonPins(int leftPin, int rightPin) {
  leftButtonPin = leftPin;
  rightButtonPin = rightPin;
  
  if (leftButtonPin >= 0) {
    pinMode(leftButtonPin, INPUT_PULLUP);
  }
  if (rightButtonPin >= 0) {
    pinMode(rightButtonPin, INPUT_PULLUP);
  }
}

void TFTKeyboard::setDebounceDelay(unsigned long delay) {
  debounceDelay = delay;
}

void TFTKeyboard::setOnConfirm(std::function<void(String)> callback) {
  onConfirm = callback;
}

void TFTKeyboard::begin() {
  tft->fillScreen(bgColor);
  draw();
}

void TFTKeyboard::draw() {
  tft->fillScreen(bgColor);
  drawHeader();
  drawTextDisplay();
  
  // Draw all keys
  for (int i = 0; i < keys.size(); i++) {
    int x, y;
    calculateKeyPosition(i, x, y);
    drawKey(i, x, y, i == selectedIndex);
  }
}

void TFTKeyboard::drawHeader() {
  tft->fillRect(0, 0, screenWidth, headerHeight, TFT_BLACK);
  tft->setTextColor(textColor);
  tft->setTextDatum(MC_DATUM);
  
  // Adaptive font size for header
  int fontSize = (screenHeight <= 160) ? 1 : 2;
  tft->drawString(title, screenWidth / 2, headerHeight / 2, fontSize);
  tft->drawLine(0, headerHeight - 1, screenWidth, headerHeight - 1, TFT_DARKGREY);
}

void TFTKeyboard::drawTextDisplay() {
  int yStart = headerHeight;
  int yEnd = yStart + textDisplayHeight;
  
  tft->fillRect(0, yStart, screenWidth, textDisplayHeight, TFT_BLACK);
  
  // Draw text
  tft->setTextColor(textColor);
  tft->setTextDatum(MC_DATUM);
  String displayText = currentText.isEmpty() ? "_" : currentText;
  
  // Adaptive font size for text display
  int fontSize = (screenHeight <= 160) ? 1 : 2;
  
  // Truncate text if too long for display
  int maxChars = (screenWidth - 20) / (fontSize == 1 ? 6 : 12);
  if (displayText.length() > maxChars) {
    displayText = "..." + displayText.substring(displayText.length() - maxChars + 3);
  }
  
  tft->drawString(displayText, screenWidth / 2, yStart + textDisplayHeight / 2, fontSize);
  
  // Draw underline
  int underlineY = yEnd - (screenHeight <= 160 ? 4 : 8);
  int underlinePadding = (screenHeight <= 160 ? 10 : 20);
  tft->drawLine(underlinePadding, underlineY, screenWidth - underlinePadding, underlineY, TFT_DARKGREY);
}

void TFTKeyboard::calculateKeyPosition(int index, int &x, int &y) {
  int row = index / keysPerRow;
  int col = index % keysPerRow;
  
  int keyboardStartY = headerHeight + textDisplayHeight;
  
  x = keyPadding + col * (keyWidth + keyPadding);
  y = keyboardStartY + keyPadding + row * (keyHeight + keyPadding);
}

bool TFTKeyboard::isModeKey(int index) {
  if (index >= keys.size()) return false;
  Key* key = keys[index];
  return key->type == KEY_MODE_TOGGLE;
}

bool TFTKeyboard::isModeActive(int index) {
  if (index >= keys.size()) return false;
  Key* key = keys[index];
  if (key->type != KEY_MODE_TOGGLE) return false;
  
  // Check if this mode toggle is currently active
  if (key->toggleMode == MODE_SPECIAL && currentMode == MODE_SPECIAL) return true;
  if (key->toggleMode == MODE_UPPERCASE && currentMode == MODE_UPPERCASE) return true;
  
  return false;
}

void TFTKeyboard::drawKey(int index, int x, int y, bool isSelected) {
  Key* key = keys[index];
  
  // Determine label based on current mode
  String label = key->label;
  if (key->type == KEY_CHAR && currentMode == MODE_UPPERCASE) {
    label.toUpperCase();
  }
  
  // Special characters mode
  if (currentMode == MODE_SPECIAL && key->type == KEY_CHAR) {
    // Map alphabet keys to special characters in special mode
    const char* specialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?/~`";
    int charIndex = index;
    if (charIndex < 26 && charIndex < strlen(specialChars)) {
      label = String(specialChars[charIndex]);
    }
  }
  
  // Check if this is an active mode toggle
  bool isActiveMode = isModeActive(index);
  
  // Adaptive corner radius
  int cornerRadius = (screenHeight <= 160) ? 4 : 6;
  
  // Background color for active mode keys
  if (isActiveMode) {
    tft->fillRoundRect(x, y, keyWidth, keyHeight, cornerRadius, TFT_WHITE);
    tft->setTextColor(TFT_BLACK);
  } else {
    tft->fillRoundRect(x, y, keyWidth, keyHeight, cornerRadius, TFT_BLACK);
    tft->setTextColor(TFT_WHITE);
  }
  
  // Draw selection border
  if (isSelected) {
    tft->drawRoundRect(x, y, keyWidth, keyHeight, cornerRadius, selectedBorderColor);
    if (keyWidth > 20 && keyHeight > 20) {
      tft->drawRoundRect(x + 1, y + 1, keyWidth - 2, keyHeight - 2, cornerRadius - 1, selectedBorderColor);
    }
  }
  
  // Draw label - adaptive font size
  tft->setTextDatum(MC_DATUM);
  int textSize = 1;
  
  // Use larger font for bigger screens and simple labels
  if (screenHeight > 160 && label.length() <= 3 && keyHeight >= 28) {
    textSize = 2;
  }
  
  tft->drawString(label, x + keyWidth / 2, y + keyHeight / 2, textSize);
}

void TFTKeyboard::next() {
  selectedIndex++;
  if (selectedIndex >= keys.size()) {
    selectedIndex = 0;
  }
  draw();
}

void TFTKeyboard::previous() {
  selectedIndex--;
  if (selectedIndex < 0) {
    selectedIndex = keys.size() - 1;
  }
  draw();
}

void TFTKeyboard::select() {
  if (keys.empty() || selectedIndex >= keys.size()) return;
  
  Key* key = keys[selectedIndex];
  
  switch (key->type) {
    case KEY_CHAR:
      if (currentMode == MODE_SPECIAL) {
        // Add special character
        const char* specialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?/~`";
        int charIndex = selectedIndex;
        if (charIndex < 26 && charIndex < strlen(specialChars)) {
          addChar(String(specialChars[charIndex]));
        }
      } else if (currentMode == MODE_UPPERCASE) {
        String upper = key->value;
        upper.toUpperCase();
        addChar(upper);
      } else {
        addChar(key->value);
      }
      break;
      
    case KEY_MODE_TOGGLE:
      // Toggle between modes
      if (key->toggleMode == MODE_SPECIAL) {
        currentMode = (currentMode == MODE_SPECIAL) ? MODE_LOWERCASE : MODE_SPECIAL;
      } else if (key->toggleMode == MODE_UPPERCASE) {
        currentMode = (currentMode == MODE_UPPERCASE) ? MODE_LOWERCASE : MODE_UPPERCASE;
      }
      draw();
      break;
      
    case KEY_DELETE:
      deleteChar();
      break;
      
    case KEY_CLEAR:
      clearText();
      break;
      
    case KEY_CONFIRM:
      confirmText();
      break;
  }
}

void TFTKeyboard::addChar(String c) {
  currentText += c;
  draw();
}

void TFTKeyboard::deleteChar() {
  if (currentText.length() > 0) {
    currentText.remove(currentText.length() - 1);
    draw();
  }
}

void TFTKeyboard::clearText() {
  currentText = "";
  draw();
}

void TFTKeyboard::confirmText() {
  if (onConfirm) {
    onConfirm(currentText);
  }
  // Serial.print("Confirmed: ");
  // Serial.println(currentText);
}

void TFTKeyboard::handleButtons() {
  if (leftButtonPin < 0 || rightButtonPin < 0) return;
  
  static unsigned long leftPressTime = 0;
  static unsigned long rightPressTime = 0;
  static bool leftWasPressed = false;
  static bool rightWasPressed = false;
  static bool bothWerePressed = false;
  
  unsigned long currentTime = millis();
  
  // Read current button states (active LOW with pullup)
  bool leftPressed = (digitalRead(leftButtonPin) == LOW);
  bool rightPressed = (digitalRead(rightButtonPin) == LOW);
  bool bothPressed = leftPressed && rightPressed;
  
  // If both buttons are currently pressed
  if (bothPressed) {
    if (!bothWerePressed) {
      bothWerePressed = true;
      bothPressStartTime = currentTime;
      actionExecuted = false;
    }
    
    leftWasPressed = false;
    rightWasPressed = false;
    return;
  }
  
  // If both buttons were just released
  if (bothWerePressed && !bothPressed) {
    unsigned long pressDuration = currentTime - bothPressStartTime;
    
    if (pressDuration >= 150 && !actionExecuted) {
      select();
      actionExecuted = true;
    }
    
    bothWerePressed = false;
    leftWasPressed = false;
    rightWasPressed = false;
    leftPressTime = 0;
    rightPressTime = 0;
    lastDebounceTime = currentTime;
    
    delay(150);
    return;
  }
  
  // Check debounce time
  if (currentTime - lastDebounceTime < debounceDelay) {
    return;
  }
  
  // Handle LEFT button (PREVIOUS)
  if (leftPressed) {
    if (!leftWasPressed) {
      leftPressTime = currentTime;
      leftWasPressed = true;
    }
  } else {
    if (leftWasPressed) {
      unsigned long pressDuration = currentTime - leftPressTime;
      if (pressDuration >= 50 && pressDuration < 500) {
        previous();
        lastDebounceTime = currentTime;
      }
      leftWasPressed = false;
      leftPressTime = 0;
    }
  }
  
  // Handle RIGHT button (NEXT)
  if (rightPressed) {
    if (!rightWasPressed) {
      rightPressTime = currentTime;
      rightWasPressed = true;
    }
  } else {
    if (rightWasPressed) {
      unsigned long pressDuration = currentTime - rightPressTime;
      if (pressDuration >= 50 && pressDuration < 500) {
        next();
        lastDebounceTime = currentTime;
      }
      rightWasPressed = false;
      rightPressTime = 0;
    }
  }
}

void TFTKeyboard::update() {
  handleButtons();
}