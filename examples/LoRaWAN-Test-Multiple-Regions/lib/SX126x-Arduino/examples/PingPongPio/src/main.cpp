#include <Arduino.h>

#include <SX126x-Arduino.h>
#include <SPI.h>

hw_config hwConfig;

#ifdef ESP32
// ESP32 - SX126x pin configuration
int PIN_LORA_RESET = 4;  // LORA RESET
int PIN_LORA_DIO_1 = 21; // LORA DIO_1
int PIN_LORA_BUSY = 22;  // LORA SPI BUSY
int PIN_LORA_NSS = 5;	// LORA SPI CS
int PIN_LORA_SCLK = 18;  // LORA SPI CLK
int PIN_LORA_MISO = 19;  // LORA SPI MISO
int PIN_LORA_MOSI = 23;  // LORA SPI MOSI
int RADIO_TXEN = 26;	 // LORA ANTENNA TX ENABLE
int RADIO_RXEN = 27;	 // LORA ANTENNA RX ENABLE
#endif
#ifdef ESP8266
// ESP32 - SX126x pin configuration
int PIN_LORA_RESET = 0;   // LORA RESET
int PIN_LORA_DIO_1 = 15;  // LORA DIO_1
int PIN_LORA_BUSY = 16;   // LORA SPI BUSY
int PIN_LORA_NSS = 2;	 // LORA SPI CS
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
int PIN_LORA_BUSY = 7;   // LORA SPI BUSY
int PIN_LORA_NSS = 11;   // LORA SPI CS
int PIN_LORA_SCLK = 12;  // LORA SPI CLK
int PIN_LORA_MISO = 14;  // LORA SPI MISO
int PIN_LORA_MOSI = 13;  // LORA SPI MOSI
int RADIO_TXEN = -1;	 // LORA ANTENNA TX ENABLE
int RADIO_RXEN = -1;	 // LORA ANTENNA RX ENABLE
// Replace PIN_SPI_MISO, PIN_SPI_SCK, PIN_SPI_MOSI with your
SPIClass SPI_LORA(NRF_SPIM2, PIN_LORA_MISO, PIN_LORA_SCLK, PIN_LORA_MOSI);
#endif

// Function declarations
void OnTxDone(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxTimeout(void);
void OnRxTimeout(void);
void OnRxError(void);
void OnCadDone(bool cadResult);
#ifdef NRF52_SERIES
// Start BLE if we compile for nRF52
#include <bluefruit.h>
void initBLE();
extern bool bleUARTisConnected;
extern BLEUart bleuart;
#endif

// Check if the board has an LED port defined
#ifdef ESP32
#define LED_BUILTIN 2
#endif
#ifdef ESP8266
#define LED_BUILTIN 2
#endif
#ifdef NRF52_SERIES
#define LED_BUILTIN 17
#endif

// Define LoRa parameters
#define RF_FREQUENCY 868000000  // Hz
#define TX_OUTPUT_POWER 22		// dBm
#define LORA_BANDWIDTH 0		// [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1		// [1: 4/5, 2: 4/6,  3: 4/7,  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 3000
#define TX_TIMEOUT_VALUE 3000

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

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	// Define the HW configuration between MCU and SX126x
	hwConfig.CHIP_TYPE = SX1262_CHIP;		  // Example uses an eByte E22 module with an SX1262
	hwConfig.PIN_LORA_RESET = PIN_LORA_RESET; // LORA RESET
	hwConfig.PIN_LORA_NSS = PIN_LORA_NSS;	 // LORA SPI CS
	hwConfig.PIN_LORA_SCLK = PIN_LORA_SCLK;   // LORA SPI CLK
	hwConfig.PIN_LORA_MISO = PIN_LORA_MISO;   // LORA SPI MISO
	hwConfig.PIN_LORA_DIO_1 = PIN_LORA_DIO_1; // LORA DIO_1
	hwConfig.PIN_LORA_BUSY = PIN_LORA_BUSY;   // LORA SPI BUSY
	hwConfig.PIN_LORA_MOSI = PIN_LORA_MOSI;   // LORA SPI MOSI
	hwConfig.RADIO_TXEN = RADIO_TXEN;		  // LORA ANTENNA TX ENABLE
	hwConfig.RADIO_RXEN = RADIO_RXEN;		  // LORA ANTENNA RX ENABLE
	hwConfig.USE_DIO2_ANT_SWITCH = true;	  // Example uses an CircuitRocks Alora RFM1262 which uses DIO2 pins as antenna control
	hwConfig.USE_DIO3_TCXO = true;			  // Example uses an CircuitRocks Alora RFM1262 which uses DIO3 to control oscillator voltage
	hwConfig.USE_DIO3_ANT_SWITCH = false;	 // Only Insight ISP4520 module uses DIO3 as antenna control

	// Initialize Serial for debug output
	Serial.begin(115200);

	Serial.println("=====================================");
	Serial.println("SX126x PingPong test");
	Serial.println("=====================================");

#ifdef NRF52_SERIES
	Serial.println("MCU Nordic nRF52832");
	pinMode(30, OUTPUT);
	digitalWrite(30, HIGH);
	// Start BLE if we compile for nRF52
	initBLE();
#endif
#ifdef ESP32
	Serial.println("MCU Espressif ESP32");
#endif
#ifdef ESP8266
	Serial.println("MCU Espressif ESP8266");
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
	lora_hardware_init(hwConfig);

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
	Radio.Rx(RX_TIMEOUT_VALUE);

	timeToSend = millis();
}

