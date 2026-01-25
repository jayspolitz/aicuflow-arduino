#include "Settings.h"

// settings
const char* PREFS = "aicu-settings";
Preferences preferences;

// props being saved
String deviceName = "device"; // auto-replaced by device id first time
String streamFileName = PROJ_FILE;

// functions
void loadSettings() {
  preferences.begin(PREFS, false); // false = r/w mode
  deviceName = preferences.getString("deviceName", String(device->kind_short) + DEVICE_ID_SUFFIX);
  streamFileName = preferences.getString("streamFileName", streamFileName);
  preferences.end();
  if (VERBOSE) {
    Serial.println("Loaded pref name: " + deviceName);
    Serial.println("Loaded pref fname: " + streamFileName);
  }
}

void saveSettings() {
  preferences.begin(PREFS, false); // false = r/w mode
  preferences.putString("deviceName", deviceName);
  preferences.putString("streamFileName", streamFileName);
  preferences.end();
  if (VERBOSE) Serial.println("Prefs saved to mem!");
}

void clearSettings() {
  preferences.begin(PREFS, false);
  preferences.clear();
  preferences.end();
  if (VERBOSE) Serial.println("Prefs cleared!");
}