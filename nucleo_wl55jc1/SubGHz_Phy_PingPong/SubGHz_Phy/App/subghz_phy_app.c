/* USER CODE BEGIN Header */
/*
 * TO DO LIST
 * RELAY == NODE ACK COUNT 같을때만 ..
 *
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_app.h"
#include "subghz_phy_app.h"
#include "radio.h"

/* USER CODE BEGIN Includes */
#include "stm32_timer.h"
#include "stm32_seq.h"
#include "utilities_def.h"
#include "app_version.h"
#include "subghz_phy_version.h"
#include <stdlib.h>
#include "sht31.h"
#include "i2c.h"
#include "main.h"
#include "board_resources.h"
#include "usart.h"
#include "CRC.h"
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */
/*NODETORELAY, NODETOGATEWAY 송신 버퍼가 다*/
#define NODE
#define NODETORELAY
//#define NODETOGATEWAY

//#define RELAY
//#define GATEWAY
/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
  RX_DONE,
  RX_TO,
  RX_ERR,
  TX_START,
  TX_DONE,
  RX_START,
  TX_TO,
} States_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Radio events function pointer */
static RadioEvents_t RadioEvents;

/* USER CODE BEGIN PV */
uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];
int8_t RssiValue = 0;
int8_t SnrValue = 0;

/* Led Timers objects*/
static  UTIL_TIMER_Object_t timerLedTx;
static  UTIL_TIMER_Object_t timerLedRx;
static  UTIL_TIMER_Object_t timerLedError;
/* RF collisions avoidance */
bool isChannelFree = true;

float humidity;
float temperature;


#if defined (NODE)
static  UTIL_TIMER_Object_t timerTx;
static	UTIL_TIMER_Object_t timerNode;
uint16_t node_ack_count = 0;
States_t State = TX_START;


#elif defined (RELAY)
static UTIL_TIMER_Object_t timerRelay;
States_t State = RX_START;
uint8_t RelayBuffer[BUFFER_SIZE];
uint16_t relay_ack_count = 0;
bool isCRCok = false;
static bool relayState = false;

#elif defined (GATEWAY)
static  UTIL_TIMER_Object_t timerGateway;
States_t State = RX_START;
int16_t receiveID;
bool isCRCok = false;
static uint32_t WatchDogRx = WATCHDOG_RX_PERIOD;
#endif

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/*!
 * @brief Function to be executed on Radio Tx Done event
 */
static void OnTxDone(void);

/**
  * @brief Function to be executed on Radio Rx Done event
  * @param  payload ptr of buffer received
  * @param  size buffer size
  * @param  rssi
  * @param  LoraSnr_FskCfo
  */
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);

/**
  * @brief Function executed on Radio Tx Timeout event
  */
static void OnTxTimeout(void);

/**
  * @brief Function executed on Radio Rx Timeout event
  */
static void OnRxTimeout(void);

/**
  * @brief Function executed on Radio Rx Error event
  */
static void OnRxError(void);

/* USER CODE BEGIN PFP */

static void SHT31_Process(void);
static void OnLedTxEvent(void *context);
static void OnLedRxEvent(void *context);
static void OnLedErrorEvent(void *context);
static uint32_t powInt(uint32_t base, uint32_t exp);

#if defined NODE
static void OnTimerTxEvent(void *context);
static void onTimerNodeEvent(void *context);
#elif defined RELAY
static void onTimerRelayEvent(void *context);
//static void RelaySendData(void);
#elif defined GATEWAY
static void onTimerGatewayEvent(void *context);
#endif

/* USER CODE END PFP */

