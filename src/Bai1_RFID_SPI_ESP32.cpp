// Khi quẹt thẻ thì LED sẽ báo hiệu thành công và hiển thị tên người dùng.

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN   5
#define RST_PIN  22
#define LED_PIN  17

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {

  Serial.begin(115200);

  SPI.begin(18, 19, 23, SS_PIN); 
  // SCK = 18
  // MISO = 19
  // MOSI = 23
  // SS = 5
  pinMode(LED_PIN, OUTPUT);
  // Khởi tạo RC522
  mfrc522.PCD_Init();

  Serial.println("Quet the RFID...");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
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

    digitalWrite(LED_PIN, LOW);
    delay(2000);
  }

  mfrc522.PICC_HaltA();
}
