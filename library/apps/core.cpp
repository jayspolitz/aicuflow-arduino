#include "core.h"

// import all apps / pages here at once
// outcommented stuff is in main ino file
// #include "library/apps/core.cpp"           // page: core utils for page / menu setup
#include "library/apps/boot.cpp"              // page: boot screen & setup
#include "library/apps/measurement.cpp"       // page: measurement helpers
#include "library/apps/about.cpp"             // page: about (message)
#include "library/apps/random.cpp"            // page: random colors screen test
#include "library/apps/wifiscan.cpp"          // page: wifi scan
#if USE_BLUETOOTH
  #include "library/apps/btscan.cpp"            // page: bluethooth scan
#endif
#include "library/apps/colortest.cpp"         // page: gradient color test page
#include "library/apps/colorwheeltest.cpp"    // page: color test page
#include "library/apps/snakegame.cpp"         // page: snake game
#include "library/apps/gol.cpp"               // page: conways game of life
#include "library/apps/mandelbrot.cpp"        // page: mandelbrot set render
#include "library/apps/wireframe3d.cpp"       // page: 3d rotation renderer
#include "library/apps/aicuflow_tutorial.cpp" // page: tutorial
#include "library/apps/architecture.cpp"      // page: hw archi
#include "library/apps/create_flow.cpp"       // page: create flow
#include "library/apps/qrcode.cpp"            // page: qr code
#include "library/apps/settingsweb.cpp"       // page: web settings config
#include "library/apps/serialmonitor.cpp"     // page: serial monitor
#include "library/apps/voltmeter.cpp"         // page: voltage measurement
// add more pages here
// #include "library/apps/_expand.cpp"        // page: custom page


// UI / App Core of this Sketch

