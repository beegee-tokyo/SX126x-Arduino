#include <Arduino.h>

#include <SX126x-Arduino.h>
#include <SPI.h>

#ifdef _VARIANT_RAK4630_
// No HW config required
#else
hw_config hwConfig;

#ifdef ESP32
#ifdef _VARIANT_RAK11200_
// ESP32 - SX126x pin configuration (RAK13300 assignment)
int PIN_LORA_RESET = WB_IO4; // LORA RESET
int PIN_LORA_DIO_1 = WB_IO6; // LORA DIO_1
int PIN_LORA_BUSY = WB_IO5;	 // LORA SPI BUSY
int PIN_LORA_NSS = SS;		 // LORA SPI CS
int PIN_LORA_SCLK = SCK;	 // LORA SPI CLK
int PIN_LORA_MISO = MISO;	 // LORA SPI MISO
int PIN_LORA_MOSI = MOSI;	 // LORA SPI MOSI
int RADIO_TXEN = -1;		 // LORA ANTENNA TX ENABLE UNUSED
int RADIO_RXEN = WB_IO3;	 // LORA ANTENNA RX ENABLE
#else
// ESP32 - SX126x pin configuration
int PIN_LORA_RESET = 4;	 // LORA RESET
int PIN_LORA_DIO_1 = 21; // LORA DIO_1
int PIN_LORA_BUSY = 22;	 // LORA SPI BUSY
int PIN_LORA_NSS = 5;	 // LORA SPI CS
int PIN_LORA_SCLK = 18;	 // LORA SPI CLK
int PIN_LORA_MISO = 19;	 // LORA SPI MISO
int PIN_LORA_MOSI = 23;	 // LORA SPI MOSI
int RADIO_TXEN = 26;	 // LORA ANTENNA TX ENABLE
int RADIO_RXEN = 27;	 // LORA ANTENNA RX ENABLE
#endif
#endif
#ifdef ESP8266
// ESP32 - SX126x pin configuration
int PIN_LORA_RESET = 0;	  // LORA RESET
int PIN_LORA_DIO_1 = 15;  // LORA DIO_1
int PIN_LORA_BUSY = 16;	  // LORA SPI BUSY
int PIN_LORA_NSS = 2;	  // LORA SPI CS
int PIN_LORA_SCLK = SCK;  // LORA SPI CLK
int PIN_LORA_MISO = MISO; // LORA SPI MISO
int PIN_LORA_MOSI = MOSI; // LORA SPI MOSI
int RADIO_TXEN = -1;	  // LORA ANTENNA TX ENABLE
int RADIO_RXEN = -1;	  // LORA ANTENNA RX ENABLE
#endif
#ifdef NRF52_SERIES
// nRF52832 - SX126x pin configuration
int PIN_LORA_RESET = 30; // LORA RESET
int PIN_LORA_DIO_1 = 27; // LORA DIO_1
int PIN_LORA_BUSY = 7;	 // LORA SPI BUSY
int PIN_LORA_NSS = 11;	 // LORA SPI CS
int PIN_LORA_SCLK = 12;	 // LORA SPI CLK
int PIN_LORA_MISO = 14;	 // LORA SPI MISO
int PIN_LORA_MOSI = 13;	 // LORA SPI MOSI
int RADIO_TXEN = -1;	 // LORA ANTENNA TX ENABLE
int RADIO_RXEN = -1;	 // LORA ANTENNA RX ENABLE
// Replace PIN_SPI_MISO, PIN_SPI_SCK, PIN_SPI_MOSI with your
SPIClass SPI_LORA(NRF_SPIM2, PIN_LORA_MISO, PIN_LORA_SCLK, PIN_LORA_MOSI);
#endif
#endif

