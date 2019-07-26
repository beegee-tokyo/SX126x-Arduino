#include <Arduino.h>
#include <SX126x-ESP32.h>

// Function declarations
void enableRX(void);
void enableTX(void);
void OnTxDone(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxTimeout(void);
void OnRxTimeout(void);
void OnRxError(void);
void OnCadDone(bool cadResult);

// Check if the board has an LED port defined
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// Define Chipset
#define SX1262_CHIP

// Define LoRa parameters
#define RF_FREQUENCY 915000000  // Hz
#define TX_OUTPUT_POWER 14		// dBm
#define LORA_BANDWIDTH 0		// [0: 125 kHz, \
                                //  1: 250 kHz, \
                                //  2: 500 kHz, \
                                //  3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12] was 7
#define LORA_CODINGRATE 1		// [1: 4/5,       \
                                //  2: 4/6, was 1 \
                                //  3: 4/7,       \
                                //  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 3000
#define TX_TIMEOUT_VALUE 3000

// ESP32 - SX126x pin configuration
#define PIN_LORA_RESET 4  // LORA RESET
#define PIN_LORA_NSS 5	// LORA SPI CS
#define PIN_LORA_SCLK 18  // LORA SPI CLK
#define PIN_LORA_MISO 19  // LORA SPI MISO
#define PIN_LORA_DIO_1 21 // LORA DIO_1
#define PIN_LORA_BUSY 22  // LORA SPI BUSY
#define PIN_LORA_MOSI 23  // LORA SPI MOSI
#define RADIO_TXEN 26	 // LORA ANTENNA TX ENABLE
#define RADIO_RXEN 27	 // LORA ANTENNA RX ENABLE
#define BUFFER_SIZE 64	// Define the payload size here

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

	// Needed for eByte E22-900M modules to control the antenna!
	pinMode(RADIO_RXEN, OUTPUT);
	pinMode(RADIO_TXEN, OUTPUT);

	// Reset the LoRa chip
	pinMode(PIN_LORA_RESET, OUTPUT);
	digitalWrite(PIN_LORA_RESET, LOW);
	delay(10);
	digitalWrite(PIN_LORA_RESET, HIGH);

	// Initialize Serial for debug output
	Serial.begin(115200);

	Serial.println("=====================================");
	Serial.println("SX126x PingPong test");
	Serial.println("=====================================");

	// Initialize the LoRa chip
	Serial.println("Starting lora_hardware_init");
	lora_hardware_init();

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
	enableRX();
	Radio.Rx(RX_TIMEOUT_VALUE);

	timeToSend = millis();
}

void loop()
{
	// Handle Radio events
	Radio.IrqProcess();
	if ((millis() - timeToSend) > 10000)
	{
		Serial.println("<<< 10 seconds >>>");
		timeToSend = millis();
	}
}

/**@brief Function to be executed on Radio Tx Done event
 */
