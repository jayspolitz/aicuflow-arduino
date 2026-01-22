#pragma once
#include <Arduino.h>
#include <stdint.h>

enum class DeviceFamily : uint8_t {
  Unknown,
  AVR,
  ESP8266,
  ESP32,
  ESP32_S2,
  ESP32_S3,
  ESP32_C3
};

struct DeviceProps {
  DeviceFamily family;
  const char* kind_slug;

  bool has_display;
  bool has_wifi;
  bool has_bt;
  bool has_ble;
  bool has_psram;
  bool has_touch;
  bool has_hall_sensor;
  bool has_usb;

  uint16_t display_width;
  uint16_t display_height;
};

const DeviceProps& getDeviceProps();