// Function declarations
void OnTxDone(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxTimeout(void);
void OnRxTimeout(void);
void OnRxError(void);
void OnCadDone(bool cadResult);

#if defined (_VARIANT_RAK4630_) || defined (_VARIANT_RAK11200)
// LED's are defined
#else
	// Check if the board has an LED port defined
	#ifdef ESP32
		#ifndef LED_BUILTIN
			#define LED_BUILTIN 2
		#endif
	#endif
	#ifdef ESP8266
		#ifndef LED_BUILTIN
			#define LED_BUILTIN 2
		#endif
	#endif
	#ifdef NRF52_SERIES
		#ifndef LED_BUILTIN
			#define LED_BUILTIN 17
		#endif
	#endif
#endif

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
#define RX_TIMEOUT_VALUE 3000
#define TX_TIMEOUT_VALUE 5000

#define BUFFER_SIZE 64 // Define the payload size here

static RadioEvents_t RadioEvents;
static uint16_t BufferSize = BUFFER_SIZE;
static uint8_t RcvBuffer[BUFFER_SIZE];
static uint8_t TxdBuffer[BUFFER_SIZE];
static bool isMaster = true;
const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

time_t timeToSend;

time_t cadTime;

uint8_t pingCnt = 0;
uint8_t pongCnt = 0;

#define NO_EVENT 0b0000000000000000
#define TX_FIN 0b0000000000000001
#define N_TX_FIN 0b1111111111111110
#define TX_ERR 0b0000000000000010
#define N_TX_ERR 0b1111111111111101
#define RX_FIN 0b0000000000000100
#define N_RX_FIN 0b1111111111111011
#define RX_ERR 0b0000000000010000
#define N_RX_ERR 0b1111111111101111
#define CAD_FIN 0b0000000000100000
#define N_CAD_FIN 0b1111111111011111

/** Semaphore used by events to wake up loop task */
SemaphoreHandle_t g_task_sem = NULL;

/** Flag for the event type */
volatile uint16_t g_task_event_type = NO_EVENT;

// RX data
uint8_t *rx_payload;
uint16_t rx_size;
int16_t rx_rssi;
int8_t rx_snr;

// CAD result
bool tx_cadResult;

void setup()
{
#ifdef _VARIANT_RAK11200_
	// RAK11200 uses RAK13300 for LoRa, needs power on
	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, HIGH);
#endif
#ifdef _VARIANT_RAK4630_
// No HW config required
#else
	// Define the HW configuration between MCU and SX126x
	hwConfig.CHIP_TYPE = SX1262_CHIP;		  // Example uses an eByte E22 module with an SX1262
	hwConfig.PIN_LORA_RESET = PIN_LORA_RESET; // LORA RESET
	hwConfig.PIN_LORA_NSS = PIN_LORA_NSS;	  // LORA SPI CS
	hwConfig.PIN_LORA_SCLK = PIN_LORA_SCLK;	  // LORA SPI CLK
	hwConfig.PIN_LORA_MISO = PIN_LORA_MISO;	  // LORA SPI MISO
	hwConfig.PIN_LORA_DIO_1 = PIN_LORA_DIO_1; // LORA DIO_1
	hwConfig.PIN_LORA_BUSY = PIN_LORA_BUSY;	  // LORA SPI BUSY
	hwConfig.PIN_LORA_MOSI = PIN_LORA_MOSI;	  // LORA SPI MOSI
	hwConfig.RADIO_TXEN = RADIO_TXEN;		  // LORA ANTENNA TX ENABLE
	hwConfig.RADIO_RXEN = RADIO_RXEN;		  // LORA ANTENNA RX ENABLE
	hwConfig.USE_DIO2_ANT_SWITCH = true;	  // Example uses an CircuitRocks Alora RFM1262 which uses DIO2 pins as antenna control
	hwConfig.USE_DIO3_TCXO = true;			  // Example uses an CircuitRocks Alora RFM1262 which uses DIO3 to control oscillator voltage
	hwConfig.USE_DIO3_ANT_SWITCH = false;	  // Only Insight ISP4520 module uses DIO3 as antenna control
	hwConfig.USE_RXEN_ANT_PWR = true;		  // RXEN is used as power for antenna switch
#endif
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	// Initialize Serial for debug output
	Serial.begin(115200);

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
			break;
		}
	}

	Serial.println("=====================================");
	Serial.println("SX126x PingPong test");
	Serial.println("=====================================");

#ifdef _VARIANT_RAK4630_
	Serial.println("RAKwireless RAK4630");
#else
	#ifdef NRF52_SERIES
		Serial.println("MCU Nordic nRF52832");
		pinMode(30, OUTPUT);
		digitalWrite(30, HIGH);
	#endif
	#ifdef ESP32
		#ifdef _VARIANT_RAK11200_
			Serial.println("RAKwireless RAK11200");
		#else
			Serial.println("MCU Espressif ESP32");
		#endif
	#endif
