#include <ModbusMaster.h>

#define MAX485_RE_DE 23    // Pin for controlling MAX485 RE/DE
#define SLAVE_ID 0x0C      // Default Modbus address of URM14

// UART2 pins
#define RXD2 16
#define TXD2 17

// Baud rates to test
uint32_t baudRates[] = {2400, 4800, 9600, 19200, 38400, 57600, 115200};
int numBaudRates = sizeof(baudRates) / sizeof(baudRates[0]);

ModbusMaster node;

void preTransmission() {
  digitalWrite(MAX485_RE_DE, HIGH);
  delay(2);
}

void postTransmission() {
  delay(2);
  digitalWrite(MAX485_RE_DE, LOW);
}

bool testBaudRate(uint32_t baudRate) {
  // Set the baud rate for Serial2
  Serial2.begin(baudRate, SERIAL_8N1, RXD2, TXD2);
  node.begin(SLAVE_ID, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  // Test communication: Read PID/VID
  uint8_t result = node.readHoldingRegisters(0x00, 2);
  if (result == node.ku8MBSuccess) {
    Serial.print("Baud rate "); Serial.print(baudRate); Serial.println(" - Communication successful!");
    return true;
  } else {
    Serial.print("Baud rate "); Serial.print(baudRate); Serial.print(" - Failed to communicate. Error: ");
    Serial.println(result);
    return false;
  }
}

void setup() {
  Serial.begin(115200);       // Serial monitor
  pinMode(MAX485_RE_DE, OUTPUT);
  digitalWrite(MAX485_RE_DE, LOW); // Start in receive mode

  Serial.println("Starting Baud Rate Test...");

  // Try each Baud rate until communication succeeds
  for (int i = 0; i < numBaudRates; i++) {
    Serial.print("Testing Baud rate: ");
    Serial.println(baudRates[i]);
    if (testBaudRate(baudRates[i])) {
      Serial.println("Baud rate successfully set!");
      break;
    }
  }
}

void loop() {
  // Main loop does nothing, we only run the baud rate test once in setup
}
