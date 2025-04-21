//-----Library-----
#include <WiFi.h>
#include <HTTPClient.h>
#include <ModbusMaster.h>

//-----Define_pin-----
#define MAX485_RE_DE 23
#define RXD2         16
#define TXD2         17

//-----Modbus_setup-----
#define SLAVE_ID     0x0C
ModbusMaster node;

//------- wifi name and password and google script ----------
const char* ssid = "TrueGigatexFiber_2.4G_CYD";
const char* password = "KhongYim";
const char* scriptURL = "https://script.google.com/macros/s/AKfycbw_-ZW99Ubt86AGSFn8CSULq05_gV0_l7Qw8rYthknkQ9OJwT6t9HQNGTNyzgJTcN4/exec";

//------RE_DE_Control-----
void preTransmission() {
  digitalWrite(MAX485_RE_DE, HIGH);
  delay(2);
}

void postTransmission() {
  delay(2);
  digitalWrite(MAX485_RE_DE, LOW);
}

//-----Scan_Wifi_function-----
void scanWiFi() {
  Serial.println("🔍 Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("❌ No networks found.");
  } else {
    Serial.printf("✅ Found %d networks:\n", n);
    for (int i = 0; i < n; ++i) {
      Serial.printf("🔹 SSID: %s | Signal: %d dBm | %s\n",
                    WiFi.SSID(i).c_str(), WiFi.RSSI(i),
                    (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Secured");
      delay(10);
    }
  }
}

//------Send_Data_to_Google_Sheet_Function-----
void SendData(float Water_Height) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(scriptURL);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{\"Water_Height\":" + String(Water_Height) + "}";
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server Response: " + response);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

//setup part
void setup() {
  // เริ่มการสื่อสารผ่าน Serial
  Serial.begin(115200);
  delay(1000);
  scanWiFi();

  // 🔌 เริ่มเชื่อมต่อ WiFi
  Serial.print("🔌 Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // รอจนกว่า WiFi จะเชื่อมต่อสำเร็จ
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");  // แสดงว่าเชื่อมต่ออยู่
  }

  // เมื่อเชื่อมต่อสำเร็จ
  Serial.println();
  Serial.println("✅ WiFi connected!");

  // เริ่มการสื่อสารกับเซ็นเซอร์
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  pinMode(MAX485_RE_DE, OUTPUT);
  digitalWrite(MAX485_RE_DE, LOW);

  node.begin(SLAVE_ID, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  uint8_t result = node.readHoldingRegisters(0x00, 2);
  if (result == node.ku8MBSuccess) {
    Serial.print(F("PID: 0x")); Serial.println(node.getResponseBuffer(0), HEX);
    Serial.print(F("VID: 0x")); Serial.println(node.getResponseBuffer(1), HEX);
  } else {
    Serial.print(F("PID/VID read failed, code=")); Serial.println(result);
  }

  uint16_t ctrl = (1 << 2); // Trigger mode only
  ctrl &= ~(1 << 0); // internal temp
  ctrl &= ~(1 << 1); // enable temp comp

  result = node.writeSingleRegister(0x08, ctrl);
  if (result == node.ku8MBSuccess) {
    Serial.println(F("Control register set OK"));
  } else {
    Serial.print(F("Control register write failed, code=")); Serial.println(result);
  }
}

// Loop part
void loop() {
  // ตรวจสอบสถานะ WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    WiFi.reconnect();
  }

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

    // ส่งข้อมูลไปยัง Google Sheets ทุก 1 วินาที
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 1000) {
      lastSend = millis();
      SendData(water_distance);  // ส่งข้อมูลระยะห่างน้ำ
    }
  } else {
    Serial.print(F("  Read failed, code=")); Serial.println(result);
  }

  delay(1000);
}
