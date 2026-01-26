/**
 *  Aicuflow ESP32 TFTMenu Demo
 *
 *  Created: 20260124, by Finn Glas @ AICU GmbH
 *  
 *  This example shows how to use TFTMenu:
 *  that way you can traverse nested menus,
 *  and mount custom pages as actions too!
 *  
 *  Check https://aicuflow.com/docs/library/arduino for more!
 */

#include "imports/TFT_eSPI/TFT_eSPI.h"
#include "library/graphics/TFTMenu.cpp" // or TFTMenuModern for a cooler style

TFT_eSPI tft = TFT_eSPI();

/* AVAILABLE TFT COLORS ===
 * TFT_BLACK, TFT_NAVY, TFT_DARKGREEN, TFT_DARKCYAN, TFT_MAROON,
 * TFT_PURPLE, TFT_OLIVE, TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLUE,
 * TFT_GREEN, TFT_CYAN, TFT_RED, TFT_MAGENTA, TFT_YELLOW, TFT_WHITE,
 * TFT_ORANGE, TFT_GREENYELLOW, TFT_PINK === */

#define LEFT_BUTTON  0
#define RIGHT_BUTTON 35

TFTMenu *mainMenu;
TFTMenu *subMenu1;
TFTMenu *subMenu2;

// custom subpages / apps
enum Page {
  PAGE_MENU,
  PAGE_CUSTOM
};
Page currentPage = PAGE_MENU;

void drawCustomPage() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(20, 40);
  tft.print("Custom Page");
  tft.setCursor(20, 60);
  tft.print("Press any key");
}
void updateCustomPage() {
  if (digitalRead(LEFT_BUTTON) == LOW ||
      digitalRead(RIGHT_BUTTON) == LOW) {

    currentPage = PAGE_MENU;
    tft.fillScreen(TFT_BLACK);
    mainMenu->draw();   // or begin() if your lib requires it
    delay(200);         // crude debounce
  }
}
void openCustomPage() {
  currentPage = PAGE_CUSTOM;
  drawCustomPage();
}

// dummy actions
void action1() { Serial.println("action 1"); }
void action2() { Serial.println("action 2"); }
void subAction1() { Serial.println("subaction 1"); }
void subAction2() { Serial.println("subaction 2"); }

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  mainMenu = new TFTMenu(&tft, "Main Menu");

  subMenu1 = new TFTMenu(&tft, "Sub Menu 1");
  subMenu1->addBackItem();
  subMenu1->setColors(TFT_BLACK, TFT_CYAN, TFT_WHITE, TFT_BLACK);
  // setColors(background, selectedBg, text, selectedText);

  subMenu2 = new TFTMenu(&tft, "Sub Menu 2");
  subMenu2->addBackItem();
  subMenu2->addItem("Subaction 1", subAction1);
  subMenu2->addItem("Subaction 2", subAction2);
  subMenu2->setColors(TFT_BLACK, TFT_DARKGREEN, TFT_WHITE, TFT_WHITE);

  subMenu1->addSubmenu("Sub Menu 2", subMenu2);
  subMenu1->addItem("Action 2", action2);

  mainMenu->addSubmenu("Sub Menu 1", subMenu1);
  mainMenu->addItem("Action 1", action1);
  mainMenu->addItem("Custom Page", openCustomPage);

  // after all propagate
  mainMenu->propagateButtonPins(LEFT_BUTTON, RIGHT_BUTTON);
  mainMenu->begin();
}

void loop() {
  if (currentPage == PAGE_MENU) {
    mainMenu->update();
  } else {
    updateCustomPage();
  }
  delay(20);
}
