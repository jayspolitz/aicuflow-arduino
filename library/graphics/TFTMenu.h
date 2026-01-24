#ifndef TFT_MENU_H
#define TFT_MENU_H

#include <TFT_eSPI.h>
#include <functional>

// Menu item types
enum MenuItemType {
  MENU_ITEM_ACTION,
  MENU_ITEM_SUBMENU,
  MENU_ITEM_BACK
};

// Forward declarations
class TFTMenu;
class MenuItem;

// MenuItem class
class MenuItem {
public:
  String label;
  MenuItemType type;
  std::function<void()> action;
  TFTMenu* submenu;
  
  MenuItem(String lbl, std::function<void()> act) 
    : label(lbl), type(MENU_ITEM_ACTION), action(act), submenu(nullptr) {}
  
  MenuItem(String lbl, TFTMenu* sub) 
    : label(lbl), type(MENU_ITEM_SUBMENU), action(nullptr), submenu(sub) {}
  
  MenuItem(String lbl, MenuItemType t) 
    : label(lbl), type(t), action(nullptr), submenu(nullptr) {}
};

// Main menu class
class TFTMenu {
private:
  TFT_eSPI* tft;
  std::vector<MenuItem*> items;
  int selectedIndex;
  TFTMenu* parentMenu;
  TFTMenu* activeMenu;  // Track which menu is actually active
  
  // Colors
  uint16_t bgColor;
  uint16_t selectedBgColor;
  uint16_t textColor;
  uint16_t selectedTextColor;
  uint16_t headerColor;
  
  // Display settings
  bool showControls;
  String title;
  int itemHeight;
  int headerHeight;
  int controlsHeight;
  int screenWidth;
  int screenHeight;
  int visibleItems;
  int scrollOffset;
  
  // Button state
  unsigned long lastDebounceTime;
  unsigned long debounceDelay;
  unsigned long bothPressStartTime;
  unsigned long singlePressStartTime;
  unsigned long buttonHoldThreshold;
  bool lastLeftState;
  bool lastRightState;
  bool lastBothState;
  bool actionExecuted;
  
  // Pins
  int leftButtonPin;
  int rightButtonPin;
  
public:
  TFTMenu(TFT_eSPI* display, String menuTitle = "Menu");
  
  // Configuration
  void setColors(uint16_t bg, uint16_t selectedBg, uint16_t text, uint16_t selectedText);
  void setHeaderColor(uint16_t color);
  void setShowControls(bool show);
  void setButtonPins(int leftPin, int rightPin);
  void propagateButtonPins(int leftPin, int rightPin);
  void setDebounceDelay(unsigned long delay);
  
  // Menu building
  void addItem(String label, std::function<void()> action);
  void addSubmenu(String label, TFTMenu* submenu);
  void addBackItem(String label = "← Back");
  
  // Display
  void begin();
  void draw();
  void update();
  
  // Navigation
  void next();
  void previous();
  void select();
  
  // Button handling
  void handleButtons();
  
  // Getters
  int getSelectedIndex() { return selectedIndex; }
  TFTMenu* getParent() { return parentMenu; }
  void setParent(TFTMenu* parent) { parentMenu = parent; }
  
  // Active menu management
  void setActive();
  bool isActive();
  static TFTMenu* currentActiveMenu;
  
private:
  void drawHeader();
  void drawMenuItem(int index, int yPos, bool isSelected);
  void drawControls();
  void drawChevron(int x, int y, bool pointsRight, uint16_t color);
  void ensureSelectedVisible();
};

#endif