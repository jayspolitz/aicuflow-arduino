#include "Settings.h"

// settings
const char* PREFS = "aicu-settings";
Preferences preferences;

// props being saved
String deviceName = device.kind_short;
const char* PREF_DNAME = "deviceName";

// functions
void loadSettings() {
  preferences.begin(PREFS, false); // false = r/w mode
  deviceName = preferences.getString(PREF_DNAME, deviceName);
  preferences.end();
  if (VERBOSE) {
    Serial.println("Loaded pref name: " + deviceName);
  }
}

void saveSettings() {
  preferences.begin(PREFS, false); // false = r/w mode
  preferences.putString(PREF_DNAME, deviceName);
  preferences.end();
  if (VERBOSE) Serial.println("Prefs saved to mem!");
}

void clearSettings() {
  preferences.begin(PREFS, false);
  preferences.clear();
  preferences.end();
  if (VERBOSE) Serial.println("Prefs cleared!");
}