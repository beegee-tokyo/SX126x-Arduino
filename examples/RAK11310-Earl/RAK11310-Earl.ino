/**
 * @file RAK11310-Earl.ino
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Simple example to use RAK11300/RAK11310 with Arduino-Pico BSP
 *
 * By default it is using LoRaWAN. Uncommenting
 * #define LORAWAN
 * enables LoRa P2P functionality
 *
 * @version 0.1
 * @date 2023-12-14
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <Arduino.h>
#include <SX126x-Arduino.h>
#include <LoRaWan-Arduino.h>
#include <task.h>
#include <map>

void task_list(void);

// Comment to switch to LoRa P2P
#define LORAWAN

// Function declarations
void OnTxDone(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxTimeout(void);
void OnRxTimeout(void);
void OnRxError(void);
void OnCadDone(bool cadResult);
static void lorawan_has_joined_handler(void);
static void lorawan_rx_handler(lmh_app_data_t *app_data);
static void lorawan_confirm_class_handler(DeviceClass_t Class);
static void lorawan_join_failed_handler(void);
static void lorawan_unconf_finished(void);
static void lorawan_conf_finished(bool result);

// Define LoRa parameters
#define RF_FREQUENCY 916100000	// Hz
#define TX_OUTPUT_POWER 22		// dBm
#define LORA_BANDWIDTH 0		// [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1		// [1: 4/5, 2: 4/6,  3: 4/7,  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8	// Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0	// Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 0
#define TX_TIMEOUT_VALUE 3000

#define BUFFER_SIZE 255 // Define the payload size here

static RadioEvents_t RadioEvents;
static uint16_t BufferSize = BUFFER_SIZE;
static uint8_t RcvBuffer[BUFFER_SIZE];
static uint8_t TxdBuffer[BUFFER_SIZE];

time_t cadTime;

// LoRaWAN settings
static uint8_t m_lora_app_data_buffer[255];									  ///< Lora user application data buffer.
static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0}; ///< Lora user application data structure.

#define JOINREQ_NBTRIALS 3 /**< Number of trials for the join request. */

/**@brief Structure containing LoRaWan parameters, needed for lmh_init()
 */
static lmh_param_t lora_param_init = {LORAWAN_ADR_OFF, DR_3, LORAWAN_PUBLIC_NETWORK, JOINREQ_NBTRIALS, LORAWAN_DEFAULT_TX_POWER, LORAWAN_DUTYCYCLE_OFF};

/**@brief Structure containing LoRaWan callback functions, needed for lmh_init()
 */
static lmh_callback_t lora_callbacks = {BoardGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
										lorawan_rx_handler, lorawan_has_joined_handler,
										lorawan_confirm_class_handler, lorawan_join_failed_handler,
										lorawan_unconf_finished, lorawan_conf_finished};

uint8_t nodeDeviceEUI[8] = {0xAF, 0x1F, 0x09, 0xFF, 0xFE, 0x06, 0x40, 0x80}; // AF1F09FFFE064080

uint8_t nodeAppEUI[8] = {0xAF, 0x1F, 0x09, 0xFF, 0xFE, 0x06, 0x40, 0x80}; // AF1F09FFFE064080

uint8_t nodeAppKey[16] = {0x2B, 0x84, 0xE0, 0xB0, 0x9B, 0x68, 0xE5, 0xCB, 0x42, 0x17, 0x6F, 0xE7, 0x53, 0xDC, 0xEE, 0x79}; // 2B84E0B09B68E5CB42176FE753DCEE79

bool network_joined = false;

static TimerEvent_t timer1_struct;
static TimerEvent_t timer2_struct;

SemaphoreHandle_t app1_sem;
SemaphoreHandle_t app2_sem;
static BaseType_t xHigherPriorityTaskWoken = pdTRUE;

void timer1_cb(void)
{
	xSemaphoreGiveFromISR(app1_sem, &xHigherPriorityTaskWoken);
}

void timer2_cb(void)
{
	xSemaphoreGiveFromISR(app2_sem, &xHigherPriorityTaskWoken);
}

void setup1()
{
	pinMode(LED_BLUE, OUTPUT);
	digitalWrite(LED_BLUE, LOW);

	// Create the app event semaphore
	app2_sem = xSemaphoreCreateBinary();
	// Initialize semaphore
	xSemaphoreGive(app2_sem);
	xSemaphoreTake(app2_sem, 10);
}