/* Exported functions ---------------------------------------------------------*/
void SubghzApp_Init(void)
{
  /* USER CODE BEGIN SubghzApp_Init_1 */
   /* LED TX timer */
	UTIL_TIMER_Create(&timerLedTx,0xFFFFFFFF,UTIL_TIMER_ONESHOT,OnLedTxEvent,NULL);
	UTIL_TIMER_SetPeriod(&timerLedTx, LED_PERIOD_MS);

   /* LED ERROR timer */
	UTIL_TIMER_Create(&timerLedError,0xFFFFFFFF,UTIL_TIMER_ONESHOT,OnLedErrorEvent,NULL);
	UTIL_TIMER_SetPeriod(&timerLedError,LED_ERROR_PERIOD_MS);

   /* LED RX timer */
	UTIL_TIMER_Create(&timerLedRx,0xFFFFFFFF,UTIL_TIMER_ONESHOT,OnLedRxEvent,NULL);
	UTIL_TIMER_SetPeriod(&timerLedRx,LED_PERIOD_MS);

#if defined NODE
	UTIL_TIMER_Create(&timerTx,0xFFFFFFFF,UTIL_TIMER_ONESHOT,OnTimerTxEvent,NULL);
	UTIL_TIMER_SetPeriod(&timerTx,TX_PERIOD_MS);
	UTIL_TIMER_Create(&timerNode,0xFFFFFFFF,UTIL_TIMER_ONESHOT,onTimerNodeEvent,NULL);
	UTIL_TIMER_SetPeriod(&timerNode,10000);

#elif defined RELAY
	UTIL_TIMER_Create(&timerRelay,0xFFFFFFFF,UTIL_TIMER_ONESHOT,onTimerRelayEvent,NULL);
	UTIL_TIMER_SetPeriod(&timerRelay,RELAY_PERIOD_MS);

#elif defined GATEWAY
	UTIL_TIMER_Create(&timerGateway,0xFFFFFFFF,UTIL_TIMER_PERIODIC,onTimerGatewayEvent,NULL);
	UTIL_TIMER_SetPeriod(&timerGateway,1000);
	UTIL_TIMER_Start(&timerGateway);
#endif

  /* USER CODE END SubghzApp_Init_1 */

  /* Radio initialization */
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init(&RadioEvents);

  /* USER CODE BEGIN SubghzApp_Init_2 */
  srand(Radio.Random());

#if (( USE_MODEM_LORA == 1 ) && ( USE_MODEM_FSK == 0 ))
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  Radio.SetMaxPayloadLength(MODEM_LORA, BUFFER_SIZE);

#elif (( USE_MODEM_LORA == 0 ) && ( USE_MODEM_FSK == 1 ))

  Radio.SetTxConfig(MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                    FSK_DATARATE, 0,
                    FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, 0, TX_TIMEOUT_VALUE);

  Radio.SetRxConfig(MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                    0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                    0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                    0, 0, false, true);

  Radio.SetMaxPayloadLength(MODEM_FSK, BUFFER_SIZE);

#else
#error "Please define a frequency band in the sys_conf.h file."
#endif /* USE_MODEM_LORA | USE_MODEM_FSK */


  Radio.SetChannel(RF_FREQUENCY);

  SYS_LED_Init(SYS_LED_BLUE);
  SYS_LED_Init(SYS_LED_GREEN);
  SYS_LED_Init(SYS_LED_RED);

  Radio.Rx(RX_TIMEOUT_VALUE);

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_SHT31_Process), 0, SHT31_Process);

#ifdef NODE
  OnTimerTxEvent(NULL);
  APP_LOG(TS_OFF, VLEVEL_L, "\r\nSensor start -> ");
  APP_LOG(TS_OFF,VLEVEL_L," ID=%02X",NODE_ID);
#endif

#ifdef RELAY
  onTimerRelayEvent(NULL);
  APP_LOG(TS_OFF, VLEVEL_L, "\r\nRELAY Start -> ");
#endif

#ifdef GATEWAY
  APP_LOG(TS_OFF, VLEVEL_L, "\r\nBase Station start -> ");
