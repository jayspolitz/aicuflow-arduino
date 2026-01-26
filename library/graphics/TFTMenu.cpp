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
  previousSelectedIndex = 0;
  previousScrollOffset = 0;
  
  // Default colors (dark theme)
  bgColor = TFT_BLACK;
  selectedBgColor = TFT_BLUE;
  textColor = TFT_WHITE;
  selectedTextColor = TFT_WHITE;
  headerColor = TFT_DARKGREY;
  
  // Display settings
  showControls = true;
  itemHeight = 40;
  headerHeight = 30;
  controlsHeight = 25;
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
  previousSelectedIndex = selectedIndex;
  previousScrollOffset = scrollOffset;
}

void TFTMenu::setActive() {
  currentActiveMenu = this;
}

bool TFTMenu::isActive() {
  return currentActiveMenu == this;
}

void TFTMenu::draw() {
  drawHeader();
  
  // Draw visible items
  int startIdx = scrollOffset;
  int endIdx = min(scrollOffset + visibleItems, (int)items.size());
  
  for (int i = startIdx; i < endIdx; i++) {
    int yPos = headerHeight + (i - scrollOffset) * itemHeight;
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
}

void TFTMenu::drawMenuItem(int index, int yPos, bool isSelected) {
  MenuItem* item = items[index];
  
  // Background
  if (isSelected) {
    tft->fillRect(0, yPos, screenWidth, itemHeight, selectedBgColor);
    tft->setTextColor(selectedTextColor);
  } else {
    tft->fillRect(0, yPos, screenWidth, itemHeight, bgColor);
    tft->setTextColor(textColor);
  }
  
  // Draw separator line
  tft->drawLine(0, yPos + itemHeight - 1, screenWidth, yPos + itemHeight - 1, TFT_DARKGREY);
  
  // Text
  tft->setTextDatum(ML_DATUM);
  tft->drawString(item->label, 10, yPos + itemHeight / 2, 2);
  
  // Draw chevron for submenu or back items
  if (item->type == MENU_ITEM_SUBMENU) {
    drawChevron(screenWidth - 20, yPos + itemHeight / 2, true, isSelected ? selectedTextColor : textColor);
  } else if (item->type == MENU_ITEM_BACK) {
    drawChevron(screenWidth - 20, yPos + itemHeight / 2, false, isSelected ? selectedTextColor : textColor);
  }
}

void TFTMenu::drawChevron(int x, int y, bool pointsRight, uint16_t color) {
  int size = 6;
  if (pointsRight) {
    tft->fillTriangle(x - size, y - size, x - size, y + size, x + size, y, color);
  } else {
    tft->fillTriangle(x + size, y - size, x + size, y + size, x - size, y, color);
  }
}

void TFTMenu::drawControls() {
  int yPos = screenHeight - controlsHeight;
  tft->fillRect(0, yPos, screenWidth, controlsHeight, headerColor);
  
  tft->setTextColor(textColor);
  tft->setTextDatum(MC_DATUM);
  
  // Left: UP
  tft->drawString("UP", screenWidth / 6, yPos + controlsHeight / 2, 2);
  
  // Center: OK
  tft->drawString("OK", screenWidth / 2, yPos + controlsHeight / 2, 2);
  
  // Right: DOWN
  tft->drawString("DOWN", screenWidth * 5 / 6, yPos + controlsHeight / 2, 2);
}

void TFTMenu::ensureSelectedVisible() {
  if (selectedIndex < scrollOffset) {
    scrollOffset = selectedIndex;
  } else if (selectedIndex >= scrollOffset + visibleItems) {
    scrollOffset = selectedIndex - visibleItems + 1;
  }
}

void TFTMenu::updateSelection() {
  // Check if scroll position changed - if so, redraw all items
  if (scrollOffset != previousScrollOffset) {
    // Scroll changed - redraw all visible items
    int startIdx = scrollOffset;
    int endIdx = min(scrollOffset + visibleItems, (int)items.size());
    
    for (int i = startIdx; i < endIdx; i++) {
      int yPos = headerHeight + (i - scrollOffset) * itemHeight;
      drawMenuItem(i, yPos, i == selectedIndex);
    }
    
    previousScrollOffset = scrollOffset;
    previousSelectedIndex = selectedIndex;
    return;
  }
  
  // No scroll change - just update the two affected items
  if (selectedIndex != previousSelectedIndex) {
    // Redraw previously selected item (now unselected)
    if (previousSelectedIndex >= scrollOffset && previousSelectedIndex < scrollOffset + visibleItems) {
      int yPos = headerHeight + (previousSelectedIndex - scrollOffset) * itemHeight;
      drawMenuItem(previousSelectedIndex, yPos, false);
    }
    
    // Redraw newly selected item
    if (selectedIndex >= scrollOffset && selectedIndex < scrollOffset + visibleItems) {
      int yPos = headerHeight + (selectedIndex - scrollOffset) * itemHeight;
      drawMenuItem(selectedIndex, yPos, true);
    }
    
    previousSelectedIndex = selectedIndex;
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
  updateSelection();
}

void TFTMenu::previous() {
  selectedIndex--;
  if (selectedIndex < 0) {
    selectedIndex = items.size() - 1;
    scrollOffset = max(0, (int)items.size() - visibleItems);
  } else {
    ensureSelectedVisible();
  }
  updateSelection();
}

void TFTMenu::select() {
  if (items.empty()) return;
  
  MenuItem* item = items[selectedIndex];
  
  if (item->type == MENU_ITEM_ACTION && item->action) {
    item->action();
  } else if (item->type == MENU_ITEM_SUBMENU && item->submenu) {
    item->submenu->begin();
  } else if (item->type == MENU_ITEM_BACK && parentMenu) {
    parentMenu->begin();
  }
}

void TFTMenu::handleButtons() {
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