// #include <Arduino.h>
// #include <SPI.h>
// #include <MFRC522.h>

// #define SS_PIN 5
// #define RST_PIN 22

// MFRC522 mfrc522(SS_PIN, RST_PIN);

// void setup() {
//   Serial.begin(115200);
//   SPI.begin();
//   mfrc522.PCD_Init();
//   Serial.println("Quet the RFID...");
// }

// void loop() {
//   if (!mfrc522.PICC_IsNewCardPresent()) {
//     return;
//   }

//   if (!mfrc522.PICC_ReadCardSerial()) {
//     return;
//   }

//   String uidString = "";

//   Serial.print("UID tag: ");

//   for (byte i = 0; i < mfrc522.uid.size; i++) {
//     if (mfrc522.uid.uidByte[i] < 0x10) {
//       uidString += "0";
//     }
//     uidString += String(mfrc522.uid.uidByte[i], HEX);
//     Serial.print(mfrc522.uid.uidByte[i], HEX);
//     Serial.print(" ");
//   }

//   Serial.println();
//   uidString.toUpperCase();

//   // Thẻ trắng
//   if (uidString == "295B0307") {
//     Serial.println("Nguyen Van Quyen");
//     Digital.Write(17, LOW);
//   }

//   mfrc522.PICC_HaltA();
// }

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN   5
#define RST_PIN  22
#define LED_PIN  17

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {

  Serial.begin(115200);

  // ===== PHẦN SPI (PHÔ RA) =====
  SPI.begin(18, 19, 23, SS_PIN); 
  // SCK = 18
  // MISO = 19
  // MOSI = 23
  // SS = 5

  pinMode(LED_PIN, OUTPUT);
  

  // Khởi tạo RC522 (bên trong dùng SPI)
  mfrc522.PCD_Init();

  Serial.println("Quet the RFID...");
}

void loop() {

  // Mặc định LED tắt
  digitalWrite(LED_PIN, HIGH);
  // nếu LED active HIGH thì đổi thành LOW

  // Không có thẻ thì thoát
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  String uidString = "";

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uidString += "0";
    }
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }

  uidString.toUpperCase();

  if (uidString == "295B0307") {
    Serial.println("Nguyen Van Quyen");

    digitalWrite(LED_PIN, LOW);   // LED sáng (active LOW)
    delay(2000);
  }

  mfrc522.PICC_HaltA();
}