void loop1()
{
	if (xSemaphoreTake(app2_sem, portMAX_DELAY) == pdTRUE)
	{
		digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
		// task_list();
	}
}

void setup()
{
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);

	digitalWrite(LED_GREEN, HIGH);
	digitalWrite(LED_BLUE, HIGH);

	time_t serial_timeout = millis();
	Serial.begin(230400);
	// On nRF52840 the USB serial is not available immediately
	while (!Serial)
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
			digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
		}
		else
		{
			digitalWrite(LED_GREEN, LOW);
			break;
		}
	}
	digitalWrite(LED_GREEN, LOW);
#ifdef NRF52_SERIES
	if (lora_rak4630_init() != 0)
#else
	if (lora_rak11300_init() != 0)
#endif
	{
		Serial.println("Failed initialization of SX1262");
	}
	else
	{
#ifdef LORAWAN
		Serial.println("Setup LoRaWAN");
		// Setup the EUIs and Keys
		lmh_setDevEui(nodeDeviceEUI);
		lmh_setAppEui(nodeAppEUI);
		lmh_setAppKey(nodeAppKey);

		// Initialize LoRaWan
		uint32_t err_code = lmh_init(&lora_callbacks, lora_param_init, true, CLASS_A, LORAMAC_REGION_EU868);
		if (err_code != 0)
		{
			Serial.printf("lmh_init failed - %d\n", err_code);
		}

		// Start Join procedure
		Serial.println("Join LoRaWAN");
		lmh_join();
#else
		// Initialize the Radio callbacks
		RadioEvents.TxDone = OnTxDone;
		RadioEvents.RxDone = OnRxDone;
		RadioEvents.TxTimeout = OnTxTimeout;
		RadioEvents.RxTimeout = OnRxTimeout;
		RadioEvents.RxError = OnRxError;
		RadioEvents.CadDone = OnCadDone;

		// Initialize the Radio
		Radio.Init(&RadioEvents);

		// Set Radio channel
		Radio.SetChannel(RF_FREQUENCY);

		// Set Radio TX configuration
		Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
						  LORA_SPREADING_FACTOR, LORA_CODINGRATE,
						  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
						  true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

		// Set Radio RX configuration
		Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
						  LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
						  LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
						  0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

		// Start LoRa
		Serial.println("Starting Radio.Rx");
		Radio.Sleep();
		Radio.Rx(RX_TIMEOUT_VALUE);

		digitalWrite(LED_BLUE, LOW);
#endif

		// Create the app event semaphore
		app1_sem = xSemaphoreCreateBinary();
		// Initialize semaphore
		xSemaphoreGive(app1_sem);
		xSemaphoreTake(app1_sem, 10);

		timer1_struct.oneShot = false;
		timer1_struct.ReloadValue = 20000;
		TimerInit(&timer1_struct, timer1_cb);
		TimerStart(&timer1_struct);

		// Create timer for second core (Blue LED blinking)
		timer2_struct.oneShot = false;
		timer2_struct.ReloadValue = 50;
		timer2_struct.Callback = timer2_cb;
		TimerInit(&timer2_struct, timer2_cb);
		TimerStart(&timer2_struct);
	}
}

bool tx_active = false;

