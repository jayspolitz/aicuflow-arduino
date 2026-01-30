/**
 * SensorMeasurement.cpp
 * 
 * Implementation of dynamic sensor measurement system
 * Lightweight and flexible - all sensors registered from main sketch
 * Works on both ESP32 and Arduino AVR platforms
 */

#include "SensorMeasurement.h"

#if IS_ESP
    #include "../display/ScrollingGraph.h"
#endif

SensorMeasurement::SensorMeasurement(const char* deviceId) 
    : deviceId(deviceId), timestamp(0) {
#if IS_AVR
    sensorCount = 0;
#endif
}

SensorMeasurement::~SensorMeasurement() {
#if IS_ESP
    for (auto* graph : graphs) {
        delete graph;
    }
#endif
}

#if IS_ESP
void SensorMeasurement::registerSensor(const char* key,
                                      std::function<double()> readFunc,
                                      uint16_t color, bool enabled, bool showGraph) {
    SensorDef def = {
        .key = key,
        .read = readFunc,
        .min = 0.0,           // Default range
        .max = 100.0,         // Can be set later if needed
        .color = color,
        .enabled = enabled,
        .showGraph = showGraph
    };
    sensors.push_back(def);
    values.push_back(0.0);
}
#else
void SensorMeasurement::registerSensor(const char* key,
                                      double (*readFunc)(),
                                      uint16_t color, bool enabled, bool showGraph) {
    if (sensorCount >= MAX_SENSORS) {
        Serial.println(F("ERROR: Max sensors reached!"));
        return;
    }
    
    sensors[sensorCount].key = key;
    sensors[sensorCount].read = readFunc;
    sensors[sensorCount].min = 0.0;
    sensors[sensorCount].max = 100.0;
    sensors[sensorCount].color = color;
    sensors[sensorCount].enabled = enabled;
    sensors[sensorCount].showGraph = showGraph;
    values[sensorCount] = 0.0;
    sensorCount++;
}
#endif

void SensorMeasurement::enableSensor(const char* key, bool enabled) {
    int idx = findSensorIndex(key);
    if (idx >= 0) {
        sensors[idx].enabled = enabled;
    }
}

void SensorMeasurement::showSensorGraph(const char* key, bool show) {
    int idx = findSensorIndex(key);
    if (idx >= 0) {
        sensors[idx].showGraph = show;
    }
}

int SensorMeasurement::findSensorIndex(const char* key) const {
#if IS_ESP
    for (size_t i = 0; i < sensors.size(); i++) {
#else
    for (int i = 0; i < sensorCount; i++) {
#endif
        if (strcmp(sensors[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

void SensorMeasurement::measure() {
    timestamp = millis();
    
#if IS_ESP
    for (size_t i = 0; i < sensors.size(); i++) {
#else
    for (int i = 0; i < sensorCount; i++) {
#endif
        if (sensors[i].enabled && sensors[i].read) {
            values[i] = sensors[i].read();
        }
    }
}

double SensorMeasurement::getValue(const char* key) const {
    int idx = findSensorIndex(key);
    return (idx >= 0) ? values[idx] : 0.0;
}

void SensorMeasurement::toJson(JsonObject& obj) const {
    obj["id"] = deviceId;
    obj["ms"] = timestamp;
    
#if IS_ESP
    for (size_t i = 0; i < sensors.size(); i++) {
#else
    for (int i = 0; i < sensorCount; i++) {
#endif
        if (sensors[i].enabled) {
            obj[sensors[i].key] = serialized(String(values[i], 2));
        }
    }
}

#if IS_ESP
void SensorMeasurement::setGraphSpacing(int barHeight, int barGap) {
    graphBarHeight = barHeight;
    graphBarGap = barGap;
}

void SensorMeasurement::initGraphs(TFT_eSPI* tft, int screenWidth, int screenHeight) {
    // Clean up existing graphs
    for (auto* graph : graphs) {
        delete graph;
    }
    graphs.clear();
    
    // Count how many graphs we need
    int graphCount = getGraphCount();
    if (graphCount == 0) return;
    
    int fullHeight = graphBarHeight + graphBarGap;
    int yOffset = screenHeight;
    
    // Create graphs in the order they were registered (bottom to top on screen)
    for (int i = sensors.size() - 1; i >= 0; i--) {
        if (!sensors[i].enabled || !sensors[i].showGraph) continue;
        
        yOffset -= fullHeight;
        ScrollingGraph* graph = new ScrollingGraph(tft);
        graph->begin(0, yOffset, screenWidth, graphBarHeight, sensors[i].color);
        graphs.insert(graphs.begin(), graph);
    }
}

void SensorMeasurement::updateGraphs() {
    int graphIdx = 0;
    for (size_t i = 0; i < sensors.size(); i++) {
        if (!sensors[i].enabled || !sensors[i].showGraph) continue;
        if (graphIdx < graphs.size()) {
            graphs[graphIdx]->update(values[i]);
            graphIdx++;
        }
    }
}
#endif

void SensorMeasurement::printValues() const {
    Serial.print(F("Timestamp: ")); 
#if IS_AVR
    Serial.println((unsigned long)timestamp);
#else
    Serial.println(timestamp);
#endif
    
#if IS_ESP
    for (size_t i = 0; i < sensors.size(); i++) {
#else
    for (int i = 0; i < sensorCount; i++) {
#endif
        if (sensors[i].enabled) {
            Serial.print(sensors[i].key); 
            Serial.print(F(": ")); 
            Serial.println(values[i]);
        }
    }
    Serial.println(F("========"));
}

int SensorMeasurement::getEnabledCount() const {
    int count = 0;
#if IS_ESP
    for (const auto& sensor : sensors) {
#else
    for (int i = 0; i < sensorCount; i++) {
        const SensorDef& sensor = sensors[i];
#endif
        if (sensor.enabled) count++;
    }
    return count;
}

int SensorMeasurement::getGraphCount() const {
    int count = 0;
#if IS_ESP
    for (const auto& sensor : sensors) {
#else
    for (int i = 0; i < sensorCount; i++) {
        const SensorDef& sensor = sensors[i];
#endif
        if (sensor.enabled && sensor.showGraph) count++;
    }
    return count;
}