#ifndef TFTKEYBOARD_H
#define TFTKEYBOARD_H

#include <Arduino.h>
#include "imports/TFT_eSPI/TFT_eSPI.h"
#include <vector>
#include <functional>

enum KeyboardMode {
  MODE_LOWERCASE,
  MODE_UPPERCASE,
  MODE_SPECIAL
};

enum KeyType {
  KEY_CHAR,
  KEY_MODE_TOGGLE,
  KEY_DELETE,
  KEY_CLEAR,
  KEY_CONFIRM
};

struct Key {
  String label;
  String value;
  KeyType type;
  KeyboardMode toggleMode;
  
  Key(String lbl, String val) : label(lbl), value(val), type(KEY_CHAR), toggleMode(MODE_LOWERCASE) {}
  Key(String lbl, KeyType t, KeyboardMode mode = MODE_LOWERCASE) : label(lbl), value(""), type(t), toggleMode(mode) {}
};

class TFTKeyboard {
private:
  TFT_eSPI* tft;
  String title;
  String currentText;
  int selectedIndex;
  KeyboardMode currentMode;
  
  std::vector<Key*> keys;
  
  // Display settings
  uint16_t bgColor;
  uint16_t textColor;
  uint16_t selectedBorderColor;
  uint16_t activeModeColor;
  
  int keyWidth;
  int keyHeight;
  int keyPadding;
  int keysPerRow;
  int headerHeight;
  int textDisplayHeight;
  int screenWidth;
  int screenHeight;
  
  // Button pins
  int leftButtonPin;
  int rightButtonPin;
  unsigned long lastDebounceTime;
  unsigned long debounceDelay;
  unsigned long bothPressStartTime;
  bool actionExecuted;
  
  // Callback
  std::function<void(String)> onConfirm;
  
  void initKeys();
  void drawHeader();
  void drawTextDisplay();
  void drawKey(int index, int x, int y, bool isSelected);
  void calculateKeyPosition(int index, int &x, int &y);
  bool isModeKey(int index);
  bool isModeActive(int index);
  
public:
  TFTKeyboard(TFT_eSPI* display, String keyboardTitle = "Keyboard");
  
  void setButtonPins(int leftPin, int rightPin);
  void setDebounceDelay(unsigned long delay);
  void setOnConfirm(std::function<void(String)> callback);
  
  void begin();
  void draw();
  void update();
  void handleButtons();
  
  void next();
  void previous();
  void select();
  
  void addChar(String c);
  void deleteChar();
  void resetButtonState();
  void clearText();
  void confirmText();
  
  String getText() { return currentText; }
  void setText(String text) { currentText = text; draw(); }
};

#endif