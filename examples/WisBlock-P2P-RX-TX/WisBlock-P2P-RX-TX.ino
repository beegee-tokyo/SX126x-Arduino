/**
 * @file main.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Simple LoRa P2P RX/TX example
 * @version 0.1
 * @date 2024-12-28
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <Arduino.h>
#include <SX126x-Arduino.h>

// Function declarations
void OnTxDone(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxTimeout(void);
void OnRxTimeout(void);
void OnRxError(void);
void OnCadDone(bool cadResult);
void tx_lora_periodic_handler(void);

// Define LoRa parameters
#define RF_FREQUENCY 916000000	// Hz
#define TX_OUTPUT_POWER 22		// dBm
#define LORA_BANDWIDTH 0		// [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1		// [1: 4/5, 2: 4/6,  3: 4/7,  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8	// Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0	// Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define TX_TIMEOUT_VALUE 5000

/** Structure for radio event callbacks */
static RadioEvents_t RadioEvents;

/** RX buffer */
static uint8_t rx_buffer[64];
#ifdef NRF52_SERIES
/** Payload for RAK4631 */
const uint8_t tx_payload[] = "RAK4631 \0";
#elif defined ESP32
/** Payload for RAK11200/RAK13300 */
const uint8_t tx_payload[] = "RAK11200\0";
#else // RAK11310
/** Payload for RAK11310 */
const uint8_t tx_payload[] = "RAK11310\0";
#endif

/** Time CAD was active */
time_t cad_time;

/** Timer event (from SX126x-Arduino lib) */
TimerEvent_t app_timer; ///< LoRa tranfer timer instance.

/** Flag to send a packet */
volatile bool send_now = false;

/**
 * @brief Arduino setup function, called once
 *
 */
void setup(void)
{
	// Prepare LED's BLUE ==> TX, GREEN ==> Received a packet
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
	digitalWrite(LED_GREEN, LOW);
	digitalWrite(LED_BLUE, LOW);

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
	digitalWrite(LED_GREEN, LOW);

	Serial.println("=====================================");
#ifdef NRF52_SERIES
	Serial.println("RAK4630 SX126x P2P RX/TX test");
#elif defined ESP32
	Serial.println("RAK11200 SX126x P2P RX/TX test");
#else // RAK11310
	Serial.println("RAK11310 SX126x P2P RX/TX test");
#endif
	Serial.println("=====================================");

	// Initialize the LoRa chip
	Serial.println("Starting lora_hardware_init");
#ifdef NRF52_SERIES
	lora_rak4630_init();
#elif defined ESP32
	lora_rak13300_init();
#else // RAK11310
	lora_rak11300_init();
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

	Radio.Rx(0);

	// Set a random send time
	// randomSeed(BoardGetRandomSeed());
	// time_t send_interval = (time_t)random(5000, 15000);

	// Set a fixed time (might cause TX conflicts)
	time_t send_interval = 5000;

	// Start a timer (SX126x-Arduino library timer function)
	app_timer.oneShot = false;
	TimerInit(&app_timer, tx_lora_periodic_handler);
	TimerSetValue(&app_timer, send_interval);
	TimerStart(&app_timer);

	Serial.printf("Starting P2P Test, send delay = %ld ms\n", send_interval);
}

/**
 * @brief Arduino loop, runs forever
 *
 */
void loop(void)
{
	if (send_now)
	{
		send_now = false;
		//-----------------------------//
		// Send without channel activity detection
		//-----------------------------//
		// Radio.Send((uint8_t *)tx_payload, 9);
		//-----------------------------//
		// Send with channel activity detection
		//-----------------------------//
		Radio.Sleep();
		Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
		cad_time = millis();
		Radio.StartCad();

		digitalWrite(LED_BLUE, HIGH);
	}
}

/**
 * @brief Callback after TX finished
 *
 */
void OnTxDone(void)
{
#ifdef NRF52_SERIES
	Serial.println("RAK4630 OnTxDone");
#elif defined ESP32
	Serial.println("RAK11200 OnTxDone");
#else // RAK11310
	Serial.println("RAK11310 OnTxDone");
#endif
	digitalWrite(LED_BLUE, LOW);
	Radio.Rx(0);
}

