// #include <WiFi.h>

// // 🔧 ใส่ชื่อ WiFi และรหัสผ่านของคุณ
// const char* ssid = "TrueGigatexFiber_2.4G_CYD";
// const char* password = "KhongYim";

// // ตัวแปรสำหรับการเชื่อมต่อครั้งเดียว
// bool isConnected = false;

// // 🧠 ฟังก์ชันแสดงสถานะการเชื่อมต่อ WiFi
// void printWiFiStatus() {
//   wl_status_t status = WiFi.status();
//   switch (status) {
//     case WL_IDLE_STATUS:
//       Serial.println("WiFi: Idle status, not connected yet.");
//       break;
//     case WL_NO_SSID_AVAIL:
//       Serial.println("WiFi: SSID not available (network not found).");
//       break;
//     case WL_SCAN_COMPLETED:
//       Serial.println("WiFi: Scan completed.");
//       break;
//     case WL_CONNECTED:
//       Serial.println("WiFi: Connected!");
//       break;
//     case WL_CONNECT_FAILED:
//       Serial.println("WiFi: Connection failed. Possibly wrong password.");
//       break;
//     case WL_CONNECTION_LOST:
//       Serial.println("WiFi: Connection lost.");
//       break;
//     case WL_DISCONNECTED:
//       Serial.println("WiFi: Disconnected.");
//       break;
//     default:
//       Serial.println("WiFi: Unknown status.");
//       break;
//   }
// }

// // 📡 ฟังก์ชันสแกน WiFi รอบตัว
// void scanWiFi() {
//   Serial.println("🔍 Scanning for WiFi networks...");
//   int n = WiFi.scanNetworks();
//   if (n == 0) {
//     Serial.println("❌ No networks found.");
//   } else {
//     Serial.printf("✅ Found %d networks:\n", n);
//     for (int i = 0; i < n; ++i) {
//       Serial.printf("🔹 SSID: %s | Signal: %d dBm | %s\n",
//                     WiFi.SSID(i).c_str(), WiFi.RSSI(i),
//                     (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Secured");
//       delay(10);
//     }
//   }
// }

// void setup() {
//   Serial.begin(115200);
//   delay(1000);

//   // 🔍 ตรวจสอบว่าเจอ WiFi หรือเปล่า
//   scanWiFi();

//   // 🔌 เริ่มเชื่อมต่อ WiFi
//   Serial.print("🔌 Connecting to WiFi: ");
//   Serial.println(ssid);
//   WiFi.begin(ssid, password);

//   int retries = 0;
//   while (WiFi.status() != WL_CONNECTED && retries < 20) {
//     delay(1000);
//     printWiFiStatus();
//     retries++;
//   }

//   if (WiFi.status() == WL_CONNECTED) {
//     Serial.println("✅ WiFi connected!");
//     Serial.print("📡 IP address: ");
//     Serial.println(WiFi.localIP());

//     // แจ้งชื่อ WiFi ที่เชื่อมต่อ
//     if (!isConnected) {
//       Serial.print("🔗 Connected to WiFi: ");
//       Serial.println(WiFi.SSID());
//       isConnected = true; // ป้องกันการแสดงข้อความนี้อีก
//     }
//   } else {
//     Serial.println("❌ Failed to connect to WiFi.");
//   }

//   // 🔎 แสดง MAC Address
//   Serial.print("MAC Address: ");
//   Serial.println(WiFi.macAddress());

//   // 🧪 แสดงรายละเอียด config
//   WiFi.printDiag(Serial);
// }

// void loop() {
//   // ตรวจสถานะ WiFi ทุก 10 วินาที

//   static unsigned long lastCheck = 0;
//   if (millis() - lastCheck > 10000) {
//     lastCheck = millis();
//     printWiFiStatus();
//   }
// }


// Library 
#include <WiFi.h>
#include <HTTPClient.h>
// 🔧 ใส่ชื่อ WiFi และรหัสผ่านของคุณ
const char* ssid = "TrueGigatexFiber_2.4G_CYD";
const char* password = "KhongYim";
const char* scriptURL = "https://script.google.com/macros/s/AKfycbw_-ZW99Ubt86AGSFn8CSULq05_gV0_l7Qw8rYthknkQ9OJwT6t9HQNGTNyzgJTcN4/exec";


//function part
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

void SendData(float temperature, float humidity) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(scriptURL);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
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
  Serial.print("📡 IP address: ");
  Serial.println(WiFi.localIP());
  // SendData(25.2,30.0);
}
//Loop part
void loop() {
   static unsigned long lastSend = 0;

  if (millis() - lastSend > 10000) {
    lastSend = millis();
    SendData(18.5, 9.0);  // หรืออ่านจากเซ็นเซอร์จริงก็ได้
  }

}


