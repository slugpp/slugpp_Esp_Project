// #include <ModbusMaster.h>

// // ------------------- Pin Configuration -------------------
// #define MAX485_RE_DE 23   // MAX485 RE/DE control pin
// #define RXD2         16   // UART2 RX (connect to RO of MAX485)
// #define TXD2         17   // UART2 TX (connect to DI of MAX485)

// // ------------------- Modbus & Sensor Setup -------------------
// #define SLAVE_ID     0x0C // URM14 default Modbus address

// ModbusMaster node;

// // ------------------- RE/DE Control -------------------
// void preTransmission() {
//   digitalWrite(MAX485_RE_DE, HIGH);
//   delay(2);           // allow driver to settle
// }

// void postTransmission() {
//   delay(2);           // wait for transmission to complete
//   digitalWrite(MAX485_RE_DE, LOW);
// }

// // ------------------- Setup -------------------
// void setup() {
//   // 1) Serial monitors
//   Serial.begin(115200);
//   while (!Serial);

//   // 2) UART2 for RS‑485
//   Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

//   // 3) DE/RE pin
//   pinMode(MAX485_RE_DE, OUTPUT);
//   digitalWrite(MAX485_RE_DE, LOW);  // start in receive mode

//   // 4) ModbusMaster init
//   node.begin(SLAVE_ID, Serial2);
//   node.preTransmission(preTransmission);
//   node.postTransmission(postTransmission);

//   Serial.println(F("=== URM14 Modbus RTU Test Start ==="));
//   delay(500);

//   // 5) Initial communication test: Read PID & VID
//   uint8_t result = node.readHoldingRegisters(0x00, 2);
//   if (result == node.ku8MBSuccess) {
//     Serial.print(F("PID: 0x")); Serial.println(node.getResponseBuffer(0), HEX);
//     Serial.print(F("VID: 0x")); Serial.println(node.getResponseBuffer(1), HEX);
//   } else {
//     Serial.print(F("PID/VID read failed, code="));
//     Serial.println(result);
//   }

//   // 6) Configure control register (0x08):
//   //    bit2 = 1 → passive (trigger) mode
//   //    bit0 = 0 → internal temp comp
//   //    bit1 = 0 → enable temp comp
//   uint16_t ctrl = 0;
//   ctrl |= (1 << 2);
//   ctrl &= ~(1 << 0);
//   ctrl &= ~(1 << 1);

//   result = node.writeSingleRegister(0x08, ctrl);
//   if (result == node.ku8MBSuccess) {
//     Serial.println(F("Control register set OK"));
//   } else {
//     Serial.print(F("Control register write failed, code="));
//     Serial.println(result);
//   }
// }

// // ------------------- Loop -------------------
// void loop() {
//   uint8_t result;
//   // build control: trigger mode + trigger bit
//   uint16_t ctrl = (1 << 2) | (1 << 3);

//   // Serial.println(F("\nTriggering measurement..."));
//   result = node.writeSingleRegister(0x08, ctrl); //trigger and then write
//   // if (result == node.ku8MBSuccess) {
//   //   Serial.println(F("  → Trigger OK"));
//   // } else {
//   //   //Serial.print(F("  → Trigger failed, code="));
//   //   Serial.println(result);
//   //   delay(1000);
//   //   return;
//   // }

//   delay(300); // wait for measurement (min 30ms, safe 300ms)

//   //Serial.println(F("Reading distance..."));
//   result = node.readHoldingRegisters(0x05, 1);
//   if (result == node.ku8MBSuccess) {
//     float distance_cm = node.getResponseBuffer(0) / 100.0; // LSB=0.1mm
//     float water_distance = 100-distance_cm;
//     Serial.print(F("Water_Distance: "));
//     Serial.print(water_distance, 1);
//     Serial.println(F(" cm"));
//     Serial.print(F("  Distance: "));
//     Serial.print(distance_cm, 1);
//     Serial.println(F(" cm"));
//   } else {
//     Serial.print(F("  Read failed, code="));
//     Serial.println(result);
//   }

