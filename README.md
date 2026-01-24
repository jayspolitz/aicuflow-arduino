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
2. Install ESP32 Board in it (official Espressif version)
3. Install Libraries (Search in Arduino IDE)
   - TFT_eSPI (needs setup)
   - ArduinoJSON
4. Clone this Repo
5. Open `aicuflow-arduino` in arduino IDE and customise settings section (Wifi, [Aicuflow Flow Id](https://aicuflow.com/flows), Details)

## Setup troubleshooting

### Supported devices

We tested this on a hand full of arduino-compatible devices. To be on the safe side, we recommend you use one of these boards. Others may still work, but are not present in our [hardware lab](https://aicuflow.com/docs/library/arduino).

- ESP32 Wroom
- ESP32 Devkit
- ESP32 Lilygo T-Display S3
- ESP32 TTGO-T1

More are coming once we are moving to a stable beta.

### Loader mode of Lilygo T-Display S3

Can't find its serial port? Long press left, keep, short click sidebutton.

### TFT Setup

You need to customise `User_Setup_Select.h` file in the TFT_eSPI Library installation. For esp32 ttgo t1:

```cpp
// TFT_eSPI: Needs modifications under Arduino/libraries/TFT_eSPI/User_Setup_Select.h:
//   UNCOMMENT THIS in that file:
//   // #include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT
//   COMMENT THIS out in that file:
//   #include <User_Setup.h>           // Default setup is root library folder
```

Similar for the Adafruit T-Display. For other boards

## Roadmap

This [arduino client](https://aicuflow.com/docs/library/arduino) library for [aicuflow](https://aicuflow.com/) is quite new. So lots to do.

### Already working features

- wifi connection with timeout
- aicuflow api client with core endpoints and auth
- aicuflow logo tft boot animation and screen timeout
- automatic scrolling line graphs on tft display
- simple sensor detection, measuring and chunked streaming

### Planned features

needed

- more boards
- automatic aicu session refreshing
- user driveable menus & config
- status indicators (wifi, power, ...)
- publish library to arduino list

cool

- multilingual mode
- auto tft config
- multi-wifi setup
- automatic sensor plug and play (would be cool)
- station mode to config using webserver
- wifi monitoring as time series sensor
- bluetooth phone integration
- qr code render on tft?

optional

- configuration from [aicuflow platform](https://aicuflow.com/flows)

## Contributing

Check [documentation](https://aicuflow.com/docs/library/arduino) and talk to [Finn](https://github.com/finnmglas) about this

### Simple process for clean code

Write great code please!

1. Clone repo and work with it
2. Fork repo as contributor (or create a new branch `[name]/[feature-name]` as team member)
3. Work on your branch!
   - no breaking changes unless discussed
   - no errors fails on supported devices!
   - small, understandable commits only!
   - do not break the library structure
4. Get your code merged!
   - no unreviewed merging of code to main

Thanks!

### Do not commit your device settings

Use `*.settings` instead.\
Example: `device1.settings`

And then copy these settings contents into your sketch to proceed, instead of committing.