#endif

  APP_LOG(TS_OFF, VLEVEL_L, " , RF=%uMHz , SF=%u",RF_FREQUENCY,LORA_SPREADING_FACTOR);
  APP_LOG(TS_OFF,VLEVEL_L," , CS=%ddBm",RF_CHANNEL_FREE_RSSI_TRESHOLD);
  APP_LOG(TS_OFF, VLEVEL_L, "\r\n");
  /* USER CODE END SubghzApp_Init_2 */
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private functions ---------------------------------------------------------*/
static void OnTxDone(void)
{
  /* USER CODE BEGIN OnTxDone */
#ifdef NODE
	 State = RX_START;
#endif
#if defined GATEWAY || defined RELAY
	 State = TX_DONE;
#endif
	 UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);
  /* USER CODE END OnTxDone */
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
  /* USER CODE BEGIN OnRxDone */
	BufferSize = size;
	memcpy(Buffer, payload, BufferSize);
	RssiValue = rssi;
	SnrValue = LoraSnr_FskCfo;
	State = RX_DONE;
	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);
  /* USER CODE END OnRxDone */
}

static void OnTxTimeout(void)
{
  /* USER CODE BEGIN OnTxTimeout */
  State = TX_TO;
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);
  /* USER CODE END OnTxTimeout */
}

static void OnRxTimeout(void)
{
  /* USER CODE BEGIN OnRxTimeout */
  State = RX_TO;
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);
  /* USER CODE END OnRxTimeout */
}

static void OnRxError(void)
{
  /* USER CODE BEGIN OnRxError */
  State = RX_ERR;
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);
  /* USER CODE END OnRxError */
}

/* USER CODE BEGIN PrFD */


static void OnLedTxEvent(void *context)
{
  SYS_LED_Off(SYS_LED_BLUE) ;
}

static void OnLedRxEvent(void *context)
{
  SYS_LED_Off(SYS_LED_GREEN) ;
}

static void OnLedErrorEvent(void *context)
{
  SYS_LED_Off(SYS_LED_RED) ;
}

static uint32_t powInt(uint32_t base, uint32_t exp)
{
    uint32_t result = 1;
    while(exp) { result *= base; exp--; }
    return result;
}



#if defined NODE
static void OnTimerTxEvent(void *context)
{
  State = TX_START;
  UTIL_TIMER_Start(&timerTx);
  UTIL_TIMER_Start(&timerLedTx);
  SYS_LED_On(SYS_LED_BLUE);
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);
}

static void onTimerNodeEvent(void *context){
	uint32_t i = 0;

	if (SHT31_GetData(SHT31_Periodic, SHT31_Medium, SHT31_NON_Stretch, SHT31_1) == SHT31_OK){
		temperature = SHT31_GetTemperature();
		humidity = SHT31_GetHumidity();
	}
	/*SEND BUFFER*/
	Buffer[i++] = NODE_START;
#ifdef NODETOGATEWAY
	Buffer[i++] = GATEWAY_ID;
#endif
#ifdef NODETORELAY
	Buffer[i++] = RELAY_ID;//RELAY_ID;//GATEWAY_ID;
#endif
	Buffer[i++] = NODE_ID;
	Buffer[i++] = (temperature < 0) ? 0x01 : 0x00;
	Buffer[i++] = (uint16_t)(temperature*100) >> 8;
	Buffer[i++] = (uint16_t)(temperature*100 ) & 0b11111111;
	Buffer[i++] = (uint16_t)(humidity*100) >> 8;
	Buffer[i++] = (uint16_t)(humidity*100) & 0b11111111;
	/*CRC-32*/
	uint32_t crc = CalcCRC(Buffer, i);
	Buffer[i++] = (crc >> 24) & 0xFF;
	Buffer[i++] = (crc >> 16) & 0xFF;
	Buffer[i++] = (crc >> 8) & 0xFF;
	Buffer[i++] = crc & 0xFF;
	Buffer[i++] = NODE_END;
	BufferSize = i;
}

#elif defined RELAY
static void onTimerRelayEvent(void *context){
	State = TX_START;
	relayState = true;
	UTIL_TIMER_Start(&timerRelay);
	UTIL_TIMER_Start(&timerLedTx);
	SYS_LED_On(SYS_LED_BLUE);
	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);

}


#elif defined GATEWAY
static void onTimerGatewayEvent(void *context){
	if (WatchDogRx>0)
		WatchDogRx--;
	else
		NVIC_SystemReset();
}
#endif

