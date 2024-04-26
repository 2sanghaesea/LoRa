/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  * @brief			: 로라 엔드 노드 (transmitter)
  * @author			: C J R Y U | 2sanghaesea
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "fonts.h"
#include "ssd1306.h"
#include <SX1276.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int _write(int file, char *ptr, int len) {
    int i;
    HAL_UART_Transmit(&huart2, (uint8_t*) ptr, len, 50);
    for (i = 0; i < len; i++) {
        ITM_SendChar(*ptr++);
    }
    return len;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//sx1276 코드 ----------------------------------------------------------------
SX1276_hw_t SX1276_hw;     // SX1276 hardware configuration struct
SX1276_t SX1276;           // SX1276 LoRa module struct

int ret;

char buffer[512];

int message;
int message_length;


//DHT11 센서 코드 -------------------------------------------------------------
void delay_us(uint16_t time) {
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	while((__HAL_TIM_GET_COUNTER(&htim1))<time);
}

//----- 펄스 대기 함수
int wait_pulse(int state) {
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	while (HAL_GPIO_ReadPin(DHT_GPIO_Port, DHT_Pin) != state) {  // 설정한 상태로 변할 때까지 대기
		if(__HAL_TIM_GET_COUNTER(&htim1) >= 100) {              // 100us 안에 신호 들어오지 않으면 타임아웃으로 간주
			return 0;
		}
	}
	return 1;
}

// 온도, 습도 저장하는 전역변수 선언
int Temperature = 0;
int Humidity = 0;

int dht11_read (void) {
	//----- Start Signal 전송
	// 포트를 출력으로 설정
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = DHT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DHT_GPIO_Port, &GPIO_InitStruct);

	// Low 18ms, High 20us 펄스 생성
	HAL_GPIO_WritePin(DHT_GPIO_Port, DHT_Pin, 0);
	delay_us(18000);
    HAL_GPIO_WritePin(DHT_GPIO_Port, DHT_Pin, 1);
    delay_us(20);

	// 포트를 입력으로 설정
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(DHT_GPIO_Port, &GPIO_InitStruct);

	//----- DHT11 응답 확인
	delay_us(40);  // 40us 대기
	if(!(HAL_GPIO_ReadPin(DHT_GPIO_Port, DHT_Pin))) {   // DHT11 응답 체크(Low)
		delay_us(80);
		if(!(HAL_GPIO_ReadPin(DHT_GPIO_Port, DHT_Pin))) // 80us뒤 DHT11 High 응답 없으면 timeout으로 간주
			return -1;
	}
	if(wait_pulse(GPIO_PIN_RESET) == 0) // 데이터 전송 시작 대기
		return -1; // timeout

	//----- DHT11 데이터 읽기
	uint8_t out[5], i, j;
	for(i = 0; i < 5; i++) {               // 습도 정수자리, 습도 소수자리, 온도 정수자리, 온도 소수자리, 체크섬 순으로 읽음
		for(j = 0; j < 8; j++) {           // 하나의 데이터는 8비트로 구성되며, 최상위 비트부터 하나씩 읽기 시작함
			if(!wait_pulse(GPIO_PIN_SET))  // 데이터 전송 시작까지 대기
				return -1;

			delay_us(40);  // 40us 대기 후 High상태이면 1, Low상태이면 0 수신
			if(!(HAL_GPIO_ReadPin (DHT_GPIO_Port, DHT_Pin)))   // Low일 경우 0
				out[i] &= ~(1<<(7-j));
			else                                              // High일 경우 1
				out[i] |= (1<<(7-j));

			if(!wait_pulse(GPIO_PIN_RESET)) // 다음 데이터 전송 시작까지 대기
				return -1;
		}
	}

	//----- 체크섬 판별
	if(out[4] != (out[0] + out[1] + out[2] + out[3]))
		return -2;

	//----- 필요 데이터 분리
	Temperature = out[2];
	Humidity = out[0];

	return 1;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

	  printf("Mode: Master\r\n");     // Print mode information
	  HAL_GPIO_WritePin(MODE_GPIO_Port, MODE_Pin, GPIO_PIN_RESET);  // Set MODE pin low for master mode


      // Initialize SX1276 LoRa module
      SX1276_hw.dio0.port = DIO0_GPIO_Port;
      SX1276_hw.dio0.pin = DIO0_Pin;
      SX1276_hw.nss.port = NSS_GPIO_Port;
      SX1276_hw.nss.pin = NSS_Pin;
      SX1276_hw.reset.port = RESET_GPIO_Port;
      SX1276_hw.reset.pin = RESET_Pin;
      SX1276_hw.spi = &hspi1;

      SX1276.hw = &SX1276_hw;

      printf("Configuring LoRa module\r\n");
      SX1276_init(&SX1276, 923100000, SX1276_POWER_20DBM, SX1276_LORA_SF_7,  // Configure LoRa module
                  SX1276_LORA_BW_125KHZ, SX1276_LORA_CR_4_5, SX1276_LORA_CRC_EN, 10);
      printf("Done configuring LoRaModule\r\n");



	  ret = SX1276_LoRaEntryTx(&SX1276, 16, 2000);     // Configure LoRa module for transmission in master mode
	  printf("Transmit Mode !\r\n");

      SSD1306_Init();
      HAL_TIM_Base_Start(&htim1);
      char h_str[15];
      char t_str[15];
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

		 printf("Master ...\r\n");
		 HAL_Delay(1000);
		 printf("Connect DHT11...\r\n");

		 if(dht11_read() == 1){
			 message_length = sprintf(buffer, "Temp : %dC, Humd : %d%%.", Temperature, Humidity);
			 ret = SX1276_LoRaEntryTx(&SX1276, message_length, 2000);
			 printf("Entry: %d\r\n", ret);

			 printf("Sending %s\r\n", buffer);
			 ret = SX1276_LoRaTxPacket(&SX1276, (uint8_t*) buffer,
									   message_length, 2000);
			 printf("Transmission: %d\r\n", ret);
			 printf("Package sent...\r\n");

			 sprintf(t_str, "Temp : %d C", Temperature);
			 sprintf(h_str, "Humd : %d %%", Humidity);
			 SSD1306_GotoXY(20,0);
			 SSD1306_Puts("TRANSMIT",&Font_11x18, 1);
			 SSD1306_GotoXY(0,20);
			 SSD1306_Puts(t_str,&Font_11x18, 1);
			 SSD1306_GotoXY(0,40);
			 SSD1306_Puts(h_str,&Font_11x18, 1);
			 SSD1306_UpdateScreen();
			 HAL_Delay(100);
		 }




    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
   {
   }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
