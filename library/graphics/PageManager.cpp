/**
 * PageManager.cpp
 * 
 * Implementation of the PageManager system.
 */

#include "PageManager.h"
#include "../graphics/TFTMenuModern.cpp" // TFTMenuModern or TFTMenu
#include "imports/TFT_eSPI/TFT_eSPI.h"

PageManager::PageManager(TFT_eSPI* tft, int leftBtn, int rightBtn, int screenIdleMs)
  : tft(tft)
  , leftButton(leftBtn)
  , rightButton(rightBtn)
  , screenIdleMs(screenIdleMs)
  , lastInputMs(millis())
  , screenAwake(true)
  , defaultPageId(nullptr)
  , currentPageId(nullptr)
  , previousPageId(nullptr)
  , currentContext(nullptr)
  , previousActiveMenu(nullptr)
  , inputBlocked(false)
  , inputBlockUntil(0)
  , lastUpdateMs(0)
{
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
}

PageManager& PageManager::setDefaultPage(const char* pageId) {
  defaultPageId = pageId;
  return *this;
}

PageManager& PageManager::registerPage(
  const char* pageId,
  std::function<void()> onOpen,
  std::function<void()> onUpdate,
  uint16_t updateDelayMs,
  bool blockInputOnOpen,
  bool keepScreenAwake
) {
  PageConfig config;
  config.onOpen = onOpen;
  config.onUpdate = onUpdate;
  config.updateDelayMs = updateDelayMs;
  config.blockInputOnOpen = blockInputOnOpen;
  config.keepScreenAwake = keepScreenAwake;
  pages[String(pageId)] = config;
  return *this;
}

void PageManager::begin() {
  if (defaultPageId == nullptr) {
    Serial.println("ERROR: No default page set!");
    return;
  }
  
  currentPageId = defaultPageId;
  PageConfig& config = pages[String(currentPageId)];
  
  if (config.onOpen) config.onOpen();
  lastUpdateMs = millis();
}

void PageManager::transitionToPage(const char* pageId) {
  if (currentPageId == pageId) return;
  
  // Auto-remember active menu
  if (TFTMenu::currentActiveMenu != nullptr) {
    previousActiveMenu = TFTMenu::currentActiveMenu;
  }
  
  previousPageId = currentPageId;
  currentPageId = pageId;
  
  PageConfig& newConfig = pages[String(currentPageId)];
  
  if (newConfig.blockInputOnOpen) {
    blockInputFor(300);
  }
  
  if (newConfig.onOpen) newConfig.onOpen();
  lastUpdateMs = millis();
}

void PageManager::openPage(const char* pageId, void* context) {
  auto it = pages.find(String(pageId));
  if (it == pages.end()) {
    Serial.print("ERROR: Page not found: ");
    Serial.println(pageId);
    return;
  }
  
  currentContext = context;
  transitionToPage(pageId);
}

void PageManager::returnToPrevious() {
  if (previousPageId != nullptr) {
    openPage(previousPageId);
  } else {
    returnToDefault();
  }
}

void PageManager::returnToDefault() {
  if (defaultPageId != nullptr) {
    openPage(defaultPageId);
  }
}

void PageManager::updateScreenPower() {
  if (!screenIdleMs) return;
  if (currentPageId == nullptr) return;
  
  PageConfig& config = pages[String(currentPageId)];
    if (config.keepScreenAwake) {
      lastInputMs = millis();
    if (!screenAwake) {
      digitalWrite(TFT_BL, HIGH);
      screenAwake = true;
    }
    return;
  }
  bool anyInput = (digitalRead(leftButton) == LOW) || (digitalRead(rightButton) == LOW);
  
  if (anyInput) {
    lastInputMs = millis();
    if (!screenAwake) {
      digitalWrite(TFT_BL, HIGH);
      screenAwake = true;
    }
  } else if (screenAwake && (millis() - lastInputMs > screenIdleMs)) {
    digitalWrite(TFT_BL, LOW);
    screenAwake = false;
  }
}

void PageManager::update() {
  // Screen power management
  updateScreenPower();
  
  // Check input blocking
  if (inputBlocked && millis() >= inputBlockUntil) {
    inputBlocked = false;
  }
  
  if (inputBlocked) {
    delay(20);
    return;
  }
  
  if (currentPageId == nullptr) {
    Serial.println("ERROR: No current page!");
    return;
  }
  
  PageConfig& config = pages[String(currentPageId)];
  
  // Handle update timing
  if (config.updateDelayMs > 0) {
    unsigned long now = millis();
    if (now - lastUpdateMs < config.updateDelayMs) {
      delay(config.updateDelayMs - (now - lastUpdateMs));
    }
    lastUpdateMs = millis();
  }
  
  // Call update handler
  if (config.onUpdate) config.onUpdate();
}

void PageManager::blockInputFor(uint16_t ms) {
  inputBlocked = true;
  inputBlockUntil = millis() + ms;
}

bool PageManager::isInputBlocked() {
  if (inputBlocked && millis() >= inputBlockUntil) {
    inputBlocked = false;
  }
  return inputBlocked;
}

bool PageManager::isOnPage(const char* pageId) const {
  return currentPageId != nullptr && strcmp(currentPageId, pageId) == 0;
}