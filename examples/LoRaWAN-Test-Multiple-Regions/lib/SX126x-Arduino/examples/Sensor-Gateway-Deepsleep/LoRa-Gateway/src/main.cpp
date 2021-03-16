#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Ticker.h>
#include <rom/rtc.h>
#include <driver/rtc_io.h>
#include <SX126x-Arduino.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

// Set to 1 to stop any output on Serial
#define BATT_SAVE_ON 0

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
int PIN_LORA_SCLK = 5;
/** LORA SPI MISO */
int PIN_LORA_MISO = 19;
/** LORA SPI MOSI */
int PIN_LORA_MOSI = 18;

// Define LoRa parameters
#define RF_FREQUENCY 915600000  // Hz
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

#define BUFFER_SIZE 512 // Define the payload size here

/** Lora events */
static RadioEvents_t RadioEvents;

// Event declarations
/** LoRa receive success */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
/** LoRa receive timeout */
void OnRxTimeout(void);
/** LoRa receive error */
void OnRxError(void);

/* The Data message that will be received from the sensor nodes */
struct dataMsg
{
	uint8_t startMark[4] = {0xAA, 0x55, 0x00, 0x00};
	uint32_t nodeId;
	int8_t tempInt;
	uint8_t tempFrac;
	uint8_t humidInt;
	uint8_t humidFrac;
	uint8_t endMark[4] = {0x00, 0x00, 0x55, 0xAA};
} dataMsg;

// Adafruit IoT stuff
/** Username */
#define AIO_USERNAME "<PUT_YOUR_AIO_USERNAME_HERE>"
/** Active key */
#define AIO_KEY " < PUT_YOUR_AIO_KEY_HERE > "
/** Server URL */
#define AIO_SERVER "io.adafruit.com"
/** Server port */
#define AIO_SERVERPORT 1883 // use 8883 for SSL

// PUT YOUR OWN WIFI AP NAME AND PASSWORD HERE
/** WiFi AP name */
#define ssid "MyWiFi"
/** WiFi password */
#define pass "qwerty123"

/** WiFi client to connect to IoT service */
WiFiClient client;
/** MQTT client to connect to io.adafruit.com */
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/** Feed for temperature of sensor 0 */
Adafruit_MQTT_Publish temp_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/t_node_0");
/** Feed for humidity of sensor 0 */
Adafruit_MQTT_Publish humid_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/h_node_0");

/** Timeout for WiFi and Adafruit connection */
time_t adafruitTimeout;

/** Sleep function */
void goToSleep(void);

/**
 * ESP32 startup
 */
