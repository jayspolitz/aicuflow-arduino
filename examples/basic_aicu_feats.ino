/**
 *  Aicuflow AI Flow Builder + Arduino ESP32 Quickstart
 *
 *  Created: 20260121, by Finn Glas @ AICU GmbH
 *
 *  This sketch showcases how you can use the aicuflow library
 *  on arduino.
 *
 *  Check https://aicuflow.com/docs/library/arduino for more!
 */

#define VERBOSE true

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "imports/ArduinoJson/ArduinoJson.h"
#include "aicuflow/AicuClient.cpp"

const char* AICU_USER = "your-user";
const char* AICU_PASS = "your-pass";

const char* WIFI_SSID = "your-wifi";
const char* WIFI_PASS = "your-pass";

const char* PROJ_FLOW = "your-ai-cu-flow-uuid";
const char* PROJ_FILE = "stream-test";

WiFiClientSecure client;
AicuClient aicu("https://prod-backend.aicuflow.com");



/* --- SETUP - Wifi, Aicu, Feature tests --- */
void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("Starting...");

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    client.setInsecure();      // TLS workaround
    aicu.setWiFiClient(&client);

    // --- Login ---
    if (!aicu.login(AICU_USER, AICU_PASS)) {
        Serial.println("Login failed!");
        return;
    }

    // --- List Flows ---
    DynamicJsonDocument flows(4096);
    if (aicu.listFlows(flows)) {
        Serial.println("Flows:");
        serializeJsonPretty(flows, Serial);
    }

    // --- Create Flow ---
    DynamicJsonDocument newFlow(1024);
    if (aicu.createFlow("Arduino Flow", "ArduinoJson created flow", newFlow)) {
        Serial.println("New Flow Created:");
        serializeJsonPretty(newFlow, Serial);
    }

    // --- Send Timeseries ---
    DynamicJsonDocument points(512);
    JsonArray arr = points.to<JsonArray>();
    JsonObject p1 = arr.createNestedObject();
    p1["ts"] = 1700000000;
    p1["value"] = 42.1;
    JsonObject p2 = arr.createNestedObject();
    p2["ts"] = 1700000060;
    p2["value"] = 43.0;

    if (aicu.sendTimeseriesPoints(PROJ_FLOW, PROJ_FILE, points)) {
        Serial.println("Timeseries sent successfully!");
    } else {
        Serial.println("Failed to send timeseries!");
    }
}



/* --- LOOP - Nothing --- */
void loop() {
  sleep(100);
}
