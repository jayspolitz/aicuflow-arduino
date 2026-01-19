void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("ESP32 booted");
}

void loop() {
  Serial.print("Millis: ");
  Serial.println(millis());
  delay(1000);
}
