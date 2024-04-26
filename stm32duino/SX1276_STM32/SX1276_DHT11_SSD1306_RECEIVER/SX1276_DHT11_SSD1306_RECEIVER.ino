#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NSS_PIN PA4
#define RESET_PIN PB0
#define DIO0_PIN PA1

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
  

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.display(); // 빈 화면 표시
  delay(2000);  // 2초 대기
  display.clearDisplay(); // 디스플레이 지우기
}

void loop() {
  int packetSize = LoRa.parsePacket();
  
  if (packetSize > 0) {
    byte a = LoRa.read();
    byte b = LoRa.read();
    byte c = LoRa.read();
    byte d = LoRa.read();
    
    float h = (a<<8|b)/100.0;
    float t = (c<<8|d)/100.0;

    Serial.print("RSSI ");
    Serial.println(LoRa.packetRssi());
    Serial.print("습도 :");
    Serial.print(h);
    Serial.print(", 온도=");
    Serial.println(t);

    display.clearDisplay(); // 디스플레이 지우기
    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // 흰색 텍스트
    display.setCursor(38,0);     // Start at top-left corner

    display.println("RECEIVED");

    display.setTextSize(1);      // 2X 스케일
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print("HUMD: ");
    display.println(h);
    display.print("TEMP : ");
    display.println(t);
    display.print("RSSI : ");
    display.println(LoRa.packetRssi());


    display.display(); // 디스플레이 업데이트
  }
}
