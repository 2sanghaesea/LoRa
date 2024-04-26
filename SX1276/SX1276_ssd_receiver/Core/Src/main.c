/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

int master;     // Master/Slave mode flag
int ret;

char buffer[512];

int message_length;

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
  /* USER CODE BEGIN 2 */

	  printf("Mode: Slave\r\n");      // Print mode information
	  HAL_GPIO_WritePin(MODE_GPIO_Port, MODE_Pin, GPIO_PIN_SET);    // Set MODE pin high for slave mode


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



	  ret = SX1276_LoRaEntryRx(&SX1276, 16, 2000);     // Configure LoRa module for reception in slave mode
	  printf("Receive Mode !\r\n");


      SSD1306_Init();


	  char received_temperature[15];
	  char received_humidity[15];
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

		 printf("Slave ...\r\n");
		 HAL_Delay(800);
		 printf("Receiving package...\r\n");

		 ret = SX1276_LoRaRxPacket(&SX1276);
		 printf("Received: %d\r\n", ret);
		 if (ret > 0) {
			SX1276_read(&SX1276, (uint8_t*) buffer, ret);
			printf("Content (%d): %s\r\n", ret, buffer);



			char *token = strtok(buffer, ",");


			// 온도와 습도 추출
			strcpy(received_temperature, token);

			strcpy(received_humidity, strtok(NULL, "."));

			// OLED에 수신된 온도와 습도 표시
			SSD1306_GotoXY(21,0);
			SSD1306_Puts("RECEIVED",&Font_11x18, 1);
			SSD1306_GotoXY(10,20);
			SSD1306_Puts(received_temperature,&Font_11x18, 1);
			SSD1306_GotoXY(0,40);

			SSD1306_Puts(received_humidity,&Font_11x18, 1);
			SSD1306_UpdateScreen();

			printf("Package received ...\r\n");




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
