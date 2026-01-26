#pragma once
#ifndef AICU_CLIENT_H
#define AICU_CLIENT_H
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "imports/ArduinoJson/ArduinoJson.h"

class AicuClient {
public:
    AicuClient(const String& baseUrl = "https://prod-backend.aicuflow.com", bool verbose = false);

    void setWiFiClient(WiFiClientSecure* client);
    void setToken(const String& token);

    bool login(const String& email, const String& password);

    // user
    bool getSettings(JsonDocument& doc);
    bool getSubscription(JsonDocument& doc);

    // flows
    bool listFlows(JsonDocument& doc);
    bool getFlow(const String& flowId, JsonDocument& doc);
    bool deleteFlow(const String& flowId);
    bool createFlow(const String& title, const String& description, JsonDocument& doc);

    // timeseries
    bool sendTimeseriesPoints(const String& flowId, const String& filename, JsonDocument& points);

private:
    String _baseUrl;
    String _authHeader;
    bool _verbose;
    WiFiClientSecure* _client = nullptr;

    bool _getJson(const String& path, JsonDocument& doc);
    bool _postJson(const String& path, JsonDocument& payload, JsonDocument& response);
    bool _postJson(const String& path, JsonDocument& payload); // ohne Response
    bool _del(const String& path);
};
#endif