#endif

	uint8_t deviceId[8];

	BoardGetUniqueId(deviceId);
	Serial.printf("BoardId: %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n",
				  deviceId[7],
				  deviceId[6],
				  deviceId[5],
				  deviceId[4],
				  deviceId[3],
				  deviceId[2],
				  deviceId[1],
				  deviceId[0]);

	// Initialize the LoRa chip
	Serial.println("Starting lora_hardware_init");
	uint32_t init_result = 0;

#if defined(_VARIANT_RAK4630_)
	init_result = lora_rak4630_init();
	Serial.printf("RAK4631 LoRa init %s\r\n", init_result == 0 ? "success" : "failed");
#else
#if defined(_VARIANT_RAK11200_)
	init_result = lora_rak13300_init();
	Serial.printf("RAK13300 LoRa init %s\r\n", init_result == 0 ? "success" : "failed");
#else
	init_result = lora_hardware_init(hwConfig);
	Serial.printf("LoRa init %s\r\n", init_result == 0 ? "success" : "failed");
#endif
#endif

	// Initialize the Radio callbacks
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.RxDone = OnRxDone;
	RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.RxTimeout = OnRxTimeout;
	RadioEvents.RxError = OnRxError;
	RadioEvents.CadDone = OnCadDone;

	// Initialize the Radio
	Radio.Init(&RadioEvents);

	// Set radio to sleep for next setup
	Radio.Sleep();

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

	// Create the task event semaphore
	g_task_sem = xSemaphoreCreateBinary();
	// Initialize semaphore
	xSemaphoreGive(g_task_sem);

	// Take the semaphore so the loop will go to sleep until an event happens
	xSemaphoreTake(g_task_sem, 10);

	// Start LoRa
	Serial.println("Starting Radio.Rx");
	Radio.Rx(RX_TIMEOUT_VALUE);
}

void loop()
{
	// Wait until semaphore is released (FreeRTOS)
	xSemaphoreTake(g_task_sem, portMAX_DELAY);

	Serial.println("Loop wakeup");

	// Handle event
	while (g_task_event_type != NO_EVENT)
	{
		if ((g_task_event_type & TX_FIN) == TX_FIN)
		{
			g_task_event_type &= N_TX_FIN;
			Serial.println("OnTxDone");
			Radio.Sleep();
			Radio.Rx(RX_TIMEOUT_VALUE);
		}
		if ((g_task_event_type & TX_ERR) == TX_ERR)
		{
			g_task_event_type &= N_TX_ERR;
			Serial.println("OnTxTimeout");
			digitalWrite(LED_BUILTIN, LOW);
			Radio.Sleep();
			Radio.Rx(RX_TIMEOUT_VALUE);
		}
		if ((g_task_event_type & RX_FIN) == RX_FIN)
		{
			g_task_event_type &= N_RX_FIN;
			Serial.println("OnRxDone");
			delay(10);

			Serial.printf("RssiValue=%d dBm, SnrValue=%d\n", rx_rssi, rx_snr);

			for (int idx = 0; idx < rx_size; idx++)
			{
				Serial.printf("%02X ", RcvBuffer[idx]);
			}
			Serial.println("");

			if (isMaster == true)
			{
				digitalWrite(LED_BUILTIN, HIGH);

				if (BufferSize > 0)
				{
					if (strncmp((const char *)RcvBuffer, (const char *)PongMsg, 4) == 0)
					{
						Serial.println("Received a PONG in OnRxDone as Master");
						// Wait 500ms before sending the next package
						delay(500);

						// Check if our channel is available for sending
						Radio.Sleep();
						Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
						cadTime = millis();
						Radio.StartCad();
						// Sending next Ping will be started when the channel is free
					}
					else if (strncmp((const char *)RcvBuffer, (const char *)PingMsg, 4) == 0)
					{ // A master already exists then become a slave
						Serial.println("Received a PING in OnRxDone as Master, switch to Slave");
						isMaster = false;
						// Check if our channel is available for sending
						Radio.Sleep();
						Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
						cadTime = millis();
						Radio.StartCad();
						// Sending next Pong will be started when the channel is free
					}
					else // valid reception but neither a PING or a PONG message
					{	 // Set device as master and start again
						isMaster = true;
						Radio.Rx(RX_TIMEOUT_VALUE);
					}
				}
			}
			else
			{
				if (BufferSize > 0)
				{
					if (strncmp((const char *)RcvBuffer, (const char *)PingMsg, 4) == 0)
					{
						Serial.println("Received a PING in OnRxDone as Slave");
						// Check if our channel is available for sending
						Radio.Sleep();
						Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
						cadTime = millis();
						Radio.StartCad();
						// Sending Pong will be started when the channel is free
					}
					else // valid reception but not a PING as expected
					{	 // Set device as master and start again
						Serial.println("Received something in OnRxDone as Slave");
						isMaster = true;
						Radio.Rx(RX_TIMEOUT_VALUE);
					}
				}
			}
		}
		if ((g_task_event_type & RX_ERR) == RX_ERR)
		{
			g_task_event_type &= N_RX_ERR;
			Serial.println("OnRxError");
			digitalWrite(LED_BUILTIN, LOW);

			if (isMaster == true)
			{
				// Wait 500ms before sending the next package
				delay(500);
				Serial.println("Send as Master after RX timeout");
				// Check if our channel is available for sending
				Radio.Sleep();
				Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
				cadTime = millis();
				Radio.StartCad();
				// Sending the ping will be started when the channel is free
			}
			else
			{
				// No Ping received within timeout, switch to Master
				isMaster = true;
				Serial.println("Switch to Master after RX timeout");
				// Check if our channel is available for sending
				Radio.Sleep();
				Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
				cadTime = millis();
				Radio.StartCad();
				// Sending the ping will be started when the channel is free
			}
		}
		if ((g_task_event_type & CAD_FIN) == CAD_FIN)
		{
			g_task_event_type &= N_CAD_FIN;
			time_t duration = millis() - cadTime;
			if (tx_cadResult)
			{
				Serial.printf("CAD returned channel busy after %ldms\n", duration);
				Radio.Rx(RX_TIMEOUT_VALUE);
			}
			else
			{
				Serial.printf("CAD returned channel free after %ldms\n", duration);
				if (isMaster)
				{
					digitalWrite(LED_BUILTIN, HIGH);
					Serial.println("Sending a PING in OnCadDone as Master");
					// Send the next PING frame
					TxdBuffer[0] = 'P';
					TxdBuffer[1] = 'I';
					TxdBuffer[2] = 'N';
					TxdBuffer[3] = 'G';
				}
				else
				{
					digitalWrite(LED_BUILTIN, LOW);
					Serial.println("Sending a PONG in OnCadDone as Slave");
					// Send the reply to the PONG string
					TxdBuffer[0] = 'P';
					TxdBuffer[1] = 'O';
					TxdBuffer[2] = 'N';
					TxdBuffer[3] = 'G';
				}
				// We fill the buffer with numbers for the payload
				for (int i = 4; i < BufferSize; i++)
				{
					TxdBuffer[i] = i - 4;
				}

				Radio.Send(TxdBuffer, BufferSize);
			}
		}
	}
}

