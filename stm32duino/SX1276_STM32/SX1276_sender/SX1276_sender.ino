#include <SPI.h>
#include <LoRa.h>

#define NSS_PIN PA4
#define RESET_PIN PB0
#define DIO0_PIN PA10

int count = 1;

void setup() {
  
  pinMode(NSS_PIN, OUTPUT);
  digitalWrite(NSS_PIN, HIGH); // NSS 핀을 비활성화 상태로 설정
  
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH); // 모듈 리셋 핀 초기화
  
  LoRa.setPins(NSS_PIN, RESET_PIN, DIO0_PIN);
  LoRa.setTxPower(20);
  
  Serial.begin(9600);
  
  if (!LoRa.begin(923100000)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  LoRa.setSyncWord(0x34);
}

void loop() {
    LoRa.beginPacket();
    LoRa.print(count);
    LoRa.endPacket();
    delay(500);
    count++; // count 값을 증가시킴
}