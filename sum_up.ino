//===== Library =====
#include <WiFi.h>
#include <HTTPClient.h>
#include <ModbusMaster.h>

//===== Define Pins =====
#define MAX485_RE_DE 23
#define RXD2         16
#define TXD2         17

//===== Modbus Config =====
#define SLAVE_ID     0x0C
ModbusMaster node;

//===== WiFi + Google Apps Script =====
const char* ssid = "TrueGigatexFiber_2.4G_CYD";
const char* password = "KhongYim";
const char* scriptURL = "https://script.google.com/macros/s/AKfycbyk-yE-SKbFYiVltWkbTjicBgEnaMIEGmYjyJPyo_RTzfRcJW2007y-Bi55mkVLtnA/exec"; 
//===== MAX485 Control =====
void preTransmission() {
  digitalWrite(MAX485_RE_DE, HIGH);
  delay(2);
}

void postTransmission() {
  delay(2);
  digitalWrite(MAX485_RE_DE, LOW);
}

//===== ฟังก์ชันส่งข้อมูลไป Google Sheet =====
void SendData(float waterlevel) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(scriptURL);
    http.addHeader("Content-Type", "application/json");

    String id = "esp001";  // หรือใช้ getChipID() เพื่อให้เป็น unique
    String jsonData = "{\"id\":\"" + id + "\",\"waterlevel\":" + String(waterlevel, 1) + "}";
    // String jsonData = "{\"waterlevel\":" + String(waterlevel, 1) + "}";
    Serial.println("JSON Data: " + jsonData);

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server Response: " + response);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      // Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

//===== ฟังก์ชัน Scan WiFi (Debug) =====
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

//===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  scanWiFi();

  Serial.print("🔌 Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi connected!");

  // RS485 & Modbus Init
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  pinMode(MAX485_RE_DE, OUTPUT);
  digitalWrite(MAX485_RE_DE, LOW);

  node.begin(SLAVE_ID, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  // อ่าน PID/VID
  uint8_t result = node.readHoldingRegisters(0x00, 2);
  if (result == node.ku8MBSuccess) {
    //Serial.print(F("PID: 0x")); Serial.println(node.getResponseBuffer(0), HEX);
    //Serial.print(F("VID: 0x")); Serial.println(node.getResponseBuffer(1), HEX);
  } else {
    Serial.print(F("PID/VID read failed, code=")); Serial.println(result);
  }

  // ตั้งค่า Trigger Mode
  uint16_t ctrl = (1 << 2); // Trigger mode only
  ctrl &= ~(1 << 0); // ปิด internal waterlevel
  ctrl &= ~(1 << 1); // ปิด waterlevel compensation
  result = node.writeSingleRegister(0x08, ctrl);
  if (result == node.ku8MBSuccess) {
    Serial.println(F("Control register set OK"));
  } else {
    Serial.print(F("Control register write failed, code=")); Serial.println(result);
  }
}

//===== Loop =====
void loop() {
  static unsigned long lastRun = 0;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    WiFi.reconnect();
    delay(1000);
    return;
  }

  // อ่านค่าทุก 5 วินาที
  if (millis() - lastRun >= 10000) {
    lastRun = millis();

    // Trigger วัดระยะ
    uint16_t ctrl = (1 << 2) | (1 << 3);
    node.writeSingleRegister(0x08, ctrl);
    delay(1000);

    uint8_t result = node.readHoldingRegisters(0x05, 1);
    if (result == node.ku8MBSuccess) {
      float distance_cm = node.getResponseBuffer(0) / 100.0;
      float waterlevel = 100.0 - distance_cm;
      waterlevel = round(waterlevel * 10.0) / 10.0;
      Serial.print(F("📏 Water Level: ")); Serial.print(waterlevel, 1); Serial.println(F(" cm"));
      SendData(waterlevel); // ส่งค่าระดับน้ำขึ้น Google Sheet
    } else {
      Serial.print(F("❌ Read failed, code=")); Serial.println(result);
    }
  }
}
