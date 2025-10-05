// Check Unit-ASR firmware version and test voice recognition
#include <Arduino.h>

#define ASR_RX_PIN 44  // ESP32 RX ← Unit-ASR TX
#define ASR_TX_PIN 43  // ESP32 TX → Unit-ASR RX

void sendCommand(uint8_t cmd) {
  uint8_t packet[] = {0xAA, 0x55, cmd, 0x55, 0xAA};
  Serial1.write(packet, 5);
  Serial.print("Sent command 0x");
  if(cmd < 16) Serial.print("0");
  Serial.println(cmd, HEX);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n=== Unit-ASR Firmware Check ===\n");

  // Init UART
  Serial1.begin(115200, SERIAL_8N1, ASR_RX_PIN, ASR_TX_PIN);

  delay(500);

  // Try firmware version command (0x45 from M5Stack library)
  Serial.println("Querying firmware version (0x45)...");
  sendCommand(0x45);
  delay(1000);

  if (Serial1.available() > 0) {
    Serial.print("Response: ");
    while(Serial1.available() > 0) {
      uint8_t b = Serial1.read();
      Serial.print("0x");
      if(b < 16) Serial.print("0");
      Serial.print(b, HEX);
      Serial.print(" ");
    }
    Serial.println("\n");
  } else {
    Serial.println("No response\n");
  }

  // Try other potential status commands
  uint8_t test_cmds[] = {0x40, 0x41, 0x42, 0x43, 0x44, 0x46, 0x47};

  for(int i = 0; i < 7; i++) {
    Serial.print("Testing command 0x");
    if(test_cmds[i] < 16) Serial.print("0");
    Serial.print(test_cmds[i], HEX);
    Serial.print(": ");

    sendCommand(test_cmds[i]);
    delay(500);

    if (Serial1.available() > 0) {
      Serial.print("Response: ");
      while(Serial1.available() > 0) {
        uint8_t b = Serial1.read();
        Serial.print("0x");
        if(b < 16) Serial.print("0");
        Serial.print(b, HEX);
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.println("No response");
    }
  }

  Serial.println("\n=== Test Complete ===");
  Serial.println("\nNow listening for voice commands...");
  Serial.println("Try saying: 'turn on', 'turn off', 'play', 'pause'");
  Serial.println("(If no factory firmware, voice won't work)\n");
}

void loop() {
  // Listen for any data from Unit-ASR
  if (Serial1.available() >= 5) {
    uint8_t data[5];
    Serial1.readBytes(data, 5);

    Serial.print("Received: ");
    for(int i = 0; i < 5; i++) {
      Serial.print("0x");
      if(data[i] < 16) Serial.print("0");
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Check if valid packet
    if (data[0] == 0xAA && data[1] == 0x55 &&
        data[3] == 0x55 && data[4] == 0xAA) {

      uint8_t cmd = data[2];
      Serial.print(">>> VOICE COMMAND DETECTED: 0x");
      if(cmd < 16) Serial.print("0");
      Serial.print(cmd, HEX);
      Serial.print(" (");
      Serial.print(cmd);
      Serial.println(")");

      // Decode common commands based on M5Stack library
      switch(cmd) {
        case 0xFF: Serial.println("    Wake word"); break;
        case 0x01: Serial.println("    Turn on the light"); break;
        case 0x02: Serial.println("    Turn off the light"); break;
        case 0x03: Serial.println("    Pause"); break;
        case 0x04: Serial.println("    Play"); break;
        case 0x05: Serial.println("    Next song"); break;
        case 0x06: Serial.println("    Previous song"); break;
        default: Serial.println("    Unknown command"); break;
      }
      Serial.println();
    }
  }

  delay(10);
}
