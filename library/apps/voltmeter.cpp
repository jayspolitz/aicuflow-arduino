#include "voltmeter.h"

int lastVoltmeterReadTime = 0;
int voltmeterReadInterval = 100; // read every 100ms
const int VOLT_PIN_EXTRA = 1; // GPIO1 (ADC1_CH0) on ESP32-S3
const int NUM_SAMPLES = 20; // Average multiple readings for stability
float voltage = 0.0;
float current = 0.0;
float resistance = 0.0;
float power = 0.0;
float minVoltage = 999.9;
float maxVoltage = 0.0;

// ESP32-S3 ADC characteristics
const float ADC_VREF = 3.3; // Reference voltage
const int ADC_RESOLUTION = 4095; // 12-bit ADC (0-4095)

// Current sensing configuration (if using a shunt resistor)
const float SHUNT_RESISTOR = 0.1; // 0.1 Ohm shunt resistor (100mΩ)
const bool USE_SHUNT = false; // Set to true if you have a shunt resistor setup

// Voltage divider if using one
const float R1 = 0.0; // Resistor to higher voltage (in kOhms)
const float R2 = 1.0; // Resistor to ground (in kOhms)

// Known reference resistor for resistance measurement mode
const float REFERENCE_VOLTAGE = 3.3; // Supply voltage
const float SERIES_RESISTOR = 10.0; // 10kΩ series resistor for resistance measurement

enum MeasurementMode {
  MODE_VOLTAGE,
  MODE_CURRENT,
  MODE_RESISTANCE
};

MeasurementMode currentMode = MODE_VOLTAGE;

void drawVoltmeterHeader() {
  tft.fillRect(0, 0, screenWidth, 24, TFT_BLUE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(2);
  tft.setCursor(4, 4);
  
  switch(currentMode) {
    case MODE_VOLTAGE:
      tft.print(en_de("Voltmeter", "Voltmeter"));
      break;
    case MODE_CURRENT:
      tft.print(en_de("Ammeter", "Amperemeter"));
      break;
    case MODE_RESISTANCE:
      tft.print(en_de("Ohmmeter", "Ohmmeter"));
      break;
  }
}

void onVoltmeterPageOpen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  
  // Configure ADC for ESP32-S3
  pinMode(VOLT_PIN_EXTRA, INPUT);
  analogReadResolution(12); // 12-bit resolution
  analogSetAttenuation(ADC_11db); // 0-3.3V range
  analogSetPinAttenuation(VOLT_PIN_EXTRA, ADC_11db);
  
  // Reset min/max
  minVoltage = 999.9;
  maxVoltage = 0.0;
  currentMode = MODE_VOLTAGE;
  
  // Draw header
  drawVoltmeterHeader();
}

float readVoltageWithAveraging() {
  long sum = 0;
  int validSamples = 0;
  
  for (int i = 0; i < NUM_SAMPLES; i++) {
    int reading = analogRead(VOLT_PIN_EXTRA);
    if (reading >= 0 && reading <= ADC_RESOLUTION) {
      sum += reading;
      validSamples++;
    }
    delayMicroseconds(100);
  }
  
  if (validSamples == 0) return 0.0;
  
  float avgRaw = sum / (float)validSamples;
  
  // Convert ADC reading to voltage
  float measuredVoltage = (avgRaw / ADC_RESOLUTION) * ADC_VREF;
  
  // Apply voltage divider calculation if using one
  if (R1 > 0) {
    measuredVoltage = measuredVoltage * ((R1 + R2) / R2);
  }
  
  return measuredVoltage;
}

void calculateElectricalValues() {
  voltage = readVoltageWithAveraging();
  
  switch(currentMode) {
    case MODE_VOLTAGE:
      // Just measure voltage
      break;
      
    case MODE_CURRENT:
      // Calculate current using shunt resistor
      // I = V / R_shunt
      if (USE_SHUNT && SHUNT_RESISTOR > 0) {
        current = voltage / SHUNT_RESISTOR; // in Amperes
      } else {
        current = 0.0;
      }
      break;
      
    case MODE_RESISTANCE:
      // Calculate resistance using voltage divider method
      // R_unknown = R_series * (V_measured / (V_ref - V_measured))
      if (voltage > 0.01 && voltage < (REFERENCE_VOLTAGE - 0.01)) {
        resistance = SERIES_RESISTOR * (voltage / (REFERENCE_VOLTAGE - voltage));
      } else if (voltage <= 0.01) {
        resistance = 999999.9; // Open circuit
      } else {
        resistance = 0.0; // Short circuit
      }
      break;
  }
  
  // Calculate power (P = V * I)
  if (currentMode == MODE_CURRENT && current > 0) {
    power = voltage * current;
  } else {
    power = 0.0;
  }
}