static void SHT31_Process(void){
	int16_t rssi;
	bool isChannelFree = true;
	uint32_t i,backoffTime,carrierSenseTime;
	switch (State){

	case TX_START:
#if defined NODE
		UTIL_TIMER_Start(&timerNode);

#elif defined RELAY
		if(Buffer[0] == NODE_START && Buffer[1] == RELAY_ID){
			i = 0;
			RelayBuffer[0] = RELAY_START;
			RelayBuffer[1] = GATEWAY_ID;
			RelayBuffer[2] = RELAY_ID;
			RelayBuffer[12] = RELAY_END;
			uint32_t crc = CalcCRC(RelayBuffer, 8);
			RelayBuffer[8] = (crc >> 24) & 0xFF;
			RelayBuffer[9] = (crc >> 16) & 0xFF;
			RelayBuffer[10] = (crc >> 8) & 0xFF;
			RelayBuffer[11] = crc & 0xFF;

			Buffer[i++] = RELAY_START;
			Buffer[i++] = NODE_ID;
			Buffer[i++] = RELAY_ID;
			if(isCRCok){
			  Buffer[i++] = ACK_OK;
			}
			else Buffer[i++] = ACK_NO;
			Buffer[i++] = RELAY_END;
			BufferSize = i;
		}


#elif defined GATEWAY
		i = 0;
		Buffer[i++] = GATEWAY_START;
		Buffer[i++] = receiveID;
		Buffer[i++] = GATEWAY_ID;
		if(isCRCok){
		  Buffer[i++] = ACK_OK;
		}
		else Buffer[i++] = ACK_NO;
		Buffer[i++] = GATEWAY_END;

		BufferSize = i;

#endif


		/*RF 충돌방지*/
		Radio.SetChannel(RF_FREQUENCY);
		Radio.Rx(0);
		HAL_Delay(Radio.GetWakeupTime());

		for (i=0;i<RF_CHANNEL_FREE_TRIALS_MAX;i++)
		{
			APP_LOG(TS_ON,VLEVEL_L,"RF Channel Sensing #%u ...",i+1);
			isChannelFree = true;
			carrierSenseTime = UTIL_TIMER_GetCurrentTime();

			while( UTIL_TIMER_GetElapsedTime(carrierSenseTime) < RSSI_SENSING_TIME)
			{
			  rssi = Radio.Rssi(MODEM_LORA);
			  if (rssi > RF_CHANNEL_FREE_RSSI_TRESHOLD) { isChannelFree = false; break; }
			}

			carrierSenseTime = UTIL_TIMER_GetElapsedTime(carrierSenseTime);
			APP_LOG(TS_OFF,VLEVEL_L," CS: %ddBm , CS time: %ums\r\n",rssi,carrierSenseTime);
			if (isChannelFree)
			{
			  break; //RF collisions avoidance loop
			}
			else
			{
			  if (i<RF_CHANNEL_FREE_TRIALS_MAX-1)
			  {
				backoffTime = CS_BACKOFF_TIME_UNIT * (1 + (rand() % powInt(2,i+1)));
				APP_LOG(TS_ON,VLEVEL_L,"RF channel is busy, next attempt after %ums...\r\n",backoffTime);
				HAL_Delay(backoffTime);
			  }
			}
	    }

		if (isChannelFree){
			Radio.SetChannel(RF_FREQUENCY);
			HAL_Delay(Radio.GetWakeupTime() + TCXO_WORKAROUND_TIME_MARGIN);
#ifdef NODE
			Radio.Send(Buffer,BufferSize);
			for(uint32_t k =0; k<BufferSize; k++) APP_LOG(TS_OFF,VLEVEL_L,"%02X|",Buffer[k]);
			APP_LOG(TS_OFF,VLEVEL_L,"\r\n");
#endif

#ifdef RELAY
			if(!relayState){
				Radio.Send(Buffer,BufferSize);
				isCRCok = false;
				APP_LOG(TS_ON,VLEVEL_L,"Relay SEND to NODE VALUE : ");
				for(uint32_t k =0; k<BufferSize; k++) APP_LOG(TS_OFF,VLEVEL_L,"%02X|",Buffer[k]);
				APP_LOG(TS_OFF,VLEVEL_L,"\r\n\r\n");
			}
			else{
				Radio.Send(RelayBuffer,13);
				APP_LOG(TS_ON,VLEVEL_L,"Relay SEND to Gateway VALUE : ");
				for(uint32_t k =0; k<13; k++) APP_LOG(TS_OFF,VLEVEL_L,"%02X|",RelayBuffer[k]);
				memset(RelayBuffer,0,13);
				APP_LOG(TS_OFF,VLEVEL_L,"\r\n\r\n");
				relayState = false;
			}


#endif

#ifdef GATEWAY
			if(isCRCok){
			Radio.Send(Buffer,BufferSize);
			isCRCok = false;
			receiveID =0;
			APP_LOG(TS_ON,VLEVEL_L,"Gateway SEND VALUE : ");
			for(uint32_t k =0; k<BufferSize; k++) APP_LOG(TS_OFF,VLEVEL_L,"%02X|",Buffer[k]);
			APP_LOG(TS_OFF,VLEVEL_L,"\r\n");
			}
#endif

		}
		else{
			UTIL_TIMER_Start(&timerLedError);
			SYS_LED_On(SYS_LED_RED);
			APP_LOG(TS_ON, VLEVEL_L, "RF channel: %uHz is BUSY\r\n\r\n",RF_FREQUENCY);
		}



		break;
	case TX_TO:
		UTIL_TIMER_Start(&timerLedError);
		SYS_LED_On(SYS_LED_RED);
		APP_LOG(TS_ON, VLEVEL_L, "TX timeout\r\n");

#if defined NODE
		Radio.Sleep();
#endif
#if defined GATEWAY || defined RELAY
		State = RX_START;
		UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);
#endif
		break;
	case TX_DONE:
#if defined GATEWAY || defined RELAY
	  UTIL_TIMER_Start(&timerLedTx);
	  SYS_LED_On(SYS_LED_BLUE);
	  State = RX_START;
	  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);
