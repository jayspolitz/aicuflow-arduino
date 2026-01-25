#include "_expand.h"

/**
 *  This is an example of how to create a custom
 *  Page / App within this Example Project.
 *
 *  Copy _expand.cpp and _expand.h, and rename them.
 *  Then link them from the main `aicuflow-arduino` file.

// in top imports
#include "library/apps/[appname].cpp"  // page: custom page

// in setupMenus:
mainMenu->addItem("Custom Page", []() { pageManager->openPage("[appname]"); });

// in setup
pageManager->registerPage("[appname]", onPageOpen, onPageUpdate)

 */

// page enter function (like arduino setup())
void onPageOpen() {
  // your inital page draw
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);

  tft.println("Example Page.");
  tft.println("Press any key");
}
// page update function (like arduino loop())
void onPageUpdate() {
  // your code here, ran within an interval of 20ms usually

  // end by allowing user to go back
  closePageIfAnyButtonIsPressed();
}

/* (Helpful) Color pallette for tft screens:
 * TFT_BLACK, TFT_NAVY, TFT_DARKGREEN, TFT_DARKCYAN, TFT_MAROON,
 * TFT_PURPLE, TFT_OLIVE, TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLUE,
 * TFT_GREEN, TFT_CYAN, TFT_RED, TFT_MAGENTA, TFT_YELLOW, TFT_WHITE,
 * TFT_ORANGE, TFT_GREENYELLOW, TFT_PINK */