void drawVoltmeterBar(float value, float maxValue, uint16_t color) {
  int barX = 10;
  int barY = screenHeight - 50;
  int barWidth = screenWidth - 20;
  int barHeight = 20;
  
  // Draw bar background
  tft.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
  tft.fillRect(barX + 2, barY + 2, barWidth - 4, barHeight - 4, TFT_BLACK);
  
  // Calculate fill
  int fillWidth = (value / maxValue) * (barWidth - 4);
  fillWidth = constrain(fillWidth, 0, barWidth - 4);
  
  // Draw filled portion
  tft.fillRect(barX + 2, barY + 2, fillWidth, barHeight - 4, color);
}

void displayVoltageMode() {
  // Main voltage reading
  tft.setTextSize(3);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(10, 35);
  tft.printf("%.3f V", voltage);
  
  // Raw ADC value
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  int rawValue = analogRead(VOLT_PIN_EXTRA);
  tft.setCursor(10, 65);
  tft.printf(en_de("Raw: %d/4095", "Roh: %d/4095"), rawValue);
  
  // Min/Max
  if (voltage < minVoltage) minVoltage = voltage;
  if (voltage > maxVoltage) maxVoltage = voltage;
  
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(10, 80);
  tft.printf(en_de("Min: %.3fV", "Min: %.3fV"), minVoltage);
  tft.setCursor(10, 92);
  tft.printf(en_de("Max: %.3fV", "Max: %.3fV"), maxVoltage);
  
  // Draw bar
  drawVoltmeterBar(voltage, 3.3, TFT_GREEN);
}

void displayCurrentMode() {
  // Voltage reading
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(10, 35);
  tft.printf("V: %.3f V", voltage);
  
  // Current reading
  tft.setTextSize(3);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(10, 55);
  if (current >= 1.0) {
    tft.printf("%.2f A", current);
  } else if (current >= 0.001) {
    tft.printf("%.1f mA", current * 1000);
  } else {
    tft.printf("%.0f uA", current * 1000000);
  }
  
  // Power
  tft.setTextSize(2);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setCursor(10, 85);
  if (power >= 1.0) {
    tft.printf(en_de("P: %.2f W", "P: %.2f W"), power);
  } else {
    tft.printf(en_de("P: %.0f mW", "P: %.0f mW"), power * 1000);
  }
  
  if (!USE_SHUNT) {
    tft.setTextSize(1);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(10, 110);
    tft.print(en_de("No shunt!", "Kein Shunt!"));
  }
}

void displayResistanceMode() {
  // Voltage reading
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(10, 35);
  tft.printf("V: %.3f V", voltage);
  
  // Resistance reading
  tft.setTextSize(3);
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.setCursor(10, 55);
  
  if (resistance >= 1000000) {
    tft.printf("%.1f M", resistance / 1000000);
    tft.write(234); // Ω symbol
  } else if (resistance >= 1000) {
    tft.printf("%.1f k", resistance / 1000);
    tft.write(234);
  } else if (resistance >= 1.0) {
    tft.printf("%.1f ", resistance);
    tft.write(234);
  } else if (resistance >= 0.001) {
    tft.printf("%.0f m", resistance * 1000);
    tft.write(234);
  } else {
    tft.print(en_de("Short", "Kurz"));
  }
  
  // Info
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setCursor(10, 85);
  tft.printf(en_de("Series R: %.0fk", "Reihen R: %.0fk"), SERIES_RESISTOR);
  
  // Draw bar (log scale approximation)
  float logR = log10(max(1.0f, resistance));
  drawVoltmeterBar(logR, 6.0, TFT_MAGENTA); // 1Ω to 1MΩ range
}

void onVoltmeterPageUpdate() {
  static unsigned long lastButtonPress = 0;
  unsigned long now = millis();
  
  // Handle mode switching with LEFT button
  if (now - lastButtonPress > 300) {
    if (digitalRead(LEFT_BUTTON) == LOW) {
      currentMode = (MeasurementMode)((currentMode + 1) % 3);
      minVoltage = 999.9;
      maxVoltage = 0.0;
      tft.fillScreen(TFT_BLACK);
      drawVoltmeterHeader();
      lastButtonPress = now;
      delay(200);
    }
    
    // RIGHT button to exit
    if (digitalRead(RIGHT_BUTTON) == LOW) {
      closePageIfAnyButtonIsPressed();
      return;
    }
  }
  
  if (millis() - lastVoltmeterReadTime > voltmeterReadInterval) {
    // Read and calculate values
    calculateElectricalValues();
    
    // Clear display area (keep header)
    tft.fillRect(0, 26, screenWidth, screenHeight - 26, TFT_BLACK);
    
    // Display based on current mode
    switch(currentMode) {
      case MODE_VOLTAGE:
        displayVoltageMode();
        break;
      case MODE_CURRENT:
        displayCurrentMode();
        break;
      case MODE_RESISTANCE:
        displayResistanceMode();
        break;
    }
    
    // Draw instructions
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(4, screenHeight - 12);
    tft.print(en_de("L:Mode R:Exit", "L:Modus R:Ende"));
    
    lastVoltmeterReadTime = millis();
  }
}