void loop()
{
	// Handle Radio events
	Radio.IrqProcess();

	// We are on FreeRTOS, give other tasks a chance to run
	delay(100);
	yield();
}

/**@brief Function to be executed on Radio Tx Done event
 */
void OnTxDone(void)
{
	Serial.println("OnTxDone");
#ifdef NRF52_SERIES
	if (bleUARTisConnected)
	{
		bleuart.print("OnTxDone\n");
	}
#endif
	Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Done event
 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	Serial.println("OnRxDone");
#ifdef NRF52_SERIES
	if (bleUARTisConnected)
	{
		bleuart.print("OnRxDone\n");
	}
#endif
	delay(10);
	BufferSize = size;
	memcpy(RcvBuffer, payload, BufferSize);

	Serial.printf("RssiValue=%d dBm, SnrValue=%d\n", rssi, snr);

	for (int idx = 0; idx < size; idx++)
	{
		Serial.printf("%02X ", RcvBuffer[idx]);
	}
	Serial.println("");

#ifdef NRF52_SERIES
	if (bleUARTisConnected)
	{
		bleuart.printf("RssiValue=%d dBm, SnrValue=%d\n", rssi, snr);
	}
#endif
	digitalWrite(LED_BUILTIN, HIGH);

	if (isMaster == true)
	{
		if (BufferSize > 0)
		{
			if (strncmp((const char *)RcvBuffer, (const char *)PongMsg, 4) == 0)
			{
				Serial.println("Received a PONG in OnRxDone as Master");
#ifdef NRF52_SERIES
				if (bleUARTisConnected)
				{
					bleuart.print("Received a PONG in OnRxDone as Master\n");
				}
#endif

				// Wait 500ms before sending the next package
				delay(500);

				// Check if our channel is available for sending
				Radio.Standby();
				Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
				cadTime = millis();
				Radio.StartCad();
				// Sending next Ping will be started when the channel is free
			}
			else if (strncmp((const char *)RcvBuffer, (const char *)PingMsg, 4) == 0)
			{ // A master already exists then become a slave
				Serial.println("Received a PING in OnRxDone as Master");
#ifdef NRF52_SERIES
				if (bleUARTisConnected)
				{
					bleuart.print("Received a PING in OnRxDone as Master\n");
				}
#endif
				isMaster = false;
				Radio.Rx(RX_TIMEOUT_VALUE);
			}
			else // valid reception but neither a PING or a PONG message
			{	// Set device as master and start again
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
#ifdef NRF52_SERIES
				if (bleUARTisConnected)
				{
					bleuart.print("Received a PING in OnRxDone as Slave\n");
				}
#endif

				// Check if our channel is available for sending
				Radio.Standby();
				Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
				cadTime = millis();
				Radio.StartCad();
				// Sending Pong will be started when the channel is free
			}
			else // valid reception but not a PING as expected
			{	// Set device as master and start again
				Serial.println("Received something in OnRxDone as Slave");
#ifdef NRF52_SERIES
				if (bleUARTisConnected)
				{
					bleuart.print("Received something in OnRxDone as Slave\n");
				}
#endif
				isMaster = true;
				Radio.Rx(RX_TIMEOUT_VALUE);
			}
		}
	}
}

