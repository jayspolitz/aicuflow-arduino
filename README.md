> **Try it out, and use this code for a PoC or your custom projects!**

# Aicuflow Arduino Library

[![](https://aicuflow.com/api/badge?label=&message=data%20storage&color=036b64)](https://aicuflow.com/docs/library/arduino)
[![](https://aicuflow.com/api/badge?label=&message=data%20visualization&color=036b64)](https://aicuflow.com/docs/library/arduino)
[![](https://aicuflow.com/api/badge?label=&message=machine%20learning&color=036b64)](https://aicuflow.com)
[![](https://aicuflow.com/api/badge?label=&message=arduino%20compatible&color=036b64)](https://aicuflow.com)

Machine learning and Arduino? [Aicuflow](https://aicuflow.com) by AICU GmbH makes it possible! Actually, more than that: pretty menus, inputs, plots and sensor streaming to the cloud - with some free data storage.

**Please star this repo, so more people can see it :)**

This code automatically detects your device properties (wifi, tft display, sensors) and adjusts to it - you don't need to set anything up after loading this repository. No need to install external libraries or dependencies, they are packaged under the `imports` folder.

If you just want to get the `AicuClient.cpp` code, it is under `library/aicuflow`. But there is more cool features worthwile exploring in this codebase (check images below)!

![](https://aicuflow.com/docs/library/arduino/esp32title.png)

(Preferably you would want to use WiFi capable boards similar to the ESP32 for this)

Connect to [aicuflow](https://aicuflow.com) using an ESP-32 or similar Microcontroller.
For more details and examples, refer to our [documentation](https://aicuflow.com/docs/library/arduino).

[![](https://aicuflow.com/api/badge?label=&message=free%20registration&color=036b64)](https://aicuflow.com/signup)
[![](https://aicuflow.com/api/badge?label=&message=quick%20setup&color=036b64)](https://aicuflow.com/docs/library/arduino)

## Cool demo screenshots

<div align="center"> <img src="https://aicuflow.com/docs/library/arduino/esp32sensors.webp" width="50%" /> </div>

![Cool demo screenshots](https://aicuflow.com/docs/library/arduino/esp32coolscreens.webp)

## Quickstart

Download the repository source first:

```bash
git clone https://github.com/AICU-HEALTH/aicuflow-arduino
```

[Click here to view full documentation](https://aicuflow.com/docs/library/arduino)

### Short install guide

1. Install Arduino IDE (v2.3.7 at time of writing)
2. Install ESP32 Board in it (official Espressif version)
3. Open `aicuflow-arduino` in arduino IDE and customise settings section (Wifi, [Aicuflow Flow Id](https://aicuflow.com/flows), Details)

In the main sketch (`aicuflow-arduino.ino`), search for **add more**, and you will find many options to add customisations (sensors, menus, pages).

### Change factory settings

You can manually set up the settings on each device via buttons, or flash them with custom built firmware and your own factory settings.

Check `aicuflow-arduino.ino` and customise factory settings.

```cpp
// === Setup ===
// needed to work
const char* WLAN_SSID = "your-wlan"; // connect to a stable WPA2 Wifi
const char* WLAN_PASS = "your-pass";
const char* AICU_USER = "your-mail"; // register at https://aicuflow.com/signup
const char* AICU_PASS = "your-pass";
const char* PROJ_FLOW = "your-ai-cu-flow-uuid"; // create / select at https://aicuflow.com/flows

// optional options
const char* PROJ_FILE = "esp32"; // will be auto created with .arrow extension
const int VERBOSE = true;
const char* API_URL = "https://prod-backend.aicuflow.com"; // dev or prod
const char* DEVICE_ID_SUFFIX = ""; // 0,1,2,3 appended to id if you have multiple of same kind
const int POINTS_BATCH_SIZE = 64; // 64 always works, 256 sometimes did, but may be too large.
const int MEASURE_DELAY_MS = 100;
const int SCREEN_IDLE_MS = 60000; // also needs TFT_BL eg 38
const int WIFI_TIMEOUT = 10000; // 10s, 0 -> blocking till wifi
```

## Supported Boards and Fixes

We tested this on a hand full of arduino-compatible devices. To be on the safe side, we recommend you use one of these boards. Others may still work, but are not present in our [hardware lab](https://aicuflow.com/docs/library/arduino#the-aicuflow-lab).

- ESP32 Wroom
- ESP32 Devkit
- ESP32 Lilygo T-Display S3
- ESP32 TTGO-T1

More are coming once we are moving to a stable beta.

Known issues and Fixes:

- **Not uploading?** Can't find its serial port? -> If you're on the Lilygo T-Display S3, you can activate the loader mode by long pressing left, keeping, then short clicking sidebutton. On other ESP32 Modules

- **Compile error** with tft on non-tft devices containing `gpio_input_get` -> compile on arduino platform `esp32:esp32@2.0.17` instead of the default `3.1.0` (everything should work on there too, but imports are a bit different)

- **Sketch too large** - this can happen if your boards partition scheme (in menu `Tool>Partition Scheme`) is defined wrongly (for example on the ttgo-t1). You can resolve it easily by switching to `Huge App` or another larger scheme that has less file system reservations.

## Features and Roadmap

This [arduino client](https://aicuflow.com/docs/library/arduino) library for [aicuflow](https://aicuflow.com/) works out of the box or can be included as a library in custom projects. We're expanding support for more devices and use-cases.

### Already working features

Aicuflow-Arduino Sketch (`aicuflow-arduino.ino`)

- automatic measurement of all on-chip sensors
- wifi connection with timeout
- aicuflow logo tft boot animation and screen timeout
- settings configuration using device buttons
- apps: scan wifi & bt
- games: snake, game of life, mandelbrot
- multilingual mode (EN, DE)

Library (folder `library/*`, see `examples/*`)

- aicuflow api client with core endpoints and auth
- automatic scrolling line graphs on tft display
- simple sensor detection, measuring and chunked streaming
- user driveable menus & config
- auto tft_espi screen selection (needs more boards)
- freezed libraries ArduinoJson and TFT_eSPI

### Planned features

Useful Feats:

- more boards (see [list of almost all microcontrollers](https://github.com/AICU-HEALTH/aicuflow-arduino/blob/main/docs/Microcontrollers.md))
- automatic aicu session refreshing
- status indicators (wifi, power, ...)
- publish library to arduino list

Cool Stuff:

- multi-wifi setup
- automatic sensor plug and play (would be cool)
- station mode to config using webserver
- wifi monitoring as time series sensor
- bluetooth phone integration
- qr code render on tft?

Optional Future:

- configuration from [aicuflow platform](https://aicuflow.com/flows)

## Contribution Guide

**Please star this repo, so more people can see it :)**

Check [documentation](https://aicuflow.com/docs/library/arduino) and talk to [Finn](https://github.com/finnmglas) about this.

### The team

Our team at AICU comes together every few months for a team retreat. During the last one we all worked together on making all parts of the aicuflow-arduino connection work nicely. This could become a tradition.

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
