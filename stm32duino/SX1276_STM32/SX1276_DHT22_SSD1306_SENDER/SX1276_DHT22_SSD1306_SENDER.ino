#include <SPI.h>
#include <LoRa.h>
#include "DHT.h"
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

#define DHTPIN PA9
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
int count = 0;
byte id = 0xC0;

long lastSendTime = 0;
bool mode = true; //true : 도씨

void setup() {
  pinMode(NSS_PIN, OUTPUT);
  digitalWrite(NSS_PIN, HIGH);
  
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  
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
  dht.begin();
  display.display(); // 빈 화면 표시
  delay(2000);  // 2초 대기
  display.clearDisplay(); // 디스플레이 지우기
}

void loop() {

  
  float h = dht.readHumidity();
  float t = dht.readTemperature();


  byte a = (int)(h*100) >> 8;
  byte b = (int)(h*100) & 0b11111111;
  byte c = (int)(t*100) >> 8;
  byte d = (int)(t*100) & 0b11111111;
 

  LoRa.beginPacket();
  
  LoRa.write(a);
  LoRa.write(b);
  LoRa.write(c);
  LoRa.write(d);
  LoRa.write(id);

  LoRa.endPacket(true);
  count++;
  

  display.clearDisplay(); 
  display.setTextSize(1);      
  display.setTextColor(SSD1306_WHITE); 
  display.setCursor(38,0);     

  display.println("ID : 192");

  display.setTextSize(1);      
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print("TEMP : ");
  display.print(t);
  display.println(" C");
  display.print("HUMD : ");
  display.print(h);
  display.println(" %");
  display.print("COUNT : ");
  display.println(count);


    display.display(); // 디스플레이 업데이트
    delay(10000);
  }

