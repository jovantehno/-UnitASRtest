// UART Passthrough - Use ESP32-S3 as USB-to-TTL adapter for Unit-ASR
// This allows flashing firmware to Unit-ASR through the ESP32

#define ASR_RX_PIN 44  // ESP32 RX ← Unit-ASR TX
#define ASR_TX_PIN 43  // ESP32 TX → Unit-ASR RX

void setup() {
  // USB Serial
  Serial.begin(115200);

  // UART to Unit-ASR
  Serial1.begin(115200, SERIAL_8N1, ASR_RX_PIN, ASR_TX_PIN);

  delay(100);
  Serial.println("ESP32-S3 UART Passthrough Mode");
  Serial.println("Ready to flash Unit-ASR firmware");
  Serial.println("All data will be forwarded between USB and Unit-ASR");
  delay(1000);
}

void loop() {
  // Forward data from USB to Unit-ASR
  if (Serial.available()) {
    Serial1.write(Serial.read());
  }

  // Forward data from Unit-ASR to USB
  if (Serial1.available()) {
    Serial.write(Serial1.read());
  }
}