#endif
		break;
	case RX_START:
		Radio.SetChannel(RF_FREQUENCY);
		Radio.Rx(RX_TIMEOUT_VALUE);
		APP_LOG(TS_ON, VLEVEL_L, "RX...\r\n");
		break;
	case RX_DONE:
#if defined NODE
		APP_LOG(TS_ON,VLEVEL_L,"RECEIVE VALUE : ");
		for(uint16_t i=0;i<BufferSize;i++) APP_LOG(TS_OFF,VLEVEL_L,"%02X|",Buffer[i]);
		APP_LOG(TS_OFF,VLEVEL_L,"\r\n");
		if(Buffer[1] == NODE_ID && Buffer[3] == ACK_OK){
			node_ack_count++;
			Radio.Sleep();
			UTIL_TIMER_Start(&timerLedRx);
			SYS_LED_On(SYS_LED_GREEN);
			APP_LOG(TS_ON,VLEVEL_L, "RX : ");
			for(uint16_t i=0;i<BufferSize;i++) APP_LOG(TS_OFF,VLEVEL_L,"%02X|",Buffer[i]);
			APP_LOG(TS_OFF,VLEVEL_L,"\r\n");
			APP_LOG(TS_ON,VLEVEL_L,"NODE ACK COUNT : %d\r\n\r\n",node_ack_count);
		}

