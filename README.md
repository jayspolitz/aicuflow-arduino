# Aicuflow Arduino Library

[![](https://aicuflow.com/api/badge?label=&message=data%20storage&color=036b64)](https://aicuflow.com/docs/library/arduino)
[![](https://aicuflow.com/api/badge?label=&message=data%20visualization&color=036b64)](https://aicuflow.com/docs/library/arduino)
[![](https://aicuflow.com/api/badge?label=&message=machine%20learning&color=036b64)](https://aicuflow.com)
[![](https://aicuflow.com/api/badge?label=&message=arduino%20compatible&color=036b64)](https://aicuflow.com)

Machine learning and Arduino? [Aicuflow](https://aicuflow.com) by AICU GmbH makes it possible!

![](https://aicuflow.com/docs/library/arduino/esp32title.png)

(Preferably you would want to use WiFi capable oards similar to the ESP32 for this)

Connect to [aicuflow](https://aicuflow.com) using an ESP-32 or similar Microcontroller.
For more details and examples, refer to our [documentation](https://aicuflow.com/docs/library/arduino).

[![](https://aicuflow.com/api/badge?label=&message=free%20registration&color=036b64)](https://aicuflow.com/signup)
[![](https://aicuflow.com/api/badge?label=&message=quick%20setup&color=036b64)](https://aicuflow.com/docs/library/arduino)

## Quickstart

[Click here to view full documentation](https://aicuflow.com/docs/library/arduino)

Or shorter:

1. Install Arduino IDE
2. Install ESP32 Board in it
3. Clone this Repo
4. Open in arduino IDE and customise settings section (Wifi, [Aicuflow Flow Id](https://aicuflow.com/flows), Details)

## Setup troubleshooting

### Loader mode of Lilygo T-Display S3

Can't find its serial port? Long press left, keep, short click sidebutton.

### TFT Setup for ttgo t1

```cpp
// TFT_eSPI: Needs modifications under Arduino/libraries/TFT_eSPI/User_Setup_Select.h:
//   UNCOMMENT THIS in that file:
//   // #include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT
//   COMMENT THIS out in that file:
//   #include <User_Setup.h>           // Default setup is root library folder
```

## Contributing

Check the docs and talk to Finn about this.

### Do not commit your device settings

Use `*.settings` instead.\
Example: `device1.settings`

And then copy these settings contents into your sketch to proceed, instead of committing.

## Contributing?

Check [documentation](https://aicuflow.com/docs/library/arduino) and contact [Finn](https://github.com/finnmglas)
