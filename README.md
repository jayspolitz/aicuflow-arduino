# Arduino-ESP32 Machine Learning using Aicuflow

Connect to [aicuflow](https://aicuflow.com) using an ESP-32 or similar Microcontroller.

See [documentation](https://aicuflow.com/docs/library/arduino)

## TFT Setup

```cpp
// TFT_eSPI: Needs modifications under Arduino/libraries/TFT_eSPI/User_Setup_Select.h:
//   UNCOMMENT THIS in that file:
//   // #include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT
//   COMMENT THIS out in that file:
//   #include <User_Setup.h>           // Default setup is root library folder
```