void loop()
{
	if (xSemaphoreTake(app1_sem, portMAX_DELAY) == pdTRUE)
	{
		task_list();
#ifdef LORAWAN
		if (lmh_join_status_get() != LMH_SET)
		{
			// Not joined, try again later
			Serial.println("Did not join network, skip sending frame");
			return;
		}

		if (tx_active)
		{
			Serial.println("Last TX still active, do not send yet");
			// error_flag = true;
			return;
		}

		digitalWrite(LED_GREEN, HIGH);
		uint32_t i = 0;
		m_lora_app_data.port = LORAWAN_APP_PORT;
		// 02683c036701d3237d0399288a0001298a00012a8a0001108a0179017401a7306601
		//
		m_lora_app_data.buffer[i++] = 0x02;
		m_lora_app_data.buffer[i++] = 0x68;
		m_lora_app_data.buffer[i++] = 0x3c;
		m_lora_app_data.buffer[i++] = 0x03;
		m_lora_app_data.buffer[i++] = 0x67;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0xd3;
		m_lora_app_data.buffer[i++] = 0x23;
		m_lora_app_data.buffer[i++] = 0x7d;
		m_lora_app_data.buffer[i++] = 0x03;
		m_lora_app_data.buffer[i++] = 0x99;
		m_lora_app_data.buffer[i++] = 0x28;
		m_lora_app_data.buffer[i++] = 0x8a;
		m_lora_app_data.buffer[i++] = 0x00;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0x29;
		m_lora_app_data.buffer[i++] = 0x8a;
		m_lora_app_data.buffer[i++] = 0x00;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0x2a;
		m_lora_app_data.buffer[i++] = 0x8a;
		m_lora_app_data.buffer[i++] = 0x00;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0x10;
		m_lora_app_data.buffer[i++] = 0x8a;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0x79;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0x74;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0xa7;
		m_lora_app_data.buffer[i++] = 0x30;
		m_lora_app_data.buffer[i++] = 0x66;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffsize = i;

		lmh_error_status error = lmh_send(&m_lora_app_data, LMH_CONFIRMED_MSG);
		if (error == LMH_SUCCESS)
		{
			tx_active = true;
		}
		else if (error == LMH_ERROR)
		{
			/// \todo Workaround. After missing ACK's the DR is reset
			// lmh_datarate_set(DR_3, false);
		}
		Serial.printf("lmh_send result %d\n", error);

#else
		uint32_t i = 0;
		m_lora_app_data.buffer[i++] = 0x02;
		m_lora_app_data.buffer[i++] = 0x68;
		m_lora_app_data.buffer[i++] = 0x3c;
		m_lora_app_data.buffer[i++] = 0x03;
		m_lora_app_data.buffer[i++] = 0x67;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0xd3;
		m_lora_app_data.buffer[i++] = 0x23;
		m_lora_app_data.buffer[i++] = 0x7d;
		m_lora_app_data.buffer[i++] = 0x03;
		m_lora_app_data.buffer[i++] = 0x99;
		m_lora_app_data.buffer[i++] = 0x28;
		m_lora_app_data.buffer[i++] = 0x8a;
		m_lora_app_data.buffer[i++] = 0x00;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0x29;
		m_lora_app_data.buffer[i++] = 0x8a;
		m_lora_app_data.buffer[i++] = 0x00;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0x2a;
		m_lora_app_data.buffer[i++] = 0x8a;
		m_lora_app_data.buffer[i++] = 0x00;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0x10;
		m_lora_app_data.buffer[i++] = 0x8a;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0x79;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0x74;
		m_lora_app_data.buffer[i++] = 0x01;
		m_lora_app_data.buffer[i++] = 0xa7;
		m_lora_app_data.buffer[i++] = 0x30;
		m_lora_app_data.buffer[i++] = 0x66;
		m_lora_app_data.buffer[i++] = 0x01;

		Radio.Standby();
		Radio.Send(m_lora_app_data.buffer, i);
#endif
	}
}

/**@brief Function to be executed on Radio Tx Done event
 */
