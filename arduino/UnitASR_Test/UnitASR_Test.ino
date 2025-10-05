#include <Arduino.h>

// Unit-ASR UART pins (GPIO43/44 are free on this board)
#define ASR_RX_PIN 44  // ESP32 RX ← Unit-ASR TX
#define ASR_TX_PIN 43  // ESP32 TX → Unit-ASR RX

// Send wake-up command to Unit-ASR
void sendWakeup() {
  uint8_t wakeCmd[] = {0xAA, 0x55, 0xFF, 0x55, 0xAA};
  Serial1.write(wakeCmd, 5);
  Serial.println("Sent wake-up command to Unit-ASR");
}

// Send query command (example)
void sendQuery() {
  uint8_t queryCmd[] = {0xAA, 0x55, 0x00, 0x55, 0xAA};
  Serial1.write(queryCmd, 5);
  Serial.println("Sent query command to Unit-ASR");
}

// Try to query firmware version or status
void queryFirmware() {
  // Some possible query commands from M5Stack library
  uint8_t queries[][5] = {
    {0xAA, 0x55, 0x01, 0x55, 0xAA},  // Query 1
    {0xAA, 0x55, 0x02, 0x55, 0xAA},  // Query 2
    {0xAA, 0x55, 0xFE, 0x55, 0xAA},  // Query FE
    {0xAA, 0x55, 0xFD, 0x55, 0xAA},  // Query FD
  };

  for(int i = 0; i < 4; i++) {
    Serial.print("Sending query 0x");
    if(queries[i][2] < 16) Serial.print("0");
    Serial.print(queries[i][2], HEX);
    Serial.print(": ");

    Serial1.write(queries[i], 5);
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
}

// Test connection by sending commands and checking responses
void testConnection() {
  Serial.println("\n--- Testing Unit-ASR Connection ---");

  // Test 1: Send wake-up command
  Serial.println("\nTest 1: Sending wake-up command...");
  sendWakeup();
  delay(1000);

  if (Serial1.available() > 0) {
    Serial.print("✓ Received ");
    Serial.print(Serial1.available());
    Serial.println(" bytes");

    while(Serial1.available() > 0) {
      uint8_t b = Serial1.read();
      Serial.print("  0x");
      if(b < 16) Serial.print("0");
      Serial.print(b, HEX);
      Serial.print(" ");
    }
    Serial.println();
  } else {
    Serial.println("✗ No response - check RX connection (Unit-ASR TX → ESP32 GPIO44)");
  }

  // Test 2: Send query command
  Serial.println("\nTest 2: Sending query command...");
  sendQuery();
  delay(1000);

  if (Serial1.available() > 0) {
    Serial.print("✓ Received ");
    Serial.print(Serial1.available());
    Serial.println(" bytes");

    while(Serial1.available() > 0) {
      uint8_t b = Serial1.read();
      Serial.print("  0x");
      if(b < 16) Serial.print("0");
      Serial.print(b, HEX);
      Serial.print(" ");
    }
    Serial.println();
  } else {
    Serial.println("✗ No response");
  }

  // Test 3: Send another wake-up
  Serial.println("\nTest 3: Sending wake-up again...");
  sendWakeup();
  delay(1000);

  if (Serial1.available() > 0) {
    Serial.print("✓ Received ");
    Serial.print(Serial1.available());
    Serial.println(" bytes");

    while(Serial1.available() > 0) {
      uint8_t b = Serial1.read();
      Serial.print("  0x");
      if(b < 16) Serial.print("0");
      Serial.print(b, HEX);
      Serial.print(" ");
    }
    Serial.println();
  } else {
    Serial.println("✗ No response");
  }

  Serial.println("\n--- Connection Test Complete ---\n");

  // Query for firmware info
  Serial.println("--- Querying Firmware Info ---\n");
  queryFirmware();
  Serial.println("\n--- Firmware Query Complete ---\n");

  Serial.println("Diagnosis:");
  Serial.println("  If TX works but no RX: Swap TX/RX wires");
  Serial.println("  If no response at all: Check power, GND, baud rate");
  Serial.println("  Unit-ASR might only respond to voice, not UART commands");
  Serial.println();
}

void setup(void) {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n=== ESP32-S3 Unit-ASR Test ===");
  Serial.println("Initializing UART...");

  // Init UART for Unit-ASR
  Serial1.begin(115200, SERIAL_8N1, ASR_RX_PIN, ASR_TX_PIN);

  Serial.println("Unit-ASR UART initialized");
  Serial.println("GPIO43 (TX) / GPIO44 (RX)");

  // Test the connection
  testConnection();

  Serial.println("Waiting for voice commands...");
  Serial.println("Say 'Hi M Five' to test");
  Serial.println("Type 'w' to send wake-up, 'q' to query");
  Serial.println("===========================\n");
}

void loop() {
  // Check for keyboard commands from Serial Monitor
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'w' || cmd == 'W') {
      sendWakeup();
    } else if (cmd == 'q' || cmd == 'Q') {
      sendQuery();
    } else if (cmd == 'f' || cmd == 'F') {
      Serial.println("\n--- Manual Firmware Query ---");
      queryFirmware();
      Serial.println("--- Query Complete ---\n");
    }
  }

  // Check if data received from Unit-ASR
  if (Serial1.available() > 0) {
    Serial.print("Received ");
    Serial.print(Serial1.available());
    Serial.println(" bytes from Unit-ASR");
  }

  if (Serial1.available() >= 5) {
    uint8_t data[5];
    Serial1.readBytes(data, 5);

    // Print raw bytes for debugging
    Serial.print("Raw: ");
    for(int i = 0; i < 5; i++) {
      Serial.print("0x");
      if(data[i] < 16) Serial.print("0");
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Check packet format: AA 55 [CMD] 55 AA
    if (data[0] == 0xAA && data[1] == 0x55 &&
        data[3] == 0x55 && data[4] == 0xAA) {

      uint8_t commandID = data[2];

      Serial.println("┌─────────────────────────────┐");
      Serial.print  ("│ VOICE COMMAND: 0x");
      if(commandID < 16) Serial.print("0");
      Serial.print(commandID, HEX);
      Serial.print(" (");
      Serial.print(commandID);
      Serial.println(")  │");

      if (commandID == 0xFF) {
        Serial.println("│ Wake word: 'Hi M Five'      │");
      } else {
        // Example command mapping
        switch(commandID) {
          case 0x14:
            Serial.println("│ Command: Turn On            │");
            break;
          case 0x15:
            Serial.println("│ Command: Turn Off           │");
            break;
          default:
            Serial.print  ("│ Unknown command             │");
            Serial.println();
            break;
        }
      }
      Serial.println("└─────────────────────────────┘\n");
    } else {
      Serial.println("(Invalid packet format)\n");
    }
  }

  delay(10);
}
