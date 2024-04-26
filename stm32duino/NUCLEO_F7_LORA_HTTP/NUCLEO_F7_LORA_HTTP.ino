#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LwIP.h>
#include <STM32Ethernet.h>
#include <ArduinoJson.h>

// FOR SX1276------------------------------
#define NSS_PIN PF5
#define RESET_PIN PF10
#define DIO0_PIN PF3
SPIClass SPI_3(PC12, PC11, PC10);
//----------------------------------------

// FOR LWIP--------------------------------
IPAddress ip(192, 168, 0, 155);
IPAddress serverIP(192, 168, 0, 48);
uint16_t serverPort = 5000;
EthernetClient client;
//-----------------------------------------
int count = 0;
bool ethernetConnected = false;

void setup() {
  pinMode(NSS_PIN, OUTPUT);
  digitalWrite(NSS_PIN, HIGH); // NSS 핀을 비활성화 상태로 설정
  
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH); // 모듈 리셋 핀 초기화
  
  LoRa.setSPI(SPI_3);
  LoRa.setPins(NSS_PIN, RESET_PIN, DIO0_PIN);
  LoRa.setTxPower(20);

  Serial.begin(9600);
  
  if (!LoRa.begin(923100000)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void sendEthernetData(float humidity, float temperature, byte id) {
  // LoRa.sleep();
  Serial.println("sending to server..");
 
  
  if (client.connect(serverIP, serverPort)) {
    Serial.println("client connect");
    StaticJsonDocument<200> doc;
    doc["id"] = id;
    doc["humidity"] = humidity;
    doc["temperature"] = temperature;
    
    char buffer[256];
    size_t n = serializeJson(doc, buffer);

    client.print("POST /receive_data HTTP/1.1\r\n");
    client.print("Host: 192.168.0.48\r\n"); 
    client.print("Content-Type: application/json\r\n");
    client.print("Content-Length: ");
    client.print(n);
    client.print("\r\n\r\n");
    client.print(buffer);

    client.stop();
  }
  else{
    ethernetConnected =false;
    Serial.println("ethernetConnected = false");
  }
}

void loop() {
  if (!ethernetConnected) {
    Ethernet.begin(ip);
    Serial.println("reconnected");
    ethernetConnected = true;
  }
 
  
  int packetSize = LoRa.parsePacket();

  if ( packetSize > 0 && packetSize ==5) { //packetSize > 0 
    byte a = LoRa.read();
    byte b = LoRa.read();
    byte c = LoRa.read();
    byte d = LoRa.read();
    byte id = LoRa.read();
    
    float h = (a<<8|b)/100.0;
    float t = (c<<8|d)/100.0;

    Serial.print("RSSI ");
    Serial.println(LoRa.packetRssi());
    Serial.print("ID =");
    Serial.print(id);
    Serial.print(", humidity :");
    Serial.print(h);
    Serial.print(", temperature=");
    Serial.print(t);
    Serial.print(" count = ");
    Serial.println(count++);
 

    sendEthernetData(h, t, id);
  }
}
