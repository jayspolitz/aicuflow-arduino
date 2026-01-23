# Arduino-ESP32 Machine Learning using Aicuflow

Connect to [aicuflow](https://aicuflow.com) using an ESP-32 or similar Microcontroller.

See [documentation](https://aicuflow.com/docs/library/arduino)

## Loader mode of Lilygo T-Display S3

Can't find its serial port? Long press left, keep, short click sidebutton.

## TFT Setup for ttgo t1

```cpp
// TFT_eSPI: Needs modifications under Arduino/libraries/TFT_eSPI/User_Setup_Select.h:
//   UNCOMMENT THIS in that file:
//   // #include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT
//   COMMENT THIS out in that file:
//   #include <User_Setup.h>           // Default setup is root library folder
```

## Do not commit your device settings

Use `*.settings.ino` instead.\
Example: `device1.settings.ino`

And then copy these settings contents into your sketch to proceed, instead of committing.
