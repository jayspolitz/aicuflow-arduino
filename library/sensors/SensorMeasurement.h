/**
 * SensorMeasurement.h
 * 
 * Dynamic sensor registry and measurement system for Aicuflow
 * Clean, minimal, flexible - register sensors individually as needed
 */

#ifndef SENSOR_MEASUREMENT_H
#define SENSOR_MEASUREMENT_H

#include <Arduino.h>
#include "imports/ArduinoJson/ArduinoJson.h"
#include <functional>
#include <vector>

#define SHOW_GRAPH true
#define HIDE_GRAPH false
#define LOG_SEND true
#define LOG_NONE false

// Forward declarations
class ScrollingGraph;
class TFT_eSPI;

/**
 * Sensor Definition - describes a single sensor
 */
struct SensorDef {
    const char* key;                   // JSON key & internal identifier
    std::function<double()> read;      // Function to read sensor value
    double min;                        // Min value for graphing
    double max;                        // Max value for graphing
    uint16_t color;                    // Graph color (TFT color constant)
    bool enabled;                      // Whether to measure this sensor
    bool showGraph;                    // Whether to display graph for this sensor
};

/**
 * SensorMeasurement - manages all sensors and their readings
 */
class SensorMeasurement {
public:
    const char* deviceId;

    SensorMeasurement(const char* deviceId="dev");
    ~SensorMeasurement();

    // Core sensor registration - use this for everything!
    void registerSensor(const char* key, 
                       std::function<double()> readFunc,
                       uint16_t color, bool enabled = true, bool showGraph = true);

    // Control
    void enableSensor(const char* key, bool enabled = true);
    void disableSensor(const char* key) { enableSensor(key, false); }
    void showSensorGraph(const char* key, bool show = true);
    void hideSensorGraph(const char* key) { showSensorGraph(key, false); }

    // Measurement
    void measure();  // Read all enabled sensors
    double getValue(const char* key) const;
    int64_t getTimestamp() const { return timestamp; }

    // JSON serialization
    void toJson(JsonObject& obj) const;

    // Graph management
    void initGraphs(TFT_eSPI* tft, int screenWidth, int screenHeight);
    void updateGraphs();
    void setGraphSpacing(int barHeight, int barGap);

    // Debug
    void printValues() const;

    // Access to sensor definitions
    const std::vector<SensorDef>& getSensors() const { return sensors; }
    int getEnabledCount() const;
    int getGraphCount() const;

private:
    std::vector<SensorDef> sensors;
    std::vector<double> values;
    std::vector<ScrollingGraph*> graphs;
    int64_t timestamp;
    
    // Graph layout settings
    int graphBarHeight = 14;
    int graphBarGap = 3;
    
    int findSensorIndex(const char* key) const;
};

#endif // SENSOR_MEASUREMENT_H