#include "TFTMenu.h"

// Static member initialization
TFTMenu* TFTMenu::currentActiveMenu = nullptr;

TFTMenu::TFTMenu(TFT_eSPI* display, String menuTitle) {
  tft = display;
  title = menuTitle;
  selectedIndex = 0;
  parentMenu = nullptr;
  activeMenu = nullptr;
  scrollOffset = 0;
  
  // Default colors (minimal black & white theme)
  bgColor = TFT_BLACK;
  selectedBgColor = TFT_BLACK;  // No background fill
  textColor = TFT_WHITE;
  selectedTextColor = TFT_WHITE;
  headerColor = TFT_BLACK;  // Clean black header
  
  // Display settings - tighter, more minimal
  showControls = true;
  itemHeight = 32;  // Smaller, tighter
  headerHeight = 35;
  controlsHeight = 20;  // Minimal bottom bar
  screenWidth = tft->width();
  screenHeight = tft->height();
  
  // Calculate visible items
  int availableHeight = screenHeight - headerHeight;
  if (showControls) availableHeight -= controlsHeight;
  visibleItems = availableHeight / itemHeight;
  
  // Button settings
  leftButtonPin = -1;
  rightButtonPin = -1;
  lastDebounceTime = 0;
  debounceDelay = 200; // Longer debounce for better feel
  bothPressStartTime = 0;
  singlePressStartTime = 0;
  buttonHoldThreshold = 100; // Shorter - more responsive
  lastLeftState = HIGH;
  lastRightState = HIGH;
  lastBothState = HIGH;
  actionExecuted = false;
}

void TFTMenu::setColors(uint16_t bg, uint16_t selectedBg, uint16_t text, uint16_t selectedText) {
  bgColor = bg;
  selectedBgColor = selectedBg;
  textColor = text;
  selectedTextColor = selectedText;
}

void TFTMenu::setHeaderColor(uint16_t color) {
  headerColor = color;
}

void TFTMenu::setShowControls(bool show) {
  showControls = show;
  // Recalculate visible items
  int availableHeight = screenHeight - headerHeight;
  if (showControls) availableHeight -= controlsHeight;
  visibleItems = availableHeight / itemHeight;
}

void TFTMenu::setButtonPins(int leftPin, int rightPin) {
  leftButtonPin = leftPin;
  rightButtonPin = rightPin;
  
  if (leftButtonPin >= 0) {
    pinMode(leftButtonPin, INPUT_PULLUP);
  }
  if (rightButtonPin >= 0) {
    pinMode(rightButtonPin, INPUT_PULLUP);
  }
}

void TFTMenu::propagateButtonPins(int leftPin, int rightPin) {
  setButtonPins(leftPin, rightPin);
  // Propagate to all submenus
  for (auto item : items) {
    if (item->type == MENU_ITEM_SUBMENU && item->submenu) {
      item->submenu->propagateButtonPins(leftPin, rightPin);
    }
  }
}

void TFTMenu::setDebounceDelay(unsigned long delay) {
  debounceDelay = delay;
}

void TFTMenu::addItem(String label, std::function<void()> action) {
  items.push_back(new MenuItem(label, action));
}

void TFTMenu::addSubmenu(String label, TFTMenu* submenu) {
  submenu->setParent(this);
  items.push_back(new MenuItem(label, submenu));
}

void TFTMenu::addBackItem(String label) {
  items.insert(items.begin(), new MenuItem(label, MENU_ITEM_BACK));
}

void TFTMenu::begin() {
  setActive();  // Mark this menu as the active one
  tft->fillScreen(bgColor);
  draw();
}

void TFTMenu::setActive() {
  currentActiveMenu = this;
}

bool TFTMenu::isActive() {
  return currentActiveMenu == this;
}

void TFTMenu::draw() {
  tft->fillScreen(bgColor);
  tft->setTextSize(1);
  drawHeader();
  
  // Calculate if we should center items vertically
  int availableHeight = screenHeight - headerHeight;
  if (showControls) availableHeight -= controlsHeight;
  
  int totalItemsHeight = items.size() * itemHeight;
  int startY = headerHeight;
  
  // Center items vertically if they don't fill the screen
  if (totalItemsHeight < availableHeight) {
    startY = headerHeight + (availableHeight - totalItemsHeight) / 2;
  }
  
  // Draw visible items
  int startIdx = scrollOffset;
  int endIdx = min(scrollOffset + visibleItems, (int)items.size());
  
  for (int i = startIdx; i < endIdx; i++) {
    int yPos = startY + (i - scrollOffset) * itemHeight;
    drawMenuItem(i, yPos, i == selectedIndex);
  }
  
  if (showControls) {
    drawControls();
  }
}

void TFTMenu::drawHeader() {
  tft->fillRect(0, 0, screenWidth, headerHeight, headerColor);
  tft->setTextColor(textColor);
  tft->setTextDatum(MC_DATUM);
  tft->drawString(title, screenWidth / 2, headerHeight / 2, 2);
  
  // Minimal separator line
  tft->drawLine(0, headerHeight - 1, screenWidth, headerHeight - 1, TFT_DARKGREY);
}

