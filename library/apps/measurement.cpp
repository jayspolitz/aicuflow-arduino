#include "measurement.h"

// connecting
void connectWifiOrTimeout() {
  if (device->has_display) tft.print(en_de("Connecting...", "Verbinden..."));
  WiFi.mode(WIFI_STA);
  WiFi.begin(wlanSSID, wlanPass);

  // try to connect (until timeout)
  unsigned long start = millis();
  while (
    (WiFi.status() != WL_CONNECTED) // not connected
    && (!WIFI_TIMEOUT || (millis() - start < WIFI_TIMEOUT)) // timeout
  ) {
    delay(500);
    if (device->has_display) tft.print("."); // Loading progress
  }

  // no connection = off
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    wifiAvailable = false;
    tft.print("\n");
    tft.setTextColor(TFT_RED);
    tft.println(en_de("WiFi failed :/", "Wlan fehlgeschlagen"));
    tft.setTextColor(TFT_WHITE);
    return;
  } else { // connected = on
    wifiAvailable = true;
  }

  // set up wifi stuff
  client.setInsecure(); // TLS workaround
  aicu.setWiFiClient(&client);
  
  // print
  if (wifiAvailable && device->has_display) {
    tft.print("\n");
    tft.setTextColor(TFT_GREEN);
    tft.print(en_de("WiFi: ", "Wlan:")); 
    tft.println(WiFi.localIP());
    tft.setTextColor(TFT_WHITE);
  } else if (device->has_display) tft.print("\n");
  Serial.print("Wifi-Mac: "); Serial.println(WiFi.macAddress());
  delay(500);
}

void connectAPI() {
  if (!aicu.login(aicuMail, aicuPass)) {
    if (device->has_display){
      tft.setTextColor(TFT_RED);
      tft.println(en_de("Login failed! :/", "Anmeldefehler! :/"));
      tft.setTextColor(TFT_WHITE);
    }
    Serial.println("Login failed!");
    return;
  } else {
    if (device->has_display){
      tft.setTextColor(TFT_GREEN);
      tft.println(en_de("Aicuflow connected!", "Aicuflow verbunden!"));
      tft.setTextColor(TFT_WHITE);
    }
    Serial.println("Login success!");
  }
}

// measurement: json collect & stream, dtype
static DynamicJsonDocument points(384*POINTS_BATCH_SIZE*2);
static JsonArray arr;
static uint16_t count = 0;
static int32_t PERIOD_US = MEASURE_DELAY_MS * 1000UL;
void initPoints() {
  points.clear();
  arr = points.to<JsonArray>();
  count = 0;
}
struct JsonBatch { DynamicJsonDocument* doc; };
QueueHandle_t flushQueue; // Async queue for sending
void flushSamplesAsync() {
  if (count == 0) return;
  DynamicJsonDocument* batch =
    new DynamicJsonDocument(points.memoryUsage() + 384 * POINTS_BATCH_SIZE * 2);
  *batch = points; // deep copy
  JsonBatch jb{ batch };
  if (xQueueSend(flushQueue, &jb, 0) != pdTRUE) {
    delete batch; // drop if queue full
  }
  points.clear();
  arr = points.to<JsonArray>();
  count = 0;
}
void addSampleAndAutoSend() {
  JsonObject obj = arr.createNestedObject();
  sensors.toJson(obj);
  if (++count >= POINTS_BATCH_SIZE) flushSamplesAsync();
}
void sendTask(void* parameter) {
  // async Send Task (Core 1 on ESP32)
  // this is used so we don't see the graph stutter
  JsonBatch jb;
  for (;;) {
    if (xQueueReceive(flushQueue, &jb, portMAX_DELAY) == pdTRUE) {
      aicu.sendTimeseriesPoints(aicuFlow, streamFileName, *jb.doc);
      delete jb.doc; // free memory
    }
  }
}
void startCPUSendTask() {
  flushQueue = xQueueCreate(8, sizeof(JsonBatch));
  // FreeRTOS Call see https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/system/freertos.html
  xTaskCreatePinnedToCore(sendTask, "sendTask", 8192, NULL, 1, NULL, 1);
}

// page

void onMeasurePageUpdate() {
  // Precise time
  static uint32_t next = micros();
  uint32_t now = micros();
  if ((int32_t)(now - next) < 0) return;
  next += PERIOD_US;

  // Measure Data
  sensors.measure();

  // Forward Data
  if (VERBOSE && !device->has_display)    sensors.printValues(); // May be BLOCKING!
  if (pageManager->screenAwake && device->has_display) // save energy
                                         sensors.updateGraphs();
  if (wifiAvailable && device->has_wifi)  addSampleAndAutoSend();

  closePageIfBothLongPressed();
}