/**
 * @brief Callback after a packet was received
 *
 * @param payload pointer to received payload
 * @param size  size of payload
 * @param rssi RSSI of received packet
 * @param snr SNR of received packet
 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
#ifdef NRF52_SERIES
	Serial.println("RAK4630 OnRxDone");
#elif defined ESP32
	Serial.println("RAK11200 OnRxDone");
#else // RAK11310
	Serial.println("RAK11310 OnRxDone");
#endif

	memcpy(rx_buffer, payload, size);

#ifdef NRF52_SERIES
	Serial.printf("RAK4630 RSSI=%d dBm, SNR=%d\n", rssi, snr);
#elif defined ESP32
	Serial.printf("RAK11200 RSSI=%d dBm, SNR=%d\n", rssi, snr);
#else // RAK11310
	Serial.printf("RAK11310 RSSI=%d dBm, SNR=%d\n", rssi, snr);
#endif
	Serial.println("|---------------------|");
	Serial.printf("Payload: %s\n", rx_buffer);
	Serial.println("|---------------------|");
	Radio.Rx(0);
	digitalWrite(LED_GREEN, HIGH);
}

/**
 * @brief Callback on TX timeout (should never happen)
 *
 */
void OnTxTimeout(void)
{
#ifdef NRF52_SERIES
	Serial.println("RAK4630 OnTxTimeout");
#elif defined ESP32
	Serial.println("RAK11200 OnTxTimeout");
#else // RAK11310
	Serial.println("RAK11310 OnTxTimeout");
#endif
	Radio.Rx(0);
	digitalWrite(LED_BLUE, LOW);
}

/**
 * @brief Callback on RX timeout (will not happen, we enabled permanent RX)
 *
 */
void OnRxTimeout(void)
{
#ifdef NRF52_SERIES
	Serial.println("RAK4630 OnRxTimeout");
#elif defined ESP32
	Serial.println("RAK11200 OnRxTimeout");
#else // RAK11310
	Serial.println("RAK11310 OnRxTimeout");
#endif
	Radio.Rx(0);
}

/**
 * @brief Callback on RX error, can be CRC mismatch or incomplete transmission
 *
 */
void OnRxError(void)
{
#ifdef NRF52_SERIES
	Serial.println("RAK4630 OnRxError");
#elif defined ESP32
	Serial.println("RAK11200 OnRxError");
#else // RAK11310
	Serial.println("RAK11310 OnRxError");
#endif
	Radio.Rx(0);
}

/**
 * @brief Callback after Channel Activity Detection is finished
 *
 * @param cad_result
 * true ==> channel is in use
 * false ==> channel is available
 */
void OnCadDone(bool cad_result)
{
	time_t duration = millis() - cad_time;
	if (cad_result)
	{
#ifdef NRF52_SERIES
		Serial.printf("RAK4630 CAD returned channel busy after %ldms --> Skip sending\n", duration);
#elif defined ESP32
		Serial.printf("RAK11200 CAD returned channel busy after %ldms --> Skip sending\n", duration);
#else // RAK11310
		Serial.printf("RAK11310 CAD returned channel busy after %ldms --> Skip sending\n", duration);
#endif
		digitalWrite(LED_BLUE, LOW);
	}
	else
	{
#ifdef NRF52_SERIES
		Serial.printf("RAK4630 Channel available after %ldms --> send now\n", duration);
#elif defined ESP32
		Serial.printf("RAK11200 Channel available after %ldms --> send now\n", duration);
#else // RAK11310
		Serial.printf("RAK11310 Channel available after %ldms --> send now\n", duration);
#endif
		Radio.Send((uint8_t *)tx_payload, 9);
	}
}

/**
 * @brief Timer callback
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Only set a flag here for the loop().
 * DO NOT TRY TO DO ANYTHING FROM HERE.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 */
void tx_lora_periodic_handler(void)
{
	send_now = true;
	digitalWrite(LED_GREEN, LOW);
}