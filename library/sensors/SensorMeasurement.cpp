/**
 * SensorMeasurement.cpp
 * 
 * Implementation of dynamic sensor measurement system
 * Lightweight and flexible - all sensors registered from main sketch
 */

#include "SensorMeasurement.h"
// or #include "sensors/SensorMeasurement.h"
#include "../graphics/ScrollingGraph.cpp"
// or #include "graphics/ScrollingGraph.cpp"
#include "imports/TFT_eSPI/TFT_eSPI.h"

SensorMeasurement::SensorMeasurement(const char* deviceId) 
    : deviceId(deviceId), timestamp(0) {
}

SensorMeasurement::~SensorMeasurement() {
    for (auto* graph : graphs) {
        delete graph;
    }
}

void SensorMeasurement::registerSensor(const char* key, const char* label,
                                      std::function<double()> readFunc,
                                      double min, double max, uint16_t color,
                                      bool enabled, bool showGraph) {
    SensorDef def = {
        .key = key,
        .label = label,
        .read = readFunc,
        .min = min,
        .max = max,
        .color = color,
        .enabled = enabled,
        .showGraph = showGraph
    };
    sensors.push_back(def);
    values.push_back(0.0);
}

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
    for (size_t i = 0; i < sensors.size(); i++) {
        if (strcmp(sensors[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

void SensorMeasurement::measure() {
    timestamp = millis();
    for (size_t i = 0; i < sensors.size(); i++) {
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
    obj["millis"] = timestamp;
    obj["device"] = deviceId;
    
    for (size_t i = 0; i < sensors.size(); i++) {
        if (sensors[i].enabled) {
            obj[sensors[i].key] = values[i];
        }
    }
}

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
        graph->begin(0, yOffset, screenWidth, graphBarHeight,
                    sensors[i].min, sensors[i].max,
                    sensors[i].color, sensors[i].label);
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

void SensorMeasurement::printValues() const {
    Serial.print("Timestamp: "); Serial.println(timestamp);
    for (size_t i = 0; i < sensors.size(); i++) {
        if (sensors[i].enabled) {
            Serial.print(sensors[i].label); 
            Serial.print(": "); 
            Serial.println(values[i]);
        }
    }
    Serial.println("========");
}

int SensorMeasurement::getEnabledCount() const {
    int count = 0;
    for (const auto& sensor : sensors) {
        if (sensor.enabled) count++;
    }
    return count;
}

int SensorMeasurement::getGraphCount() const {
    int count = 0;
    for (const auto& sensor : sensors) {
        if (sensor.enabled && sensor.showGraph) count++;
    }
    return count;
}