void setup()
{
#if BATT_SAVE_ON == 0
	// Start serial communication
	Serial.begin(115200);
#endif

	// Check the reasons the CPU's are started. We need this later to decide if a full initialization
	// of the SX126x is required or if the chip just woke up triggered by an interrupt from the SX126x
	RESET_REASON cpu0WakeupReason = rtc_get_reset_reason(0);
	RESET_REASON cpu1WakeupReason = rtc_get_reset_reason(1);

	// Show we are awake
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	/** LoRa HW configuration */
	hw_config hwConfig;

	// Define the HW configuration between MCU and SX126x
	hwConfig.CHIP_TYPE = SX1262_CHIP;		  // Example uses an eByte E22 module with an SX1262
	hwConfig.PIN_LORA_RESET = PIN_LORA_RESET; // LORA RESET
	hwConfig.PIN_LORA_NSS = PIN_LORA_NSS;	 // LORA SPI CS
	hwConfig.PIN_LORA_SCLK = PIN_LORA_SCLK;   // LORA SPI CLK
	hwConfig.PIN_LORA_MISO = PIN_LORA_MISO;   // LORA SPI MISO
	hwConfig.PIN_LORA_DIO_1 = PIN_LORA_DIO_1; // LORA DIO_1
	hwConfig.PIN_LORA_BUSY = PIN_LORA_BUSY;   // LORA SPI BUSY
	hwConfig.PIN_LORA_MOSI = PIN_LORA_MOSI;   // LORA SPI MOSI
	hwConfig.RADIO_TXEN = -1;				  // LORA ANTENNA TX ENABLE
	hwConfig.RADIO_RXEN = -1;				  // LORA ANTENNA RX ENABLE
	hwConfig.USE_DIO2_ANT_SWITCH = true;	  // Example uses an CircuitRocks Alora RFM1262 which uses DIO2 pins as antenna control
	hwConfig.USE_DIO3_TCXO = true;			  // Example uses an CircuitRocks Alora RFM1262 which uses DIO3 to control oscillator voltage
	hwConfig.USE_DIO3_ANT_SWITCH = false;	 // Only Insight ISP4520 module uses DIO3 as antenna control

	// Slowing down the ESP32 to 1/4 of its speed saves more energy
	setCpuFrequencyMhz(80);

	// Initialize the LoRa chip
	if ((cpu0WakeupReason == DEEPSLEEP_RESET) || (cpu1WakeupReason == DEEPSLEEP_RESET))
	{
		// Wake up reason was a DEEPSLEEP_RESET, which means we were woke up by the SX126x
#if BATT_SAVE_ON == 0
		Serial.println("Starting lora_hardware_re_init");
#endif
		lora_hardware_re_init(hwConfig);
	}
	else
	{
		// Other wake up reasons mean we need to do a complete initialization of the SX126x
#if BATT_SAVE_ON == 0
		Serial.println("Starting lora_hardware_init");
#endif
		lora_hardware_init(hwConfig);
	}

	// Initialize the Radio callbacks we need to receive data
	RadioEvents.RxDone = OnRxDone;
	RadioEvents.RxTimeout = OnRxTimeout;
	RadioEvents.RxError = OnRxError;

	// Check the wakeup reason
	if ((cpu0WakeupReason == DEEPSLEEP_RESET) || (cpu1WakeupReason == DEEPSLEEP_RESET))
	{
		// Wake up reason was a DEEPSLEEP_RESET, just re-initialize the callbacks
#if BATT_SAVE_ON == 0
		Serial.println("Trying to handle SX1262 event after deep sleep wakeup");
#endif
		// Re-initialize the Radio
		Radio.ReInit(&RadioEvents);

		// Initialize WiFi connection
		WiFi.begin(ssid, pass);

		// Handle LoRa events to find out what the reason for the wake up was and handle the data
		Radio.IrqProcessAfterDeepSleep();
	}
	else
	{
#if BATT_SAVE_ON == 0
		Serial.println("Power on reset, reinitialize the Radio");
#endif
		// Other wake up reasons mean we need to do a complete initialization of the SX126x
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
	}

	// goToSleep will start listening to LoRa data packages and put the ESP32 into deep sleep
	goToSleep();
}

/**
 * ESP32 main task
 */
void loop()
{
}

/**
 * Put SX126x into RX mode and ESP32 into deep-sleep mode
 */
void goToSleep(void)
{
	// Start waiting for data package
	Radio.Standby();
	SX126xSetDioIrqParams(IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT,
						  IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT,
						  IRQ_RADIO_NONE, IRQ_RADIO_NONE);
	// To get maximum power savings we use Radio.SetRxDutyCycle instead of Radio.Rx(0)
	// This function keeps the SX1261/2 chip most of the time in sleep and only wakes up short times
	// to catch incoming data packages
	Radio.SetRxDutyCycle(2 * 1024 * 1000 * 15.625, 10 * 1024 * 1000 * 15.625);

	// Go back to bed
#if BATT_SAVE_ON == 0
	Serial.println("Start sleeping");
#endif
	// Make sure the DIO1, RESET and NSS GPIOs are hold on required levels during deep sleep
	rtc_gpio_set_direction((gpio_num_t)PIN_LORA_DIO_1, RTC_GPIO_MODE_INPUT_ONLY);
	rtc_gpio_pulldown_en((gpio_num_t)PIN_LORA_DIO_1);
	rtc_gpio_set_direction((gpio_num_t)PIN_LORA_RESET, RTC_GPIO_MODE_OUTPUT_ONLY);
	rtc_gpio_set_level((gpio_num_t)PIN_LORA_RESET, HIGH);
	rtc_gpio_set_direction((gpio_num_t)PIN_LORA_NSS, RTC_GPIO_MODE_OUTPUT_ONLY);
	rtc_gpio_set_level((gpio_num_t)PIN_LORA_NSS, HIGH);
	// Setup deep sleep with wakeup by external source
	esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_LORA_DIO_1, RISING);
	// Finally set ESP32 into sleep
	esp_deep_sleep_start();
}

