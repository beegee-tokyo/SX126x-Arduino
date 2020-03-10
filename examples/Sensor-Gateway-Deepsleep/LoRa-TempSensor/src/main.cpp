#include <Arduino.h>
#include <SX126x-Arduino.h>
#include <SPI.h>
#include "Adafruit_Si7021.h"

// Set to 1 to stop any output on Serial
#define BATT_SAVE_ON 0

/** Number of retries if CAD shows busy */
#define CAD_RETRY 20

// LoRa definitions
#define RF_FREQUENCY 915600000  // Hz
#define TX_OUTPUT_POWER 22		// dBm
#define LORA_BANDWIDTH 0		// [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12] [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_CODINGRATE 1		// [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 5000
#define TX_TIMEOUT_VALUE 5000

// ESP32 Feather - Alora RFM1262 pin configuration
/** LORA RESET */
int PIN_LORA_RESET = 32;
/** LORA DIO_1 */
int PIN_LORA_DIO_1 = 14;
/** LORA SPI BUSY */
int PIN_LORA_BUSY = 27;
/** LORA SPI CS */
int PIN_LORA_NSS = 33;
/** LORA SPI CLK */
int PIN_LORA_SCLK = SCK;
/** LORA SPI MISO */
int PIN_LORA_MISO = MISO;
/** LORA SPI MOSI */
int PIN_LORA_MOSI = MOSI;
/** LORA ANTENNA TX ENABLE */
int RADIO_TXEN = -1;
/** LORA ANTENNA RX ENABLE */
int RADIO_RXEN = -1;

/** LoRa callback events */
static RadioEvents_t RadioEvents;

// LoRa callback functions
/** LoRa transmit success */
void OnTxDone(void);
/** LoRa transmit timeout */
void OnTxTimeout(void);
/** LoRa CAD finished */
void OnCadDone(bool cadResult);

/** The Data message will be sent by the node */
struct dataMsg
{
	uint8_t startMark[4] = {0xAA, 0x55, 0x00, 0x00};
	uint32_t nodeId;
	int8_t tempInt;
	uint8_t tempFrac;
	uint8_t humidInt;
	uint8_t humidFrac;
	uint8_t endMark[4] = { 0x00, 0x00, 0x55, 0xAA};
} dataMsg;

/** The sensor node ID, created from MAC of ESP32 */
uint32_t deviceID;

/** LoRa error timeout */
time_t loraTimeout;

/** CAD repeat counter */
uint8_t cadRepeat;

/** SI 7021 sensor */
Adafruit_Si7021 sensor = Adafruit_Si7021();

/** Conversion factor for micro seconds to seconds */
#define uS_TO_mS_FACTOR 1000
/** Time ESP32 will sleep (in milliseconds) */
long sleepForMillis = 30000;

/** The wakeup time */
time_t wakeup;

/** Sleep function */
void goToSleep(void);

