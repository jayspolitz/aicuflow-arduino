// all pins (experimental compile feature)
#include "../device/DeviceProps.h"
#include "../sensors/SensorMeasurement.h"

// use the global ones
extern const DeviceProps* device;
extern SensorMeasurement sensors;

void registerAllOtherPins() {
    #if IS_ESP // ESP Boards: some more pins
        // 1. Definiere absolut sichere Pin-Listen (nur Pins, die physisch an den Header-Leisten liegen)
        // LilyGo T-Display-S3: Nur diese Pins sind sicher als IO nutzbar
        int safeS3[] = {1, 2, 3, 10, 11, 12, 13, 14, 16, 17, 18, 21, 43, 44}; 

        // ESP32-TTGO-T1: Nur diese Pins sind sicher (ohne TFT/Serial/Flash)
        int safeT1[] = {2, 12, 13, 15, 17, 21, 22, 25, 26, 27, 32, 33};

        bool isS3 = (device->kind_slug == "lilygo-t-display-s3");
        bool isT1 = (device->kind_slug == "esp32-ttgo-t1");

        // Wähle die Liste basierend auf dem Board
        int* currentPins;
        int pinCount = 0;

        if (isS3) {
            currentPins = safeS3;
            pinCount = sizeof(safeS3) / sizeof(safeS3[0]);
        } else if (isT1) {
            currentPins = safeT1;
            pinCount = sizeof(safeT1) / sizeof(safeT1[0]);
        } else {
            return; // Oder Standard-Logik für andere Boards
        }

        for (int i = 0; i < pinCount; i++) {
            int pin = currentPins[i];

            // Schutz vor Button-Überschreibung
            if (pin == LEFT_BUTTON || pin == RIGHT_BUTTON) continue;

            // Digitaler Part
            pinMode(pin, INPUT_PULLUP);
            
            // Analoger Part (Nur ADC1 nutzen für WiFi-Stabilität)
            // S3 ADC1: GPIO 1-10 | T1 ADC1: GPIO 32-39
            bool isADC1 = (isS3 && pin >= 1 && pin <= 10) || (isT1 && pin >= 32);
            
            if (isADC1) {
                char* aName = new char[8];
                sprintf(aName, "A%d", pin);
                sensors.registerSensor(aName, [pin]() -> float { return (float)analogRead(pin); }, 0, true, false);
            } else {
            char* dName = new char[8];
            sprintf(dName, "D%d", pin);
            sensors.registerSensor(dName, [pin](){ return (float) !digitalRead(pin); }, 0, true, false);
            }
        }
    #else      // AVR Boards: register all pins
        // all digital pins
        pinMode(2, INPUT_PULLUP);
        pinMode(3, INPUT_PULLUP);
        pinMode(4, INPUT_PULLUP);
        pinMode(5, INPUT_PULLUP);
        pinMode(6, INPUT_PULLUP);
        pinMode(7, INPUT_PULLUP);
        sensors.registerSensor("D2", [](){ return (float) !digitalRead(2); }, 0, true, false);
        sensors.registerSensor("D3", [](){ return (float) !digitalRead(3); }, 0, true, false);
        sensors.registerSensor("D4", [](){ return (float) !digitalRead(4); }, 0, true, false);
        sensors.registerSensor("D5", [](){ return (float) !digitalRead(5); }, 0, true, false);
        sensors.registerSensor("D6", [](){ return (float) !digitalRead(6); }, 0, true, false);
        sensors.registerSensor("D7", [](){ return (float) !digitalRead(7); }, 0, true, false);
        // second row of digital pins
        pinMode(8, INPUT_PULLUP);
        pinMode(9, INPUT_PULLUP);
        pinMode(10, INPUT_PULLUP);
        pinMode(11, INPUT_PULLUP);
        pinMode(12, INPUT_PULLUP);
        pinMode(13, INPUT_PULLUP);
        sensors.registerSensor("D8", [](){ return (float) !digitalRead(8); }, 0, true, false);
        sensors.registerSensor("D9", [](){ return (float) !digitalRead(9); }, 0, true, false);
        sensors.registerSensor("D10", [](){ return (float) !digitalRead(10); }, 0, true, false);
        sensors.registerSensor("D11", [](){ return (float) !digitalRead(11); }, 0, true, false);
        sensors.registerSensor("D12", [](){ return (float) !digitalRead(12); }, 0, true, false);
        sensors.registerSensor("D13", [](){ return (float) !digitalRead(13); }, 0, true, false);
        // all analog pins
        pinMode(A0, INPUT);
        pinMode(A1, INPUT);
        pinMode(A2, INPUT);
        pinMode(A3, INPUT);
        pinMode(A4, INPUT);
        pinMode(A5, INPUT);
        sensors.registerSensor("A0", []() -> float { return (float)analogRead(A0); }, 0, true, false);
        sensors.registerSensor("A1", []() -> float { return (float)analogRead(A1); }, 0, true, false);
        sensors.registerSensor("A2", []() -> float { return (float)analogRead(A2); }, 0, true, false);
        sensors.registerSensor("A3", []() -> float { return (float)analogRead(A3); }, 0, true, false);
        sensors.registerSensor("A4", []() -> float { return (float)analogRead(A4); }, 0, true, false);
        sensors.registerSensor("A5", []() -> float { return (float)analogRead(A5); }, 0, true, false);
    #endif
}