/**@brief Function to be executed on Radio Rx Done event
 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
#if BATT_SAVE_ON == 0
	Serial.println("Receive finished");

	char debugOutput[1024];
	for (int idx = 0; idx < size; idx++)
	{
		sprintf(&debugOutput[(idx * 2)], "%02X", payload[idx]);
	}
	Serial.printf("Data: %s\n", debugOutput);
#endif

	// Save received message in a buffer
	memcpy((void *)&dataMsg, payload, size);

	// Check if it is a valid message
	char startMark[4] = {0xAA, 0x55, 0x00, 0x00};
	char endMark[4] = {0x00, 0x00, 0x55, 0xAA};
	if ((memcmp((void *)dataMsg.startMark, startMark, 4) != 0) || (memcmp((void *)dataMsg.endMark, endMark, 4) != 0))
	{
#if BATT_SAVE_ON == 0
		Serial.println("Data is not for us");
		goToSleep();
#endif
	}

	// Wait for WiFi connection
	while (!WiFi.isConnected())
	{
		if ((millis() - adafruitTimeout) > 60000)
		{
#if BATT_SAVE_ON == 0
			Serial.println("Timeout connecting WiFi");
#endif
			goToSleep();
		}
	}

	int8_t ret;

	// Connect to Adafruit IO
	while ((ret = mqtt.connect()) != 0)
	{
		if ((millis() - adafruitTimeout) > 60000)
		{
#if BATT_SAVE_ON == 0
			Serial.println("Timeout connecting to Adafruit IO");
#endif
			goToSleep();
		}
	}

	// Topic for publishing
	char devTopic[256];
	// Data for publishing
	char devValue[128];
	// Publish Temperature from received message
	sprintf(devTopic, "%s/feeds/%08X_t",
			AIO_USERNAME,
			dataMsg.nodeId);
	sprintf(devValue, "%.2f", ((float)dataMsg.tempFrac / 100.0) + (float)dataMsg.tempInt);
#if BATT_SAVE_ON == 0
	Serial.printf("Publishing topic %s with value %s\n", devTopic, devValue);
#endif
	mqtt.publish(devTopic, devValue);
	
	// Publish Humidity from received message
	sprintf(devTopic, "%s/feeds/%08X_h",
			AIO_USERNAME,
			dataMsg.nodeId);
	sprintf(devValue, "%.2f", ((float)dataMsg.humidFrac / 100.0) + (float)dataMsg.humidInt);
#if BATT_SAVE_ON == 0
	Serial.printf("Publishing topic %s with value %s\n", devTopic, devValue);
#endif
	mqtt.publish(devTopic, devValue);

	// Cleanup
	mqtt.disconnect();
	WiFi.disconnect(true);

	// LoRa receive finished, go back to bed
	goToSleep();
}

/**@brief Function to be executed on Radio Rx Timeout event
 */
void OnRxTimeout(void)
{
#if BATT_SAVE_ON == 0
	Serial.println("Receive timeout");
#endif
	// LoRa RX failed, go back to bed
	goToSleep();
}

/**@brief Function to be executed on Radio Rx Error event
 */
void OnRxError(void)
{
#if BATT_SAVE_ON == 0
	Serial.println("Receive error");
#endif
	// LoRa RX failed, go back to bed
	goToSleep();
}
