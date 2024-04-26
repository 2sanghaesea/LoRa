/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    subghz_phy_app.h
  * @author  MCD Application Team
  * @brief   Header of application of the SubGHz_Phy Middleware
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SUBGHZ_PHY_APP_H__
#define __SUBGHZ_PHY_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define NODE_ID						  (uint8_t)(0x07)
#define GATEWAY_ID					  (uint8_t)(0xFF)
#define NODE_START					  (uint8_t)(0xB0)
#define NODE_END				      (uint8_t)(0xC0)
#define GATEWAY_START				  (uint8_t)(0xB1)
#define GATEWAY_END					  (uint8_t)(0xC1)
#define RELAY_ID					  (uint8_t)(0xEE)
#define RELAY_START					  (uint8_t)(0xB2)
#define RELAY_END					  (uint8_t)(0xC2)
#define ACK_OK						  (uint8_t)(0xDD)
#define ACK_NO						  (uint8_t)(0xDE)

#define RX_TIMEOUT_VALUE                            7000
#define TX_TIMEOUT_VALUE                            3000
#define BUFFER_SIZE                                 64  /* Define the payload size here */
#define LED_PERIOD_MS                               100
#define LED_ERROR_PERIOD_MS                         500
#define TX_PERIOD_MS                                10000  /* App TX duty cycle */
#define RELAY_PERIOD_MS								11000

#define WATCHDOG_RX_PERIOD						    60
#define SENSOR_READ_INTERVAL_MS						100


#define TCXO_WORKAROUND_TIME_MARGIN                 50  /* 50ms margin */

#define RF_CHANNEL_FREE_TRIALS_MAX                  5
#define RF_CHANNEL_FREE_RSSI_TRESHOLD               -80  /* [dBm] */
#define RSSI_SENSING_TIME                           10   /* [ms] */
#define CS_BACKOFF_TIME_UNIT                        20   /* [ms] */


/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* MODEM type: one shall be 1 the other shall be 0 */
#define USE_MODEM_LORA  1
#define USE_MODEM_FSK   0

#define RF_FREQUENCY                                923000000 /* Hz */

#ifndef TX_OUTPUT_POWER   /* please, to change this value, redefine it in USER CODE SECTION */
#define TX_OUTPUT_POWER                             14        /* dBm */
#endif /* TX_OUTPUT_POWER */

#if (( USE_MODEM_LORA == 1 ) && ( USE_MODEM_FSK == 0 ))
#define LORA_BANDWIDTH                              0         /* [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] */
#define LORA_SPREADING_FACTOR                       7         /* [SF7..SF12] */
#define LORA_CODINGRATE                             1         /* [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
#define LORA_PREAMBLE_LENGTH                        8         /* Same for Tx and Rx */
#define LORA_SYMBOL_TIMEOUT                         5         /* Symbols */
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif (( USE_MODEM_LORA == 0 ) && ( USE_MODEM_FSK == 1 ))

#define FSK_FDEV                                    25000     /* Hz */
#define FSK_DATARATE                                50000     /* bps */
#define FSK_BANDWIDTH                               50000     /* Hz */
#define FSK_PREAMBLE_LENGTH                         5         /* Same for Tx and Rx */
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
#error "Please define a modem in the compiler subghz_phy_app.h."
#endif /* USE_MODEM_LORA | USE_MODEM_FSK */

#define PAYLOAD_LEN                                 64

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros -----------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief  Init Subghz Application
  */
void SubghzApp_Init(void);

/* USER CODE BEGIN EFP */
void SHT31_LORA_Init(void);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*__SUBGHZ_PHY_APP_H__*/
