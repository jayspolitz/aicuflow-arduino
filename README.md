> **Try it out, and use this code for a PoC or your custom projects!**

# Aicuflow Arduino Library

[![](https://aicuflow.com/api/badge?label=&message=data%20storage&color=036b64)](https://aicuflow.com/docs/library/arduino)
[![](https://aicuflow.com/api/badge?label=&message=data%20visualization&color=036b64)](https://aicuflow.com/docs/library/arduino)
[![](https://aicuflow.com/api/badge?label=&message=machine%20learning&color=036b64)](https://aicuflow.com)
[![](https://aicuflow.com/api/badge?label=&message=arduino%20compatible&color=036b64)](https://aicuflow.com)

Machine learning and Arduino? [Aicuflow](https://aicuflow.com) by AICU GmbH makes it possible! Actually, more than that: pretty menus, inputs, plots and sensor streaming to the cloud - with some free data storage.

![](https://aicuflow.com/docs/library/arduino/esp32title.png)

(Preferably you would want to use WiFi capable oards similar to the ESP32 for this)

Connect to [aicuflow](https://aicuflow.com) using an ESP-32 or similar Microcontroller.
For more details and examples, refer to our [documentation](https://aicuflow.com/docs/library/arduino).

[![](https://aicuflow.com/api/badge?label=&message=free%20registration&color=036b64)](https://aicuflow.com/signup)
[![](https://aicuflow.com/api/badge?label=&message=quick%20setup&color=036b64)](https://aicuflow.com/docs/library/arduino)

## Quickstart

Download the repository source first:

```bash
git clone https://github.com/AICU-HEALTH/aicuflow-arduino
```

[Click here to view full documentation](https://aicuflow.com/docs/library/arduino)

Short install guide:

1. Install Arduino IDE
2. Install ESP32 Board in it (official Espressif version)
3. Install Libraries (Search in Arduino IDE)
   - TFT_eSPI (needs setup, see TFT Setup below)
   - ArduinoJSON
4. Open `aicuflow-arduino` in arduino IDE and customise settings section (Wifi, [Aicuflow Flow Id](https://aicuflow.com/flows), Details)

In the main sketch (`aicuflow-arduino.ino`), search for **add more**, and you will find many options to add customisations (sensors, menus, pages).

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

Open your file manager. Go to Documents > Arduino > libraries > TFT_eSPI.

You need to customise `User_Setup_Select.h` file in there (comment 1 line and uncomment 1 line).

Example for esp32 ttgo t1:

```cpp
// TFT_eSPI: Needs modifications under Arduino/libraries/TFT_eSPI/User_Setup_Select.h:
//   UNCOMMENT THIS in that file:
//   // #include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT
//   COMMENT THIS out in that file:
//   #include <User_Setup.h>           // Default setup is root library folder
```

Similar for the Adafruit T-Display.

For other boards, you just search the board / display name in the setup file. It basically contains definitions of the connections between your board and the display.

### Known issues and Fixes

- **Compile error** with tft on non-tft devices containing `gpio_input_get` -> compile on arduino platform `esp32:esp32@2.0.17` instead of the default `3.1.0` (everything should work on there too, but imports are a bit different)

## Roadmap

This [arduino client](https://aicuflow.com/docs/library/arduino) library for [aicuflow](https://aicuflow.com/) is quite new. So lots to do.

### Already working features

Library (folder `library/*`, see `examples/*`)

- aicuflow api client with core endpoints and auth
- automatic scrolling line graphs on tft display
- simple sensor detection, measuring and chunked streaming
- user driveable menus & config

Aicu Embedded OS (`aicuflow-arduino.ino`)

- wifi connection with timeout
- aicuflow logo tft boot animation and screen timeout

### Planned features

needed

- more boards (see [list of almost all microcontrollers](https://github.com/AICU-HEALTH/aicuflow-arduino/blob/main/docs/Microcontrollers.md))
- automatic aicu session refreshing
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