void TFTMenu::drawMenuItem(int index, int yPos, bool isSelected) {
  MenuItem* item = items[index];
  
  // Clean background - no fill
  tft->fillRect(0, yPos, screenWidth, itemHeight, bgColor);
  tft->setTextColor(textColor);
  
  // Draw rounded rectangle border around selected item (Apple-style)
  if (isSelected) {
    int padding = 12;  // Padding from edges
    int cornerRadius = 8;  // Rounded corners
    
    // Draw rounded rect outline
    tft->drawRoundRect(padding, yPos + 4, screenWidth - (padding * 2), itemHeight - 8, cornerRadius, TFT_WHITE);
  }
  
  // Text - centered with left padding
  tft->setTextDatum(ML_DATUM);
  int textPadding = isSelected ? 24 : 20;
  tft->drawString(item->label, textPadding, yPos + itemHeight / 2, 2);
  
  // Draw minimal chevron for submenu or back items
  if (item->type == MENU_ITEM_SUBMENU) {
    drawChevron(screenWidth - 24, yPos + itemHeight / 2, true, textColor);
  } else if (item->type == MENU_ITEM_BACK) {
    drawChevron(screenWidth - 24, yPos + itemHeight / 2, false, textColor);
  }
}

void TFTMenu::drawChevron(int x, int y, bool pointsRight, uint16_t color) {
  int size = 4;  // Smaller, more delicate
  
  // Draw outline chevron (not filled - modern look)
  if (pointsRight) {
    // Right-pointing chevron >
    tft->drawLine(x - size, y - size, x + size, y, color);
    tft->drawLine(x + size, y, x - size, y + size, color);
  } else {
    // Left-pointing chevron <
    tft->drawLine(x + size, y - size, x - size, y, color);
    tft->drawLine(x - size, y, x + size, y + size, color);
  }
}

void TFTMenu::drawControls() {
  int yPos = screenHeight - controlsHeight;
  
  // Pure black background - minimal
  tft->fillRect(0, yPos, screenWidth, controlsHeight, TFT_BLACK);
  
  // Thin separator line at top
  tft->drawLine(0, yPos, screenWidth, yPos, TFT_DARKGREY);
  
  // Minimal white text - small font
  tft->setTextColor(TFT_WHITE);
  tft->setTextDatum(MC_DATUM);
  
  // Left: UP
  tft->drawString("UP", screenWidth / 6, yPos + controlsHeight / 2, 1);
  
  // Center: OK
  tft->drawString("OK", screenWidth / 2, yPos + controlsHeight / 2, 1);
  
  // Right: DN
  tft->drawString("DN", screenWidth * 5 / 6, yPos + controlsHeight / 2, 1);
}

void TFTMenu::ensureSelectedVisible() {
  if (selectedIndex < scrollOffset) {
    scrollOffset = selectedIndex;
  } else if (selectedIndex >= scrollOffset + visibleItems) {
    scrollOffset = selectedIndex - visibleItems + 1;
  }
}

void TFTMenu::next() {
  selectedIndex++;
  if (selectedIndex >= items.size()) {
    selectedIndex = 0;
    scrollOffset = 0;
  } else {
    ensureSelectedVisible();
  }
  draw();
}

void TFTMenu::previous() {
  selectedIndex--;
  if (selectedIndex < 0) {
    selectedIndex = items.size() - 1;
    scrollOffset = max(0, (int)items.size() - visibleItems);
  } else {
    ensureSelectedVisible();
  }
  draw();
}

void TFTMenu::select() {
  if (items.empty()) return;
  
  MenuItem* item = items[selectedIndex];
  
  if (item->type == MENU_ITEM_ACTION && item->action) {
    item->action();
  } else if (item->type == MENU_ITEM_SUBMENU && item->submenu) {
    item->submenu->begin();  // This will set the submenu as active
  } else if (item->type == MENU_ITEM_BACK && parentMenu) {
    parentMenu->begin();  // This will set the parent as active
  }
}

void TFTMenu::handleButtons() {
  // Don't check isActive here - update() already routes to the right menu
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
      // Just started pressing both
      bothWerePressed = true;
      bothPressStartTime = currentTime;
      actionExecuted = false;
    }
    
    // Don't do anything else while both are pressed
    leftWasPressed = false;
    rightWasPressed = false;
    return;
  }
  
  // If both buttons were just released
  if (bothWerePressed && !bothPressed) {
    unsigned long pressDuration = currentTime - bothPressStartTime;
    
    // Only trigger if both were held for at least 150ms
    if (pressDuration >= 150 && !actionExecuted) {
      select();
      actionExecuted = true;
    }
    
    // Reset everything
    bothWerePressed = false;
    leftWasPressed = false;
    rightWasPressed = false;
    leftPressTime = 0;
    rightPressTime = 0;
    lastDebounceTime = currentTime;
    
    // Wait a bit before accepting new input
    delay(150);
    return;
  }
  
  // Check debounce time
  if (currentTime - lastDebounceTime < debounceDelay) {
    return;
  }
  
  // Handle LEFT button (UP)
  if (leftPressed) {
    if (!leftWasPressed) {
      leftPressTime = currentTime;
      leftWasPressed = true;
    }
  } else {
    // Left button released
    if (leftWasPressed) {
      unsigned long pressDuration = currentTime - leftPressTime;
      // Only navigate if button was pressed alone (not as part of both-button press)
      if (pressDuration >= 50 && pressDuration < 500) {
        previous();
        lastDebounceTime = currentTime;
      }
      leftWasPressed = false;
      leftPressTime = 0;
    }
  }
  
  // Handle RIGHT button (DOWN)
  if (rightPressed) {
    if (!rightWasPressed) {
      rightPressTime = currentTime;
      rightWasPressed = true;
    }
  } else {
    // Right button released
    if (rightWasPressed) {
      unsigned long pressDuration = currentTime - rightPressTime;
      // Only navigate if button was pressed alone (not as part of both-button press)
      if (pressDuration >= 50 && pressDuration < 500) {
        next();
        lastDebounceTime = currentTime;
      }
      rightWasPressed = false;
      rightPressTime = 0;
    }
  }
}

void TFTMenu::update() {
  // Always call update on the currently active menu, not just this one
  if (currentActiveMenu != nullptr) {
    currentActiveMenu->handleButtons();
  }
}