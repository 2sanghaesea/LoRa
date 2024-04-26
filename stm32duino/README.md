# SX1276_STM32duino
#### ※Using ArduinoIDE -> No FreeRTOS※ <br/>
Contact me if you want to get FreeRTOS ver. or MQTT ver project <br/>
* * *
###### 사전작업
* 추가 보드관리자 아래 링크 추가 <br/>
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json <br/>
* 보드매니저 STM32 MCU based boards 추가<br/>
* 라이브러리 다운
  * DHT11 (Dhruba Saha) 2.1.0 Ver
  * Adafruit_SSD1306.h
  * LoRa by Sandeep Mistry
* 사용 보드
  * Nucleo-F103RB 2대 (노드)
  * Nucleo-F303RE 2대 (노드)
  * Nucleo-F767ZI 1대 (게이트웨이)


* * * 
<div align="center">

|   sx1276   | Nucleo-F103RB | Nucleo-F303RE | Nucleo-F767ZI |
|:----------:|:--------------:|:--------------:|:--------------:|
|    VCC     |      3.3V      |      3.3V      |      3.3V      |
|    GND     |      GND       |      GND       |      GND       |
|    DIO0    |      PA1       |      PA1       |      PF3       |
|    SCK     |      PA5       |      PA5       |      PC10      |
|    MISO    |      PA6       |      PA6       |      PC11      |
|    MOSI    |      PA7       |      PA7       |      PC12      |
|    NSS     |      PA4       |      PA4       |      PF5       |
|    RST     |      PB0       |      PB0       |      PF3       |

</div>


* * *


![Untitled](https://github.com/2sanghaesea/SX1276_STM32/assets/116778214/7c81e0d7-5181-4339-ae57-d80fa637e8cf) <br/>