void setupMenus() {
  const char* s_back = en_de("Back", "Zurueck");
  // microcontroller tools
  mucToolsMenu = new TFTMenu(&tft, en_de("Hardware", "Hardware"));
  mucToolsMenu->addBackItem(s_back);
  mucToolsMenu->addItem(en_de("Voltmeter", "Voltmeter"), []() { pageManager->openPage("volt"); });
  mucToolsMenu->addItem(en_de("Serialmonitor", "UART Logger"), []() { pageManager->openPage("serial"); });
  mucToolsMenu->addItem(en_de("HW-Diagram", "HW-Diagramm"), []() { pageManager->openPage("architecture"); });

  // connection tools
  connectToolsMenu = new TFTMenu(&tft, en_de("Wireless", "Funk"));
  connectToolsMenu->addBackItem(s_back);
  connectToolsMenu->addItem(en_de("Wifi Scan", "Wlan Suche"), []() { pageManager->openPage("wifiscan"); });
  #if USE_BLUETOOTH
    connectToolsMenu->addItem(en_de("BT Scan", "BT Suche"), []() { pageManager->openPage("btscan"); });
  #endif

  // graphicstest
  graphicsMenu = new TFTMenu(&tft, en_de("Graphics", "Grafiken"));
  graphicsMenu->addBackItem(s_back);
  graphicsMenu->addItem(en_de("QR-Code", "QR-Code"), []() { pageManager->openPage("qr"); });
  graphicsMenu->addItem(en_de("Color Test", "Farbtest"), []() { pageManager->openPage("colors"); });
  graphicsMenu->addItem(en_de("Color Wheel", "Farbrad"), []() { pageManager->openPage("colorwheel"); });
  graphicsMenu->addItem(en_de("Random Color", "Farbrauschen"), []() { pageManager->openPage("random"); });

  // games
  gamesMenu = new TFTMenu(&tft, en_de("Games","Spiele"));
  gamesMenu->addBackItem(s_back);
  gamesMenu->addItem(en_de("Snake Game", "Snake Spiel"), []() { pageManager->openPage("snake"); });
  gamesMenu->addItem("Game of Life", []() { pageManager->openPage("gol"); });
  gamesMenu->addItem("Mandelbrot", []() { pageManager->openPage("mandelbrot"); });
  gamesMenu->addItem(en_de("3D Objects", "3D Objekte"), []() { pageManager->openPage("3d"); });

  // tools
  toolsMenu = new TFTMenu(&tft, "Tools");
  toolsMenu->addBackItem(s_back);
  toolsMenu->addSubmenu(en_de("Hardware", "Hardware"), mucToolsMenu);
  toolsMenu->addSubmenu(en_de("Wireless", "Funkmodule"), connectToolsMenu);
  toolsMenu->addSubmenu(en_de("Graphics", "Grafiken"), graphicsMenu);
  toolsMenu->addSubmenu(en_de("Games","Spiele"), gamesMenu);
  
  // wifi settings
  wifiMenu = new TFTMenu(&tft, en_de("WiFi Settings", "Wlan Zugang"));
  wifiMenu->addBackItem(s_back);
  wifiMenu->addItem(en_de("WiFi SSID", "Wlan Name"), []() { pageManager->openPage("keyboard", (void*)3); });
  wifiMenu->addItem(en_de("WiFi Passwd", "Wlan Passwd"), []() { pageManager->openPage("keyboard", (void*)4); });
  
  // aicuflow settings
  aicuAPIMenu = new TFTMenu(&tft, "Aicuflow API");
  aicuAPIMenu->addBackItem(s_back);
  aicuAPIMenu->addItem(en_de("User Mail", "Login Mail"), []() { pageManager->openPage("keyboard", (void*)5); });
  aicuAPIMenu->addItem(en_de("User Passwd.", "Login Passwd."), []() { pageManager->openPage("keyboard", (void*)6); });
  aicuAPIMenu->addItem(en_de("Create Flow", "Neuer Flow"), []() { pageManager->openPage("create", (void*)6); });
  aicuAPIMenu->addItem(en_de("Flow ID", "Flow Id"), []() { pageManager->openPage("keyboard", (void*)7); });
  aicuAPIMenu->addItem(en_de("Device Name", "Geraetename"), []() { pageManager->openPage("keyboard", (void*)1); }); // 1 = device name context
  aicuAPIMenu->addItem(en_de("File Name", "Dateiname"), []() { pageManager->openPage("keyboard", (void*)2); }); // 2 = streamfilename context
  
  // settings
  btnSetMenu = new TFTMenu(&tft, en_de("Manual Setup", "Manuell Setup"));
  btnSetMenu->addBackItem(s_back);
  btnSetMenu->addSubmenu(en_de("WiFi Settings", "Wlan Zugang"), wifiMenu);
  btnSetMenu->addSubmenu("Aicuflow API", aicuAPIMenu);
  btnSetMenu->addItem(en_de("Language: EN", "Language: DE"), []() { locale = locale == "en" ? "de" : "en"; saveSettings(); esp_restart(); });
  btnSetMenu->addItem(en_de("Restart Device", "Neu starten"), []() { esp_restart(); });

  // settings
  settingsMenu = new TFTMenu(&tft, en_de("Settings", "Einstellungen"));
  settingsMenu->addBackItem(s_back);
  settingsMenu->addItem(en_de("Easy Setup", "Leichtes Setup"),  []() { pageManager->openPage("webset"); });
  settingsMenu->addSubmenu(en_de("Manual Setup", "Manuell Setup"), btnSetMenu);
  settingsMenu->addItem(en_de("Tutorial", "Einfuehrung"), []() { pageManager->openPage("tutorial"); });
  settingsMenu->addItem(en_de("Factory Reset", "Zurucksetzen"), []() { clearSettings(); esp_restart(); });

  // main
  mainMenu = new TFTMenu(&tft, en_de("Aicuflow IoT+AI", "Aicuflow IoT+KI"));
  mainMenu->addItem(en_de("Measure", "Messung"), []() {
    if (wlanSSID == "" || aicuMail == "" || aicuPass == "") { // not configured
      pageManager->openPage("webset");
    } else { // configured
      pageManager->openPage("measure");
    }
  });
  mainMenu->addSubmenu(en_de("Tools", "Tools"), toolsMenu);
  mainMenu->addSubmenu(en_de("Setup", "Setup"), settingsMenu);
  mainMenu->addItem(en_de("About", "Infos"), []() { pageManager->openPage("about"); });

  // add more menus here
  // mainMenu->addItem("Custom Page", []() { pageManager->openPage("[appname]"); });
  
  // keyboard
  keyboard = new TFTKeyboard(&tft, en_de("Enter Text", "Text eingeben"));
  keyboard->setButtonPins(LEFT_BUTTON, RIGHT_BUTTON);
  keyboard->setOnConfirm(onTextConfirmed);
  
  // after all propagate
  mainMenu->propagateButtonPins(LEFT_BUTTON, RIGHT_BUTTON);
}
void setupPages() {
  pageManager = new PageManager(&tft, LEFT_BUTTON, RIGHT_BUTTON, SCREEN_IDLE_MS);
  
  pageManager->registerPage("menu", onMenuPageOpen, []() { mainMenu->update(); });
  pageManager->registerPage("keyboard", onKeyboardPageOpen, []() { keyboard->update(); });
  
  pageManager->registerPage("measure", measurementSetup, onMeasurePageUpdate, 0, false); // no delay!
  pageManager->registerPage("about", onAboutPageOpen, closePageIfAnyButtonIsPressed);
  pageManager->registerPage("tutorial", onAicuflowTutorialPageOpen, onAicuflowTutorialPageUpdate, 20, true, true); // delay, keep screen awake!
  pageManager->registerPage("create", onCreateFlowPageOpen, onCreateFlowPageUpdate);
  
  pageManager->registerPage("wifiscan", onWifiPageOpen, onWifiPageUpdate);
  #if USE_BLUETOOTH
    pageManager->registerPage("btscan", onBTPageOpen, onBTPageUpdate);
  #endif

  pageManager->registerPage("architecture", onHWDiagPageOpen, onHWDiagPageUpdate);
  pageManager->registerPage("colors", onColorTestPageOpen, onColorTestPageUpdate);
  pageManager->registerPage("colorwheel", onColorWheelTestPageOpen, onColorWheelTestPageUpdate);
  pageManager->registerPage("random", nullptr, []() { onRandomPageUpdate(); closePageIfAnyButtonIsPressed(); });
  pageManager->registerPage("snake", onSnakeGamePageOpen, onSnakeGamePageUpdate);
  pageManager->registerPage("gol", onGameOfLifePageOpen, onGameOfLifePageUpdate);
  pageManager->registerPage("mandelbrot", onMandelbrotPageOpen, onMandelbrotPageUpdate);
  pageManager->registerPage("3d", onWireframe3dPageOpen, onWireframe3dPageUpdate);
  pageManager->registerPage("qr", onQRCodePageOpen, onQRCodePageUpdate, 20, true, true); // no timeout
  pageManager->registerPage("webset", onSettingsWebPageOpen, onSettingsWebPageUpdate, 20, true, true); // no timeout
  pageManager->registerPage("serial", onSerialMonitorPageOpen, onSerialMonitorPageUpdate, 20, true, true); // no timeout
  pageManager->registerPage("volt", onVoltmeterPageOpen, onVoltmeterPageUpdate, 20, true, true); // no timeout
  
  // add more apps here, see `library/apps/_expand.cpp`
  // pageManager->registerPage("[appname]", onPageOpen, onPageUpdate);

  pageManager->setDefaultPage(
    device->has_display ? "menu" : "measure" // meause auto on no-screen devices
  );
  pageManager->begin();

  if (!hasDoneTutorial) {
    if (device->has_display) pageManager->openPage("tutorial"); // screen -> tutorial
    else { // no screen -> websetup and auto start
      Serial.println("Device is not yet configured.");
      Serial.println("Please connect to WiFi 'aicuflow'.");
      Serial.println("Then, open http://192.168.4.1/");
      Serial.println("After configuration, measurement will auto-start.");
      pageManager->openPage("webset");
    }
  }
}
void onMenuPageOpen() {
  tft.fillScreen(TFT_BLACK);
  TFTMenu* prevMenu = pageManager->getPreviousMenu();
  (prevMenu ? prevMenu : mainMenu)->begin();
}