void OnTxDone(void)
{
	Serial.println("OnTxDone");
	Radio.Standby();
	Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Done event
 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	Serial.println("OnRxDone");
	delay(10);
	BufferSize = size;
	memcpy(RcvBuffer, payload, BufferSize);

	Serial.printf("RssiValue=%d dBm, SnrValue=%d\n", rssi, snr);

	for (int idx = 0; idx < size; idx++)
	{
		Serial.printf("%02X ", RcvBuffer[idx]);
	}
	Serial.println("");

	digitalWrite(LED_GREEN, HIGH);
	digitalWrite(LED_BLUE, LOW);

	Radio.Standby();
	Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Tx Timeout event
 */
void OnTxTimeout(void)
{
	// Radio.Sleep();
	Serial.println("OnTxTimeout");
	digitalWrite(LED_GREEN, LOW);
	digitalWrite(LED_BLUE, HIGH);

	Radio.Standby();
	Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Timeout event
 */
void OnRxTimeout(void)
{
	Serial.println("OnRxTimeout");

	digitalWrite(LED_GREEN, LOW);
	digitalWrite(LED_BLUE, HIGH);

	Radio.Standby();
	Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Error event
 */
void OnRxError(void)
{
	Serial.println("OnRxError");
	digitalWrite(LED_GREEN, LOW);
	digitalWrite(LED_BLUE, HIGH);

	Radio.Standby();
	Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Error event
 */
void OnCadDone(bool cadResult)
{
	time_t duration = millis() - cadTime;
	if (cadResult)
	{
		Serial.printf("CAD returned channel busy after %ldms\n", duration);
		Radio.Standby();
		Radio.Rx(RX_TIMEOUT_VALUE);
	}
	else
	{
		Serial.printf("CAD returned channel free after %ldms\n", duration);
		Serial.println("Sending a PING in OnCadDone as Master");
		// Send the next PING frame
		TxdBuffer[0] = 'P';
		TxdBuffer[1] = 'I';
		TxdBuffer[2] = 'N';
		TxdBuffer[3] = 'G';
		// We fill the buffer with numbers for the payload
		for (int i = 4; i < BufferSize; i++)
		{
			TxdBuffer[i] = i - 4;
		}

		Radio.Standby();
		Radio.Send(TxdBuffer, BufferSize);
	}
}

/**@brief LoRa function for handling OTAA join failed
 */
static void lorawan_join_failed_handler(void)
{
	Serial.println("OVER_THE_AIR_ACTIVATION failed!");
	Serial.println("Check your EUI's and Keys's!");
	Serial.println("Check if a Gateway is in range!");
	lmh_join();
}

/**@brief LoRa function for handling HasJoined event.
 */
static void lorawan_has_joined_handler(void)
{
	Serial.println("Network Joined");
	network_joined = true;
	digitalWrite(LED_BLUE, LOW);
}

/**@brief Function for handling LoRaWan received data from Gateway
 *
 * @param[app_data] app_data  Pointer to rx data
 */
static void lorawan_rx_handler(lmh_app_data_t *app_data)
{
	digitalWrite(LED_GREEN, LOW);
	Serial.printf("LoRa Packet received on port %d, size:%d, rssi:%d, snr:%d\n",
				  app_data->port, app_data->buffsize, app_data->rssi, app_data->snr);

	switch (app_data->port)
	{
	case 3:
		// Port 3 switches the class
		if (app_data->buffsize == 1)
		{
			switch (app_data->buffer[0])
			{
			case 0:
				lmh_class_request(CLASS_A);
				break;

			case 1:
				lmh_class_request(CLASS_B);
				break;

			case 2:
				lmh_class_request(CLASS_C);
				break;

			default:
				break;
			}
		}
		break;

	case LORAWAN_APP_PORT:
		// YOUR_JOB: Take action on received data
		break;

	default:
		break;
	}
	tx_active = false;
}

static void lorawan_confirm_class_handler(DeviceClass_t Class)
{
	digitalWrite(LED_GREEN, LOW);
	Serial.printf("switch to class %c done\n", "ABC"[Class]);

	// Informs the server that switch has occurred ASAP
	m_lora_app_data.buffsize = 0;
	m_lora_app_data.port = LORAWAN_APP_PORT;
	lmh_send(&m_lora_app_data, LMH_UNCONFIRMED_MSG);
}

static void lorawan_unconf_finished(void)
{
	digitalWrite(LED_GREEN, LOW);
	Serial.println("TX unconfirmed finished");
	tx_active = false;
}

static void lorawan_conf_finished(bool result)
{
	digitalWrite(LED_GREEN, LOW);
	Serial.printf("TX confirmed finished with %s\n", result ? "ACK" : "NAK");
	tx_active = false;
}

std::map<eTaskState, const char *> eTaskStateName{{eReady, "Ready"}, {eRunning, "Running"}, {eBlocked, "Blocked"}, {eSuspended, "Suspended"}, {eDeleted, "Deleted"}};
void task_list(void)
{
	int tasks = uxTaskGetNumberOfTasks();
	TaskStatus_t *pxTaskStatusArray = new TaskStatus_t[tasks];
	unsigned long runtime;
	tasks = uxTaskGetSystemState(pxTaskStatusArray, tasks, &runtime);
	Serial.printf("# Tasks: %d\n", tasks);
	Serial.println("ID, NAME,            STATE,        PRIO,  CYCLES");
	for (int i = 0; i < tasks; i++)
	{
		Serial.printf("%02d: %-16s %-10s      %d    %lu\n", i, pxTaskStatusArray[i].pcTaskName, eTaskStateName[pxTaskStatusArray[i].eCurrentState], (int)pxTaskStatusArray[i].uxCurrentPriority, pxTaskStatusArray[i].ulRunTimeCounter);
	}
	delete[] pxTaskStatusArray;
}
