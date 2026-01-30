/**
 * SensorMeasurement.h
 * 
 * Dynamic sensor registry and measurement system for Aicuflow
 * Clean, minimal, flexible - register sensors individually as needed
 * 
 * Platform support: ESP32, ESP8266, Arduino AVR
 */

#ifndef SENSOR_MEASUREMENT_H
#define SENSOR_MEASUREMENT_H

#include <Arduino.h>

// Platform detection - assumes IS_ESP and IS_AVR are defined in build
#include "../../imports/ArduinoJson/ArduinoJson.h"
#if IS_ESP
    #include <functional>
    #include <vector>
    #include "../graphics/ScrollingGraph.cpp"
    #include "imports/TFT_eSPI/TFT_eSPI.h"
#endif

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
#if IS_ESP
    std::function<double()> read;      // Function to read sensor value (ESP)
#else
    double (*read)();                  // Function pointer for AVR
#endif
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
    
    SensorMeasurement(const char* deviceId = "dev");
    ~SensorMeasurement();

    // Core sensor registration - use this for everything!
#if IS_ESP
    void registerSensor(const char* key, 
                       std::function<double()> readFunc,
                       uint16_t color, bool enabled = true, bool showGraph = true);
#else
    void registerSensor(const char* key, 
                       double (*readFunc)(),
                       uint16_t color, bool enabled = true, bool showGraph = true);
#endif

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

#if IS_ESP
    // Graph management (ESP only)
    void initGraphs(TFT_eSPI* tft, int screenWidth, int screenHeight);
    void updateGraphs();
    void setGraphSpacing(int barHeight, int barGap);
#endif

    // Debug
    void printValues() const;

    // Access to sensor definitions
#if IS_ESP
    const std::vector<SensorDef>& getSensors() const { return sensors; }
#else
    int getSensorCount() const { return sensorCount; }
    const SensorDef* getSensors() const { return sensors; }
#endif
    int getEnabledCount() const;
    int getGraphCount() const;

private:
#if IS_ESP
    std::vector<SensorDef> sensors;
    std::vector<double> values;
    std::vector<ScrollingGraph*> graphs;
    
    // Graph layout settings
    int graphBarHeight = 14;
    int graphBarGap = 3;
#else
    // Static arrays for AVR
    static const int MAX_SENSORS = 32;
    SensorDef sensors[MAX_SENSORS];
    double values[MAX_SENSORS];
    int sensorCount;
#endif

    int64_t timestamp;
    
    int findSensorIndex(const char* key) const;
};

#endif // SENSOR_MEASUREMENT_H