#elif defined RELAY
		if(Buffer[0] == NODE_START && Buffer[1] == RELAY_ID){
			uint32_t receiveCRC = ((uint32_t)Buffer[8] << 24) | ((uint32_t)Buffer[9] << 16) | ((uint32_t)Buffer[10] << 8) | (uint32_t)Buffer[11];
			uint32_t calculateCRC = CalcCRC(Buffer, 8);
			if(receiveCRC == calculateCRC){
				APP_LOG(TS_ON, VLEVEL_L ,"\r\nNODE %d RECEIVE : ",Buffer[2]);
				for(uint32_t k = 0; k<13; k++)APP_LOG(TS_OFF, VLEVEL_L, "%02X | ",Buffer[k]);
				APP_LOG(TS_OFF, VLEVEL_L, "\r\n");
				isCRCok = true;
				memcpy(RelayBuffer,Buffer,BufferSize);
				State = TX_START;
				UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);
			}

		}
		else if(Buffer[0] == GATEWAY_START && Buffer[1] == RELAY_ID && Buffer[3] == ACK_OK){
			relay_ack_count++;
			APP_LOG(TS_ON,VLEVEL_L,"RELAY ACK COUNT : %d\r\n\r\n",relay_ack_count);
			State = TX_START;
			UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);
		}

#elif defined GATEWAY
		if(Buffer[1] == GATEWAY_ID){
			APP_LOG(TS_ON,VLEVEL_L,"RECEIVE VALUE : ");
			for(uint16_t i=0;i<BufferSize;i++) APP_LOG(TS_OFF,VLEVEL_L,"%02X|",Buffer[i]);
			APP_LOG(TS_OFF,VLEVEL_L,"\r\n");

			uint32_t receiveCRC = ((uint32_t)Buffer[8] << 24) | ((uint32_t)Buffer[9] << 16) | ((uint32_t)Buffer[10] << 8) | (uint32_t)Buffer[11];
			uint32_t calculateCRC = CalcCRC(Buffer, 8);
			if(receiveCRC == calculateCRC){
				receiveID = Buffer[2];
				isCRCok = true;
				if(Buffer[0] == NODE_START && Buffer[12] == NODE_END){
					APP_LOG(TS_ON, VLEVEL_L ,"\r\nNODE %d RECEIVE : ",receiveID);
					for(uint32_t k = 0; k<13; k++)APP_LOG(TS_OFF, VLEVEL_L, "%02X | ",Buffer[k]);
					APP_LOG(TS_OFF, VLEVEL_L, "\r\n");
				}
				else if(Buffer[0] == RELAY_START && Buffer[12] == RELAY_END){
					APP_LOG(TS_ON, VLEVEL_L ,"\r\nRELAY %d RECEIVE : ",receiveID);
					for(uint32_t k = 0; k<13; k++)APP_LOG(TS_OFF, VLEVEL_L, "%02X | ",Buffer[k]);
					APP_LOG(TS_OFF, VLEVEL_L, "\r\n");
				}
				else{
					APP_LOG(TS_ON, VLEVEL_L, "WHAT THE FUCK! \r\n");
				}

				if(Buffer[3] == 0){
					APP_LOG(TS_OFF,VLEVEL_L,"\r\nTemperature : +");
				}
				else{
					APP_LOG(TS_OFF,VLEVEL_L,"\r\nTemperature : -");
				}
				uint16_t temp = (Buffer[4]<<8|Buffer[5])/100.0;
				uint16_t humi = (Buffer[6]<<8|Buffer[7])/100.0;
				APP_LOG(TS_OFF,VLEVEL_L,"%dC , Humidity : %d%% \r\n",temp,humi);
				APP_LOG(TS_OFF,VLEVEL_L,"\r\n");
			}
		}
		 State = TX_START;
		 UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);
#endif
		break;
	case RX_TO:
#ifdef NODE
		Radio.Sleep();
		APP_LOG(TS_ON, VLEVEL_L, "RX timeout\r\n\r\n");
#endif
#if defined GATEWAY || defined RELAY
		State = RX_START;
		UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);
#endif
		break;

	case RX_ERR:
#ifdef NODE
		Radio.Sleep();
#endif
		UTIL_TIMER_Start(&timerLedError);
		SYS_LED_On(SYS_LED_RED);
		APP_LOG(TS_ON, VLEVEL_L, "RX error\r\n");
#if defined GATEWAY || defined RELAY
		State = RX_START;
		UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SHT31_Process), CFG_SEQ_Prio_0);
#endif
		break;


	default:
		break;
	}
}

/* USER CODE END PrFD */
