#include "DeviceProps.h"
// or #include "device/DeviceProps.h"

#if defined(ARDUINO_AVR_UNO)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::AVR, "arduino-uno", "auno", \
  false,false,false,false,false,false,false,false, \
  0,0 }

#elif defined(ARDUINO_AVR_MEGA2560)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::AVR, "arduino-mega-2560", "amega", \
  false,false,false,false,false,false,false,false, \
  0,0 }

#elif defined(ARDUINO_AVR_LEONARDO)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::AVR, "arduino-leonardo", "aleo", \
  false,false,false,false,false,false,false,true, \
  0,0 }

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

// LilyGo T-Display S3 - Try multiple possible board definitions
#elif defined(ARDUINO_LILYGO_T_DISPLAY_S3) || \
      defined(ARDUINO_TTGO_TDISPLAY_S3) || \
      defined(LILYGO_T_DISPLAY_S3) || \
      (defined(CONFIG_IDF_TARGET_ESP32S3) && defined(T_DISPLAY_S3))
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_S3, "lilygo-t-display-s3", "e32ts3", \
  true,true,true,true,true,false,false,true, \
  320,170 }
  // would be cool
  //#include "DisplayT_S3.h"

#elif defined(ARDUINO_TTGO_T1)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_FAMILY, "esp32-ttgo-t1", "e32tt1", \
  true,true,true,true,false,false,true,false, \
  128,64 }
  // would be cool
  //#include "DisplayTTGO.h"

#elif defined(ARDUINO_ESP32_DEV) || defined(ARDUINO_ESP32_DEVKITC)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_FAMILY, "esp32-devkitc", "e32dkc", \
  false,true,true,true,false,false,true,false, \
  0,0 }

#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_S3, "esp32-s3-generic", "e32s3", \
  false,true,true,true,true,true,false,true, \
  0,0 }

#elif defined(CONFIG_IDF_TARGET_ESP32S2)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_S2, "esp32-s2-generic", "e32s2", \
  false,true,false,false,false,true,false,true, \
  0,0 }

#elif defined(CONFIG_IDF_TARGET_ESP32C3)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_C3, "esp32-c3-generic", "e32c3", \
  false,true,true,true,false,false,false,true, \
  0,0 }

#elif defined(ESP32)
#define DEVICE_PROPS_INIT { \
  DeviceFamily::ESP32_FAMILY, "esp32-generic", "e32gn", \
  false,true,true,true,false,false,true,false, \
  0,0 }

#else
#define DEVICE_PROPS_INIT { \
  DeviceFamily::Unknown, "unknown", "unk", \
  false,false,false,false,false,false,false,false, \
  0,0 }
#endif

static const DeviceProps DEV = DEVICE_PROPS_INIT;

const DeviceProps& getDeviceProps() {
  return DEV;
}
