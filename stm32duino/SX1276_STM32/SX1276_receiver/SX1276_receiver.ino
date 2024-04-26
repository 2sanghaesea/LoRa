#include <SPI.h>
#include <LoRa.h>

#define NSS_PIN PA4
#define RESET_PIN PB0
#define DIO0_PIN PA1

char rec[8];

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
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    int i = 0;
    while (LoRa.available()) {
      rec[i] = (char)LoRa.read();
      i++;
    }
    rec[i] = '\0'; // 문자열 끝을 나타내는 NULL 문자 추가
    
    Serial.print("RSSI ");
    Serial.println(LoRa.packetRssi());
    Serial.println(rec); // 수신한 데이터를 시리얼 모니터에 출력
  }
}
