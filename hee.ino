// #include <ModbusMaster.h>
// #include "BluetoothSerial.h"

// // RS485 Control
// #define RE_DE 23
// #define RX_PIN 16
// #define TX_PIN 17

// BluetoothSerial SerialBT;
// ModbusMaster node;

// void preTransmission() {
//   digitalWrite(RE_DE, HIGH);
//   delay(10);
// }

// void postTransmission() {
//   digitalWrite(RE_DE, LOW);
//   delay(10);
// }

// void setup() {
//   Serial.begin(115200);
//   Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
//   pinMode(RE_DE, OUTPUT);
//   digitalWrite(RE_DE, LOW);

//   SerialBT.begin("ESP32_BT");  // 🔹 เปิด Bluetooth

//   node.begin(1, Serial2);
//   node.preTransmission(preTransmission);
//   node.postTransmission(postTransmission);

//   Serial.println("Modbus RS485 + Bluetooth Monitor");
//   SerialBT.println("Bluetooth ready! Waiting for data...");
// }

// void loop() {
//   uint8_t result;
//   uint16_t data;

//   result = node.readHoldingRegisters(0x0000, 1);

//   if (result == node.ku8MBSuccess) {
//     data = node.getResponseBuffer(0);
//     float waterLevel = data / 10.0;

//     String msg = "Water Level: " + String(waterLevel, 2) + " cm";
//     Serial.println(msg);
//     SerialBT.println(msg);  // 🔹 ส่งไปมือถือทาง Bluetooth
//   } else {
//     String err = "Modbus Error Code: " + String(result);
//     Serial.println(err);
//     SerialBT.println(err);  // 🔹 แจ้งผ่าน BT ด้วย
//   }

//   delay(1000);
// }
#include <ModbusMaster.h>
#include "BluetoothSerial.h"
#include "esp_sleep.h"

// RS485 Control
#define RE_DE 23
#define RX_PIN 16
#define TX_PIN 17

// GPIO0 (Reset pin) for external wakeup
#define WAKEUP_PIN GPIO_NUM_0

BluetoothSerial SerialBT;
ModbusMaster node;

unsigned long startTime;
const unsigned long ACTIVE_DURATION = 60000UL;  // 1 minute

void preTransmission() {
  digitalWrite(RE_DE, HIGH);
  delay(10);
}

void postTransmission() {
  digitalWrite(RE_DE, LOW);
  delay(10);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  pinMode(RE_DE, OUTPUT);
  digitalWrite(RE_DE, LOW);

  SerialBT.begin("ESP32_BT");
  Serial.println("📡 Modbus RS485 + Bluetooth Started");
  SerialBT.println("📡 Bluetooth Ready!");

  node.begin(1, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  // ตรวจสอบว่าตื่นจากสาเหตุใด
  esp_sleep_wakeup_cause_t wakeCause = esp_sleep_get_wakeup_cause();
  if (wakeCause == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("🔔 Woke up from GPIO0 (button press)");
  } else if (wakeCause == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("⏰ Woke up from timer");
  } else {
    Serial.println("🔋 wake up");
  }

  startTime = millis();
}

void loop() {
  // วนลูปรับค่าจาก RS485 ทุกๆ 1 วินาที
  while (millis() - startTime < ACTIVE_DURATION) {
    uint8_t result = node.readHoldingRegisters(0x0000, 1);

    if (result == node.ku8MBSuccess) {
      uint16_t data = node.getResponseBuffer(0);
      float waterLevel = data / 10.0;

      String msg = "Water Level: " + String(waterLevel, 2) + " cm";
      Serial.println(msg);
      SerialBT.println(msg);
    } 
    // else {
    //   String err = "Modbus Error Code: " + String(result);
    //   Serial.println(err);
    //   SerialBT.println(err);
    // }
    delay(1000);
  }

  // ครบ 1 นาที → เตรียมเข้าสู่ deep sleep
  Serial.println("💤 Sleep after 1 minute...");
  SerialBT.println("💤 Sleep now...");

  // ตั้งเวลาปลุกใหม่ และให้ GPIO0 สามารถปลุกได้
  esp_sleep_enable_timer_wakeup(60000000ULL);  // 1 นาที
  esp_sleep_enable_ext0_wakeup(WAKEUP_PIN, 0); // LOW = ปลุก

  Serial.flush();
  SerialBT.flush();
  delay(200);

  esp_deep_sleep_start();  // เข้าสู่ deep sleep → ตื่นแล้วเริ่มใหม่ที่ setup()
}