// Keyboard
void onKeyboardPageOpen() {
  int context = pageManager->getContext<int>();
  if (context == 1) keyboard->setText(deviceName);          // 1 = CONTEXT_DEVICE_NAME
  else if (context == 2) keyboard->setText(streamFileName); // 2 = CONTEXT_FILE_NAME

  else if (context == 3) keyboard->setText(wlanSSID); // 3 = CONTEXT_WLAN_SSID
  else if (context == 4) keyboard->setText(wlanPass); // 4 = CONTEXT_WLAN_PASS
  else if (context == 5) keyboard->setText(aicuMail); // 5 = CONTEXT_AICU_USER
  else if (context == 6) keyboard->setText(aicuPass); // 6 = CONTEXT_AICU_PASS
  else if (context == 7) keyboard->setText(aicuFlow); // 7 = CONTEXT_AICU_FLOW
  
  tft.fillScreen(TFT_BLACK);
  keyboard->begin();
}

void onTextConfirmed(String text) {
  int context = pageManager->getContext<int>();
  if (context == 1) deviceName = text;          // 1 = CONTEXT_DEVICE_NAME
  else if (context == 2) streamFileName = text; // 2 = CONTEXT_FILE_NAME

  else if (context == 3) wlanSSID = text; // 3 = CONTEXT_WLAN_SSID
  else if (context == 4) wlanPass = text; // 4 = CONTEXT_WLAN_PASS
  else if (context == 5) aicuMail = text; // 5 = CONTEXT_AICU_USER
  else if (context == 6) aicuPass = text; // 6 = CONTEXT_AICU_PASS
  else if (context == 7) aicuFlow = text; // 7 = CONTEXT_AICU_FLOW

  saveSettings();
  applySettings();
  #if VERBOSE
    Serial.println("Saved: " + text);
  #endif
  pageManager->returnToPrevious();
  pageManager->blockInputFor(300);
}

void measurementSetup() {
  if (device->has_display)               plotScreen(1000);
  if (device->has_wifi)                  connectWifiOrTimeout();
  if (device->has_wifi && wifiAvailable) connectAPI();
  if (!sensors.getEnabledCount())        registerSensors();
  if (device->has_display)               initSensorGraphs();
  if (device->has_wifi)                  startCPUSendTask();
}