/**@brief Function to be executed on Radio Tx Timeout event
 */
void OnTxTimeout(void)
{
	// Radio.Sleep();
	Serial.println("OnTxTimeout");
#ifdef NRF52_SERIES
	if (bleUARTisConnected)
	{
		bleuart.print("OnTxTimeout\n");
	}
#endif
	digitalWrite(LED_BUILTIN, LOW);

	Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Timeout event
 */
void OnRxTimeout(void)
{
	Serial.println("OnRxTimeout");
#ifdef NRF52_SERIES
	if (bleUARTisConnected)
	{
		bleuart.print("OnRxTimeout\n");
	}
#endif

	digitalWrite(LED_BUILTIN, LOW);

	if (isMaster == true)
	{
		// Wait 500ms before sending the next package
		delay(500);

		// Check if our channel is available for sending
		Radio.Standby();
		Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
		cadTime = millis();
		Radio.StartCad();
		// Sending the ping will be started when the channel is free
	}
	else
	{
		// No Ping received within timeout, switch to Master
		isMaster = true;
		// Check if our channel is available for sending
		Radio.Standby();
		Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
		cadTime = millis();
		Radio.StartCad();
		// Sending the ping will be started when the channel is free
	}
}

/**@brief Function to be executed on Radio Rx Error event
 */
void OnRxError(void)
{
	Serial.println("OnRxError");
#ifdef NRF52_SERIES
	if (bleUARTisConnected)
	{
		bleuart.print("OnRxError\n");
	}
#endif
	digitalWrite(LED_BUILTIN, LOW);

	if (isMaster == true)
	{
		// Wait 500ms before sending the next package
		delay(500);

		// Check if our channel is available for sending
		Radio.Standby();
		Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
		cadTime = millis();
		Radio.StartCad();
		// Sending the ping will be started when the channel is free
	}
	else
	{
		Radio.Rx(RX_TIMEOUT_VALUE);
	}
}

/**@brief Function to be executed on Radio Rx Error event
 */
void OnCadDone(bool cadResult)
{
	time_t duration = millis() - cadTime;
	if (cadResult)
	{
		Serial.printf("CAD returned channel busy after %ldms\n", duration);
#ifdef NRF52_SERIES
		if (bleUARTisConnected)
		{
			bleuart.printf("CAD returned channel busy after %ldms\n", duration);
		}
#endif
		Radio.Rx(RX_TIMEOUT_VALUE);
	}
	else
	{
		Serial.printf("CAD returned channel free after %ldms\n", duration);
#ifdef NRF52_SERIES
		if (bleUARTisConnected)
		{
			bleuart.printf("CAD returned channel free after %ldms\n", duration);
		}
#endif
		if (isMaster)
		{
			Serial.println("Sending a PING in OnCadDone as Master");
#ifdef NRF52_SERIES
			if (bleUARTisConnected)
			{
				bleuart.print("Sending a PING in OnCadDone as Master\n");
			}
#endif
			// Send the next PING frame
			TxdBuffer[0] = 'P';
			TxdBuffer[1] = 'I';
			TxdBuffer[2] = 'N';
			TxdBuffer[3] = 'G';
		}
		else
		{
			Serial.println("Sending a PONG in OnCadDone as Slave");
#ifdef NRF52_SERIES
			if (bleUARTisConnected)
			{
				bleuart.print("Sending a PONG in OnCadDone as Slave\n");
			}
#endif
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
