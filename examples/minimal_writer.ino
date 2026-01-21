/**
 *  Aicuflow AI Flow Builder + Arduino ESP32 Quickstart
 *
 *  Created: 20260121, by Finn Glas @ AICU GmbH
 *
 *  Minimal Writer Sketch - sends points
 *
 *  Check https://aicuflow.com/docs/library/arduino for more!
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "AicuClient.h"

const char* AICU_USER = "your-user";
const char* AICU_PASS = "your-pass";
const char* WIFI_SSID = "your-wifi";
const char* WIFI_PASS = "your-pass";
const char* PROJECT_FLOW = "your-ai-cu-flow-uuid";
const char* PROJECT_FILE = "stream-test.arrow";

WiFiClientSecure client;
AicuClient aicu("https://prod-backend.aicuflow.com", true);

/* --- SETUP - Connect, Send Data --- */
void setup() {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) delay(500);

    client.setInsecure();
    aicu.setWiFiClient(&client);
    if (!aicu.login(AICU_USER, AICU_PASS)) return;

    DynamicJsonDocument points(512);
    JsonObject point = points.to<JsonArray>().createNestedObject();
    point["ts"] = 1700000000;
    point["value1"] = 42.1;
    point["value2"] = 42.7;

    aicu.sendTimeseriesPoints(PROJECT_FLOW, PROJECT_FILE, points);
}

/* --- LOOP - Nothing --- */
void loop() {}
