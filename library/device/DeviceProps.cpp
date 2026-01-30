#include "DeviceProps.h"
// or #include "device/DeviceProps.h"

#if defined(ARDUINO_AVR_UNO)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::AVR_FAMILY, "arduino-uno", "auno", \
  false,false,false,false,false,false,false,false, \
  0,0 }
#define IS_AVR 1
#define IS_ESP 0

#elif defined(ARDUINO_AVR_MEGA2560)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::AVR_FAMILY, "arduino-mega-2560", "amega", \
  false,false,false,false,false,false,false,false, \
  0,0 }
#define IS_AVR 1
#define IS_ESP 0

#elif defined(ARDUINO_AVR_LEONARDO)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::AVR_FAMILY, "arduino-leonardo", "aleo", \
  false,false,false,false,false,false,false,true, \
  0,0 }
#define IS_AVR 1
#define IS_ESP 0

#elif defined(ESP8266)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP8266, \
  /* slug */ \
  #if defined(ARDUINO_ESP8266_NODEMCU) \
    "esp8266-nodemcu", "e82nmcu" \
  #elif defined(ARDUINO_ESP8266_WEMOS_D1MINI) \
    "esp8266-d1-mini", "e82d1m1" \
  #else \
    "esp8266-generic", "e8266gn" \
  #endif \
  , \
  false, true, false, false, false, false, false, false, \
  0,0 }
#define IS_AVR 0
#define IS_ESP 1

// LilyGo T-Display S3 - Try multiple possible board definitions
#elif defined(ARDUINO_LILYGO_T_DISPLAY_S3) || \
      defined(ARDUINO_TTGO_TDISPLAY_S3) || \
      defined(LILYGO_T_DISPLAY_S3) || \
      (defined(CONFIG_IDF_TARGET_ESP32S3) && defined(T_DISPLAY_S3))
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_S3, "lilygo-t-display-s3", "e32ts3", \
  true,true,true,true,true,false,false,true, \
  320,170 }
#define IS_AVR 0
#define IS_ESP 1

#elif defined(ARDUINO_TTGO_T1)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_FAMILY, "esp32-ttgo-t1", "e32tt1", \
  true,true,true,true,false,false,true,false, \
  128,64 }
#define IS_AVR 0
#define IS_ESP 1

#elif defined(ARDUINO_ESP32_DEV) || defined(ARDUINO_ESP32_DEVKITC)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_FAMILY, "esp32-devkitc", "e32dkc", \
  false,true,true,true,false,false,true,false, \
  0,0 }
#define IS_AVR 0
#define IS_ESP 1

#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_S3, "esp32-s3-generic", "e32s3", \
  false,true,true,true,true,true,false,true, \
  0,0 }
#define IS_AVR 0
#define IS_ESP 1

#elif defined(CONFIG_IDF_TARGET_ESP32S2)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_S2, "esp32-s2-generic", "e32s2", \
  false,true,false,false,false,true,false,true, \
  0,0 }
#define IS_AVR 0
#define IS_ESP 1

#elif defined(CONFIG_IDF_TARGET_ESP32C3)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_C3, "esp32-c3-generic", "e32c3", \
  false,true,true,true,false,false,false,true, \
  0,0 }
#define IS_AVR 0
#define IS_ESP 1

#elif defined(ESP32)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_FAMILY, "esp32-generic", "e32gn", \
  false,true,true,true,false,false,true,false, \
  0,0 }
#define IS_AVR 0
#define IS_ESP 1

#else
#define DEVICE_PROPS_INIT { \
  DeviceFamily::Unknown, "unknown", "unk", \
  false,false,false,false,false,false,false,false, \
  0,0 }
#define IS_AVR 0
#define IS_ESP 1
#endif

static const DeviceProps DEV = DEVICE_PROPS_INIT;

const DeviceProps& getDeviceProps() {
  return DEV;
}

// define missing functions the ESP32 IDF has
#if IS_AVR
// tft colors
#define TFT_PURPLE 0
#define TFT_BLUE 0
#define TFT_GREEN 0
#define TFT_RED 0
#define TFT_ORANGE 0
#define TFT_YELLOW 0
#define TFT_CYAN 0
#define TFT_MAGENTA 0
#define TFT_BLACK 0
#define TFT_WHITE 0
#define TFT_BL -1

// make arduinojson not break
#define ARDUINOJSON_USE_LONG_LONG 1

// temperature
double temperatureRead() {
  unsigned int wADC;
  double t;
  // 1. Set reference to internal 1.1V & select channel 8 (internal temp sensor)
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // Enable ADC
  delay(20);            // Wait for reference to stabilize
  // 2. Start conversion
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC)); // Wait for conversion to finish
  // 3. Read ADC value (ADCW reads ADCL & ADCH correctly)
  wADC = ADCW;
  // 4. Convert to Celsius (standard formula: (ADC - Offset) / Slope)
  // These values (324.31 and 1.22) vary per chip
  t = (wADC - 324.31) / 1.22;
  return t;
}
#endif