//   delay(1000);
// }

#include <ModbusMaster.h>
#include "BluetoothSerial.h"  // << เพิ่มตรงนี้

// ------------------- Pin Configuration -------------------
#define MAX485_RE_DE 23
#define RXD2         16
#define TXD2         17

// ------------------- Modbus & Sensor Setup -------------------
#define SLAVE_ID     0x0C

ModbusMaster node;
BluetoothSerial BTSerial;  // << ประกาศ Bluetooth Serial

// ------------------- RE/DE Control -------------------
void preTransmission() {
  digitalWrite(MAX485_RE_DE, HIGH);
  delay(2);
}

void postTransmission() {
  delay(2);
  digitalWrite(MAX485_RE_DE, LOW);
}

// ------------------- Setup -------------------
void setup() {
  Serial.begin(115200);
  BTSerial.begin("ESP32_URM14");  // << ตั้งชื่อ Bluetooth
  while (!Serial);

  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  pinMode(MAX485_RE_DE, OUTPUT);
  digitalWrite(MAX485_RE_DE, LOW);

  node.begin(SLAVE_ID, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  Serial.println(F("=== URM14 Modbus RTU Test Start ==="));
  BTSerial.println(F("=== URM14 Modbus RTU Test Start ==="));
  delay(500);

  uint8_t result = node.readHoldingRegisters(0x00, 2);
  if (result == node.ku8MBSuccess) {
    Serial.print(F("PID: 0x")); Serial.println(node.getResponseBuffer(0), HEX);
    Serial.print(F("VID: 0x")); Serial.println(node.getResponseBuffer(1), HEX);

    BTSerial.print(F("PID: 0x")); BTSerial.println(node.getResponseBuffer(0), HEX);
    BTSerial.print(F("VID: 0x")); BTSerial.println(node.getResponseBuffer(1), HEX);
  } else {
    Serial.print(F("PID/VID read failed, code=")); Serial.println(result);
    BTSerial.print(F("PID/VID read failed, code=")); BTSerial.println(result);
  }

  uint16_t ctrl = (1 << 2); // Trigger mode only
  ctrl &= ~(1 << 0); // internal temp
  ctrl &= ~(1 << 1); // enable temp comp

  result = node.writeSingleRegister(0x08, ctrl);
  if (result == node.ku8MBSuccess) {
    Serial.println(F("Control register set OK"));
    BTSerial.println(F("Control register set OK"));
  } else {
    Serial.print(F("Control register write failed, code=")); Serial.println(result);
    BTSerial.print(F("Control register write failed, code=")); BTSerial.println(result);
  }
}

// ------------------- Loop -------------------
void loop() {
  uint8_t result;
  uint16_t ctrl = (1 << 2) | (1 << 3);

  result = node.writeSingleRegister(0x08, ctrl);
  delay(300);

  result = node.readHoldingRegisters(0x05, 1);
  if (result == node.ku8MBSuccess) {
    float distance_cm = node.getResponseBuffer(0) / 100.0;
    float water_distance = 100 - distance_cm;

    Serial.print(F("Water_Distance: ")); Serial.print(water_distance, 1); Serial.println(F(" cm"));
    Serial.print(F("  Distance: ")); Serial.print(distance_cm, 1); Serial.println(F(" cm"));

    BTSerial.print(F("Water_Distance: ")); BTSerial.print(water_distance, 1); BTSerial.println(F(" cm"));
    BTSerial.print(F("  Distance: ")); BTSerial.print(distance_cm, 1); BTSerial.println(F(" cm"));
  } else {
    Serial.print(F("  Read failed, code=")); Serial.println(result);
    BTSerial.print(F("  Read failed, code=")); BTSerial.println(result);
  }

  delay(1000);
}

