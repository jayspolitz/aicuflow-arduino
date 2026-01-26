#include "Settings.h"

// settings
const char* PREFS = "aicu-settings";
Preferences preferences;

// props being saved; defaults here from main sketch, same order
String wlanSSID = WLAN_SSID;
String wlanPass = WLAN_PASS;
String aicuMail = AICU_USER;
String aicuPass = AICU_PASS;
String aicuFlow = PROJ_FLOW;

String locale = "de"; // en

String streamFileName = PROJ_FILE;
String deviceName = "device"; // auto-replaced by device id first time

// functions
void loadSettings() {
  preferences.begin(PREFS, false); // false = r/w mode

  wlanSSID = preferences.getString("wlanSSID", wlanSSID);
  wlanPass = preferences.getString("wlanPass", wlanPass);
  aicuMail = preferences.getString("aicuMail", aicuMail);
  aicuPass = preferences.getString("aicuPass", aicuPass);
  aicuFlow = preferences.getString("aicuFlow", aicuFlow);

  locale = preferences.getString("locale", locale);

  streamFileName = preferences.getString("streamFileName", streamFileName);
  deviceName = preferences.getString("deviceName", String(device->kind_short) + DEVICE_ID_SUFFIX);

  preferences.end();
  if (VERBOSE) {
    Serial.println("Loaded pref name: " + deviceName);
    Serial.println("Loaded pref fname: " + streamFileName);
    // tbd more
  }
}

void saveSettings() {
  preferences.begin(PREFS, false); // false = r/w mode

  preferences.putString("wlanSSID", wlanSSID);
  preferences.putString("wlanPass", wlanPass);
  preferences.putString("aicuMail", aicuMail);
  preferences.putString("aicuPass", aicuPass);
  preferences.putString("aicuFlow", aicuFlow);

  preferences.putString("locale", locale);

  preferences.putString("streamFileName", streamFileName);
  preferences.putString("deviceName", deviceName);

  preferences.end();
  if (VERBOSE) Serial.println("Prefs saved to mem!");
}

void clearSettings() {
  preferences.begin(PREFS, false);
  preferences.clear();
  preferences.end();
  if (VERBOSE) Serial.println("Prefs cleared!");
}

void applySettings() {
  sensors.deviceId = deviceName.c_str();
}

const char* en_de(const char* en, const char* de) {
    return (locale == "de") ? de : en;
}
