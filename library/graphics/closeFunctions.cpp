#include "library/graphics/PageManager.h"
#include "library/graphics/TFTMenu.h"

extern int LEFT_BUTTON;
extern int RIGHT_BUTTON;
extern PageManager* pageManager;
extern TFTMenu* mainMenu;

void closePage() {
  pageManager->returnToPrevious();
}
void toStartPage() {
  pageManager->openPage("menu");
  mainMenu->setActive();
  mainMenu->selectedIndex = 0;
  mainMenu->begin();
}
void closePageIfAnyButtonIsPressed() {
  if (digitalRead(LEFT_BUTTON) == LOW || digitalRead(RIGHT_BUTTON) == LOW) {
    pageManager->returnToPrevious();
  }
}
void closePageIfBothLongPressed() {
  static uint8_t state = 0;
  static unsigned long firstPressTime = 0;
  static unsigned long bothPressTime = 0;
  static unsigned long firstReleaseTime = 0;
  
  bool left  = (digitalRead(LEFT_BUTTON) == LOW);
  bool right = (digitalRead(RIGHT_BUTTON) == LOW);
  unsigned long now = millis();
  
  switch (state) {
    // 0: Idle - waiting for first button
    case 0:
      if (left || right) {
        firstPressTime = now;
        state = 1;
      }
      break;
      
    // 1: One button down - wait for other within 400ms
    case 1:
      if (left && right) {
        // Both now pressed
        bothPressTime = now;
        state = 2;
      } else if (!left && !right) {
        // Released before second joined
        state = 0;
      } else if (now - firstPressTime > 400) {
        // Timeout
        state = 0;
      }
      break;
      
    // 2: Both pressed - hold for 3000ms
    case 2:
      if (!left || !right) {
        // Released too early
        state = 0;
      } else if (now - bothPressTime >= 3000) {
        // Held long enough
        state = 3;
      }
      break;
      
    // 3: Waiting for first release
    case 3:
      if (!left || !right) {
        firstReleaseTime = now;
        state = 4;
      }
      break;
      
    // 4: One released - other must release within 400ms
    case 4:
      if (!left && !right) {
        // Both released - SUCCESS
        pageManager->returnToPrevious();
        state = 0;
      } else if (now - firstReleaseTime > 400) {
        // Timeout - not released together
        state = 0;
      }
      break;
  }
}