/**@brief Function to be executed on Radio Tx Done event
 */
void OnTxDone(void)
{
	Serial.println("OnTxDone CB");
	g_task_event_type |= TX_FIN;
	// Wake up task to send initial packet
	xSemaphoreGive(g_task_sem);
}

/**@brief Function to be executed on Radio Rx Done event
 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	Serial.println("OnRxDone CB");
	rx_payload = payload;
	rx_size = size;
	rx_rssi = rssi;
	rx_snr = snr;
	g_task_event_type |= RX_FIN;

	memcpy(RcvBuffer, payload, size);

	// Wake up task to send initial packet
	xSemaphoreGive(g_task_sem);
}

/**@brief Function to be executed on Radio Tx Timeout event
 */
void OnTxTimeout(void)
{
	Serial.println("OnTxTimeout CB");
	g_task_event_type |= TX_ERR;

	// Wake up task to send initial packet
	xSemaphoreGive(g_task_sem);
}

/**@brief Function to be executed on Radio Rx Timeout event
 */
void OnRxTimeout(void)
{
	Serial.println("OnRxTimeout CB");
	g_task_event_type |= RX_ERR;

	// Wake up task to send initial packet
	xSemaphoreGive(g_task_sem);
}

/**@brief Function to be executed on Radio Rx Error event
 */
void OnRxError(void)
{
	Serial.println("OnRxError CB");
	g_task_event_type |= RX_ERR;

	// Wake up task to send initial packet
	xSemaphoreGive(g_task_sem);
}

/**@brief Function to be executed on CAD Done event
 */
void OnCadDone(bool cadResult)
{
	Serial.println("OnCadDone CB");
	tx_cadResult = cadResult;
	g_task_event_type |= CAD_FIN;

	// Wake up task to send initial packet
	xSemaphoreGive(g_task_sem);
}