void setup()
{
	// Record the wakeup time
	wakeup = millis();

	// Slowing down the ESP32 to 1/4 of its speed saves more energy
	setCpuFrequencyMhz(80);

	// Show we are awake
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	/** HW configuration structure for the LoRa library */
	hw_config hwConfig;

#if BATT_SAVE_ON == 0
	// Start Serial
	Serial.begin(115200);
#endif

	// Create node ID
	uint8_t deviceMac[8];

	BoardGetUniqueId(deviceMac);

	deviceID += (uint32_t)deviceMac[2];
	deviceID += (uint32_t)deviceMac[3] << 8;
	deviceID += (uint32_t)deviceMac[4] << 16;
	deviceID += (uint32_t)deviceMac[5] << 24;

#if BATT_SAVE_ON == 0
	Serial.println("++++++++++++++++++++++++++++++++++++++");
	Serial.printf("Sensor node ID %08X using frequency %.1f MHz\n", deviceID, (double)(RF_FREQUENCY/1000000.0));
	Serial.println("++++++++++++++++++++++++++++++++++++++");
#endif

	// Define the HW configuration between MCU and SX126x
	hwConfig.CHIP_TYPE = SX1262_CHIP;		  // eByte E22 module with an SX1262
	hwConfig.PIN_LORA_RESET = PIN_LORA_RESET; // LORA RESET
	hwConfig.PIN_LORA_NSS = PIN_LORA_NSS;	 // LORA SPI CS
	hwConfig.PIN_LORA_SCLK = PIN_LORA_SCLK;   // LORA SPI CLK
	hwConfig.PIN_LORA_MISO = PIN_LORA_MISO;   // LORA SPI MISO
	hwConfig.PIN_LORA_DIO_1 = PIN_LORA_DIO_1; // LORA DIO_1
	hwConfig.PIN_LORA_BUSY = PIN_LORA_BUSY;   // LORA SPI BUSY
	hwConfig.PIN_LORA_MOSI = PIN_LORA_MOSI;   // LORA SPI MOSI
	hwConfig.RADIO_TXEN = RADIO_TXEN;		  // LORA ANTENNA TX ENABLE
	hwConfig.RADIO_RXEN = RADIO_RXEN;		  // LORA ANTENNA RX ENABLE
	hwConfig.USE_DIO2_ANT_SWITCH = true;	  // Example uses an eByte E22 module which uses RXEN and TXEN pins as antenna control
	hwConfig.USE_DIO3_TCXO = true;			  // Example uses an eByte E22 module which uses DIO3 to control oscillator voltage
	hwConfig.USE_DIO3_ANT_SWITCH = false;	 // Only Insight ISP4520 module uses DIO3 as antenna control

	if (lora_hardware_init(hwConfig) != 0)
	{
#if BATT_SAVE_ON == 0
		Serial.println("Error in hardware init");
#endif
	}

	// Initialize the callbacks we need for sending
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.CadDone = OnCadDone;
	Radio.Init(&RadioEvents);

	// Put LoRa into standby
	Radio.Standby();

	// Set Frequency
	Radio.SetChannel(RF_FREQUENCY);

	// Set transmit configuration
	Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
					  LORA_SPREADING_FACTOR, LORA_CODINGRATE,
					  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
					  true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

	// Set receive configuration
	Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
					  LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
					  LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
					  0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

	// Initialize SI7021 temperature sensor
	if (!sensor.begin())
	{
#if BATT_SAVE_ON == 0
		Serial.println("Did not find Si7021 sensor!");
#endif
		goToSleep();
	}

	// Get the sensor values
	float sensTemp = sensor.readTemperature();
	float sensHumid = sensor.readHumidity();
	if ((sensTemp == NAN) || (sensHumid == NAN) || (sensTemp == 255.255) || (sensHumid == 255.255))
	{
		// Error reading the sensor, try one more time
		float sensTemp = sensor.readTemperature();
		float sensHumid = sensor.readHumidity();
		if ((sensTemp == NAN) || (sensHumid == NAN))
		{
			// Second reading failed as well, give up
#if BATT_SAVE_ON == 0
			Serial.println("Could not read sensor data, skip sending");
#endif
			goToSleep();
		}
	}

	// Prepare data package
	dataMsg.nodeId = deviceID;
	dataMsg.tempInt = (int8_t) sensTemp;
	dataMsg.tempFrac = (uint8_t)((sensTemp - dataMsg.tempInt) * 100);
	dataMsg.humidInt = (uint8_t)sensHumid;
	dataMsg.humidFrac = (uint8_t)((sensHumid - dataMsg.humidInt) * 100);

#if BATT_SAVE_ON == 0
	Serial.printf("Finished reading sensor after %ldms\n", (millis() - wakeup));

	Serial.print("Temp ");
	Serial.print(dataMsg.tempInt);
	Serial.print(".");
	Serial.println(dataMsg.tempFrac);

	Serial.print("Humid ");
	Serial.print(dataMsg.humidInt);
	Serial.print(".");
	Serial.println(dataMsg.humidFrac);

	Serial.println("Data package as HEX values:");
	char *printData = (char *) &dataMsg;
	for (int idx=0; idx < sizeof(dataMsg); idx++)
	{
		Serial.printf("%02X ", printData[idx]);
	}
#endif

	// Start sending
	Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);

	// Counter for repeated CAD in case we have many traffic
	cadRepeat = 0;

	Radio.StartCad();

	// Make sure we detect a timeout during sending
	loraTimeout = millis();
}

void loop()
{
	// Handle LoRa events
	Radio.IrqProcess();

	// Check for LoRa timeout
	if ((millis() - loraTimeout) > 120000)
	{
#if BATT_SAVE_ON == 0
		Serial.println("LoRa loop timeout");
#endif
		// LoRa failed, go back to bed
		goToSleep();
	}
	delay(100);
}

/**
 * Put sensor mode into deep-sleep mode
 */
void goToSleep(void)
{
	// Send the LoRa module to sleep
	Radio.Standby();
	Radio.Sleep();
#if BATT_SAVE_ON == 0
	Serial.printf("Sleeping after %ldms\n", (millis() - wakeup));
#endif 

	// Go back to bed for 
	time_t awakeTime = millis() - wakeup;

	esp_sleep_enable_timer_wakeup((sleepForMillis - awakeTime) * uS_TO_mS_FACTOR);
	esp_deep_sleep_start();
}

/**@brief Function to be executed on Radio Tx Done event
 */
void OnTxDone(void)
{
#if BATT_SAVE_ON == 0
	Serial.println("Transmit finished");
#endif
	// LoRa failed, go back to bed
	goToSleep();
}

/**@brief Function to be executed on Radio Tx Timeout event
 */
void OnTxTimeout(void)
{
#if BATT_SAVE_ON == 0
	Serial.println("Transmit timeout");
#endif
	// LoRa failed, go back to bed
	goToSleep();
}

/**@brief Function to be executed on Radio Rx Error event
 */
void OnCadDone(bool cadResult)
{
	cadRepeat++;
	Radio.Standby();
	if (cadResult)
	{
#if BATT_SAVE_ON == 0
		Serial.printf("CAD returned channel busy %d times\n", cadRepeat);
#endif
		if (cadRepeat < 6)
		{
			// Retry CAD
			Radio.Standby();
			Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
			Radio.StartCad();
		}
		else
		{
			// LoRa is too busy? Go to bed
			goToSleep();
		}
	}
	else
	{
#if BATT_SAVE_ON == 0
		Serial.printf("CAD returned channel free after %d times\n", cadRepeat);
#endif
		// Send data
		Radio.Send((uint8_t *) &dataMsg, sizeof(dataMsg));
	}
}
