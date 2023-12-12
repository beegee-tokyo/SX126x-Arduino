#include <Arduino.h>
#define RAK11300 1
#include <SX126x-Arduino.h>
#include <LoRaWan-Arduino.h>

#define LORAWAN

#ifndef LED_BLUE
#define LED_BLUE (24)
#endif

#ifndef LED_GREEN
#define LED_GREEN (23)
#endif

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

uint8_t nodeDeviceEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x06, 0x40, 0x80};

uint8_t nodeAppEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x01, 0x42, 0xC8};

uint8_t nodeAppKey[16] = {0x2B, 0x84, 0xE0, 0xB0, 0x9B, 0x68, 0xE5, 0xCB, 0x42, 0x17, 0x6F, 0xE7, 0x53, 0xDC, 0xEE, 0x79};

bool network_joined = false;

static TimerEvent_t timer1_struct;
static TimerEvent_t timer2_struct;

void timer1_cb(void)
{
	digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
}
void timer2_cb(void)
{
	digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
}

// void setup1()
// {
// }

// void loop1()
// {
// 	delay(250);
// 	digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
// 	// Serial.println("Core 2 wakeup");
// }

time_t send_wait;

void setup()
{
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);

	digitalWrite(LED_GREEN, HIGH);
	digitalWrite(LED_BLUE, HIGH);

	time_t serial_timeout = millis();
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

	if (lora_rak11300_init() != 0)
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
		uint32_t err_code = lmh_init(&lora_callbacks, lora_param_init, true, CLASS_A, LORAMAC_REGION_KR920);
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
#endif
		// timer1_struct.oneShot = false;
		// TimerInit(&timer1_struct, timer1_cb);
		// TimerSetValue(&timer1_struct, 5000);
		// timer2_struct.oneShot = false;
		// TimerInit(&timer2_struct, timer2_cb);
		// TimerSetValue(&timer2_struct, 10000);

		// TimerStart(&timer1_struct);
		// TimerStart(&timer2_struct);
	}

	send_wait = millis();
}

bool tx_active = false;

void loop()
{
	// vTaskDelay(20000);
	if ((millis() - send_wait) >= 20000)
	{
		send_wait = millis();

#ifdef LORAWAN
		if (lmh_join_status_get() != LMH_SET)
		{
			// Not joined, try again later
			Serial.println("Did not join network, skip sending frame");
			// Start Join procedure
			lmh_join();
			return;
		}

	if (tx_active)
	{
		Serial.println("Last TX still active, do not send yet");
		return;
	}
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
}

/**@brief LoRa function for handling HasJoined event.
 */
static void lorawan_has_joined_handler(void)
{
	Serial.println("Network Joined");
	network_joined = true;
}

/**@brief Function for handling LoRaWan received data from Gateway
 *
 * @param[app_data] app_data  Pointer to rx data
 */
static void lorawan_rx_handler(lmh_app_data_t *app_data)
{
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
	Serial.printf("switch to class %c done\n", "ABC"[Class]);

	// Informs the server that switch has occurred ASAP
	m_lora_app_data.buffsize = 0;
	m_lora_app_data.port = LORAWAN_APP_PORT;
	lmh_send(&m_lora_app_data, LMH_UNCONFIRMED_MSG);
}

static void lorawan_unconf_finished(void)
{
	Serial.println("TX unconfirmed finished");
	tx_active = false;
}

static void lorawan_conf_finished(bool result)
{
	Serial.printf("TX confirmed finished with %s\n", result ? "ACK" : "NAK");
	tx_active = false;
}
