/**
 * PageManager.h
 * 
 * A clean, reliable page management system for ESP32 TFT displays.
 * Handles page routing, rendering, input blocking, and loop timing.
 * 
 * Created: 2026-01-25
 * Part of Aicuflow IoT Library
 */

#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#include <Arduino.h>
#include <functional>
#include <map>

// Forward declarations
class TFT_eSPI;
class TFTMenu;

/**
 * PageManager - Main page management class
 */
class PageManager {
public:
  PageManager(TFT_eSPI* tft, int leftBtn, int rightBtn, int screenIdleMs = 60000);
  
  // Setup - chainable one-liners!
  PageManager& setDefaultPage(const char* pageId);
  PageManager& registerPage(
    const char* pageId,
    std::function<void()> onOpen,
    std::function<void()> onUpdate,
    uint16_t updateDelayMs = 20,
    bool blockInputOnOpen = true,
    bool keepScreenAwake = false
  );
  
  void begin();
  
  // Navigation - auto-remembers menus!
  void openPage(const char* pageId, void* context = nullptr);
  void returnToPrevious();
  void returnToDefault();
  
  // Main loop
  void update();
  
  // Input blocking
  void blockInputFor(uint16_t ms);
  bool isInputBlocked();
  
  // Context (for keyboard etc)
  template<typename T>
  T getContext() { return (T)(uintptr_t)currentContext; }
  
  // State
  const char* getCurrentPageId() const { return currentPageId; }
  bool isOnPage(const char* pageId) const;
  TFTMenu* getPreviousMenu() { return previousActiveMenu; }
  
  bool screenAwake;

private:
  struct PageConfig {
    std::function<void()> onOpen;
    std::function<void()> onUpdate;
    uint16_t updateDelayMs;
    bool blockInputOnOpen;
    bool keepScreenAwake;
  };
  
  TFT_eSPI* tft;
  int leftButton, rightButton;
  int screenIdleMs;
  uint32_t lastInputMs;
  
  std::map<String, PageConfig> pages;
  
  const char* defaultPageId;
  const char* currentPageId;
  const char* previousPageId;
  void* currentContext;
  
  TFTMenu* previousActiveMenu;
  
  bool inputBlocked;
  unsigned long inputBlockUntil;
  unsigned long lastUpdateMs;
  
  void transitionToPage(const char* pageId);
  void updateScreenPower();
};

#endif // PAGEMANAGER_H