void OnTxDone(void)
{
	Serial.println("OnTxDone");
	enableRX();
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
	digitalWrite(LED_BUILTIN, HIGH);

	if (isMaster == true)
	{
		if (BufferSize > 0)
		{
			if (strncmp((const char *)RcvBuffer, (const char *)PongMsg, 4) == 0)
			{
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

				// Wait 500ms before sending the next package
				delay(500);

				// Check if our channel is available for sending
				enableRX();
				SX126xSetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
				SX126xSetDioIrqParams(IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED,
									  IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED,
									  IRQ_RADIO_NONE, IRQ_RADIO_NONE);
				cadTime = millis();
				Radio.StartCad();

				// Sending will be started when the channel is free
				// enableTX();
				// Radio.Send(TxdBuffer, BufferSize);
			}
			else if (strncmp((const char *)RcvBuffer, (const char *)PingMsg, 4) == 0)
			{ // A master already exists then become a slave
				isMaster = false;
				enableRX();
				Radio.Rx(RX_TIMEOUT_VALUE);
			}
			else // valid reception but neither a PING or a PONG message
			{	// Set device as master and start again
				isMaster = true;
				enableRX();
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
				// Send the reply to the PONG string
				TxdBuffer[0] = 'P';
				TxdBuffer[1] = 'O';
				TxdBuffer[2] = 'N';
				TxdBuffer[3] = 'G';
				// We fill the buffer with numbers for the payload
				for (int i = 4; i < BufferSize; i++)
				{
					TxdBuffer[i] = i - 4;
				}

				// Wait 500ms before sending the next package
				delay(500);

				// Check if our channel is available for sending
				enableRX();
				SX126xSetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
				SX126xSetDioIrqParams(IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED,
									  IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED,
									  IRQ_RADIO_NONE, IRQ_RADIO_NONE);
				cadTime = millis();
				Radio.StartCad();

				// Sending will be started when the channel is free
				// enableTX();
				// Radio.Send(TxdBuffer, BufferSize);
			}
			else // valid reception but not a PING as expected
			{	// Set device as master and start again
				isMaster = true;
				enableRX();
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
	digitalWrite(LED_BUILTIN, LOW);

	enableRX();
	Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Timeout event
 */
void OnRxTimeout(void)
{
	Serial.println("OnRxTimeout");
	delay(10);
	digitalWrite(LED_BUILTIN, LOW);

	if (isMaster == true)
	{
		// Send the next PING frame
		TxdBuffer[0] = 'P';
		TxdBuffer[1] = 'I';
		TxdBuffer[2] = 'N';
		TxdBuffer[3] = 'G';
		for (int i = 4; i < BufferSize; i++)
		{
			TxdBuffer[i] = i - 4;
		}

		Serial.println("Radio.Standby");
		Radio.Standby();
		// Wait 500ms before sending the next package
		delay(500);

		// Check if our channel is available for sending
		Serial.println("Set CAD params");
		enableRX();
		SX126xSetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
		SX126xSetDioIrqParams(IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED,
							  IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED,
							  IRQ_RADIO_NONE, IRQ_RADIO_NONE);
		cadTime = millis();
		Serial.println("Start CAD");
		Radio.StartCad();

		// Sending will be started when the channel is free
		// enableTX();
		// Radio.Send(TxdBuffer, BufferSize);
	}
	else
	{
		enableRX();
		Radio.Rx(RX_TIMEOUT_VALUE);
	}
}

/**@brief Function to be executed on Radio Rx Error event
 */
void OnRxError(void)
{
	Serial.println("OnRxError");
	delay(10);
	digitalWrite(LED_BUILTIN, LOW);

	if (isMaster == true)
	{
		// Send the next PING frame
		TxdBuffer[0] = 'P';
		TxdBuffer[1] = 'I';
		TxdBuffer[2] = 'N';
		TxdBuffer[3] = 'G';
		for (int i = 4; i < BufferSize; i++)
		{
			TxdBuffer[i] = i - 4;
		}

		// Wait 500ms before sending the next package
		delay(500);

		// Check if our channel is available for sending
		enableRX();
		SX126xSetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
		SX126xSetDioIrqParams(IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED,
							  IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED,
							  IRQ_RADIO_NONE, IRQ_RADIO_NONE);
		cadTime = millis();
		Radio.StartCad();

		// Sending will be started when the channel is free
		// enableTX();
		// Radio.Send(TxdBuffer, BufferSize);
	}
	else
	{
		enableRX();
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
	}
	else
	{
		Serial.printf("CAD returned channel free after %ldms\n", duration);
		enableTX();
		Radio.Send(TxdBuffer, BufferSize);
	}
}

/**@brief Function to switch the LoRa antenna of an eByte E22 module to receive mode
 */
void enableRX(void)
{
	digitalWrite(RADIO_RXEN, HIGH);
	digitalWrite(RADIO_TXEN, LOW);
}

/**@brief Function to switch the LoRa antenna of an eByte E22 module to transmit mode
 */
void enableTX(void)
{
	digitalWrite(RADIO_RXEN, LOW);
	digitalWrite(RADIO_TXEN, HIGH);
}
