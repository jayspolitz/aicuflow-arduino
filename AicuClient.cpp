#include "AicuClient.h"

// ---- constructor ----
AicuClient::AicuClient(const String& baseUrl, bool verbose)
: _baseUrl(baseUrl), _verbose(verbose) {}

// ---- set TLS client ----
void AicuClient::setWiFiClient(WiFiClientSecure* client) {
    _client = client;
}

// ---- set token manually ----
void AicuClient::setToken(const String& token) {
    _authHeader = "Bearer " + token;
}

// ---- login ----
bool AicuClient::login(const String& email, const String& password) {
    DynamicJsonDocument payload(512);
    payload["email"] = email;
    payload["password"] = password;
    payload["keep_me_logged_in"] = true;

    DynamicJsonDocument response(1024);
    if (!_postJson("/user/auth/login/", payload, response)) return false;

    const char* token = response["data"]["accesstoken"];
    if (!token || strlen(token) == 0) {
        if (_verbose) Serial.println("Token missing in login response!");
        return false;
    }

    setToken(String(token));
    if (_verbose) Serial.println("Login successful!");
    return true;
}

// ---- GET helper ----
bool AicuClient::_getJson(const String& path, JsonDocument& doc) {
    if (!_client) return false;

    HTTPClient http;
    http.begin(*_client, _baseUrl + path);
    http.setTimeout(10000); // 10 seconds
    if (_authHeader.length()) http.addHeader("Authorization", _authHeader);

    if (_verbose) Serial.print("GET "); Serial.println(_baseUrl + path);

    int code = http.GET();
    String body = http.getString();
    http.end();

    if (_verbose) {
        Serial.print("HTTP Code: "); Serial.println(code);
        Serial.println("Body:"); Serial.println(body);
    }

    if (code < 200 || code >= 300) return false;

    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        if (_verbose) Serial.println("JSON parse failed!");
        return false;
    }
    return true;
}

// ---- POST helper ----
bool AicuClient::_postJson(const String& path, JsonDocument& payload, JsonDocument& response) {
    if (!_client) return false;

    String payloadStr;
    serializeJson(payload, payloadStr);

    HTTPClient http;
    http.begin(*_client, _baseUrl + path);
    http.setTimeout(10000); // 10 seconds
    http.addHeader("Content-Type", "application/json");
    if (_authHeader.length()) http.addHeader("Authorization", _authHeader);

    if (_verbose) {
        Serial.print("POST "); Serial.println(_baseUrl + path);
        Serial.print("Payload: "); Serial.println(payloadStr);
    }

    int code = http.POST(payloadStr);
    String body = http.getString();
    http.end();

    if (_verbose) {
        Serial.print("HTTP Code: "); Serial.println(code);
        Serial.println("Body:"); Serial.println(body);
    }

    if (code < 200 || code >= 300) return false;

    DeserializationError err = deserializeJson(response, body);
    if (err) {
        if (_verbose) Serial.println("JSON parse failed!");
        return false;
    }

    return true;
}

// ---- POST without response ----
bool AicuClient::_postJson(const String& path, JsonDocument& payload) {
    DynamicJsonDocument dummy(1);
    return _postJson(path, payload, dummy);
}

// ---- DELETE helper ----
bool AicuClient::_del(const String& path) {
    if (!_client) return false;

    HTTPClient http;
    http.begin(*_client, _baseUrl + path);
    if (_authHeader.length()) http.addHeader("Authorization", _authHeader);

    if (_verbose) Serial.print("DELETE "); Serial.println(_baseUrl + path);

    int code = http.sendRequest("DELETE");
    http.end();

    if (_verbose) Serial.print("HTTP DELETE Code: "); Serial.println(code);

    return code >= 200 && code < 300;
}

// ---- User endpoints ----
bool AicuClient::getSettings(JsonDocument& doc) { return _getJson("/user/profile/", doc); }
bool AicuClient::getSubscription(JsonDocument& doc) { return _getJson("/subscriptions/me/", doc); }

// ---- Flow endpoints ----
bool AicuClient::listFlows(JsonDocument& doc) { return _getJson("/flows/?page=1&page_size=100", doc); }
bool AicuClient::getFlow(const String& flowId, JsonDocument& doc) { return _getJson("/flows/" + flowId + "/", doc); }
bool AicuClient::deleteFlow(const String& flowId) { return _del("/flows/" + flowId + "/"); }

bool AicuClient::createFlow(const String& title, const String& description, JsonDocument& doc) {
    DynamicJsonDocument payload(512);
    payload["title"] = title;
    payload["description"] = description;
    payload["is_global"] = false;
    payload["is_template"] = false;

    return _postJson("/flows/", payload, doc);
}

// ---- Timeseries ----
bool AicuClient::sendTimeseriesPoints(const String& flowId, const String& filename, JsonDocument& points) {
    if (!points.is<JsonArray>()) {
        if (_verbose) Serial.println("Points must be a JSON array!");
        return false;
    }

    JsonArray arr = points.as<JsonArray>();

    for (JsonObject obj : arr) {
        for (JsonPair kv : obj) {
            if (kv.value().is<int>() || kv.value().is<long>() || kv.value().is<unsigned long>()) {
                int64_t val = kv.value().as<int64_t>();
                kv.value().set(val);  // <-- Korrekt
            } else if (kv.value().is<float>() || kv.value().is<double>()) {
                double val = kv.value().as<double>();
                kv.value().set(val);   // <-- Korrekt
            }
        }
    }


    // POST
    String path = "/data/write-values/?flow=" + flowId + "&filename=" + filename;
    DynamicJsonDocument resp(1024);
    bool ok = _postJson(path, points, resp);

    if (_verbose) {
        Serial.println("Timeseries POST response:");
        serializeJsonPretty(resp, Serial);
    }
    return ok;
}
