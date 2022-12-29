#include <Arduino.h>
#include <LoRaWan-Arduino.h>
#include <SPI.h>

// LoRaWan setup definitions
#define SCHED_MAX_EVENT_DATA_SIZE APP_TIMER_SCHED_EVENT_DATA_SIZE // Maximum size of scheduler events
#define SCHED_QUEUE_SIZE 60	// Maximum number of events in the scheduler queue

/**< Maximum number of events in the scheduler queue  */
#define LORAWAN_APP_DATA_BUFF_SIZE 256 // Size of the data to be transmitted
#define LORAWAN_APP_TX_DUTYCYCLE 5000 // Defines the application data transmission duty cycle. 30s, value in [ms]
#define APP_TX_DUTYCYCLE_RND 1000 // Defines a random delay for application data transmission duty cycle. 1s, value in [ms]
#define JOINREQ_NBTRIALS 3	

bool doOTAA = true;
hw_config hwConfig;

#ifdef ESP32
// ESP32 - SX126x pin configuration
int PIN_LORA_RESET = 4;	 // LORA RESET
int PIN_LORA_NSS = 5;	 // LORA SPI CS
int PIN_LORA_SCLK = 18;	 // LORA SPI CLK
int PIN_LORA_MISO = 19;	 // LORA SPI MISO
int PIN_LORA_MOSI = 23;	 // LORA SPI MOSI
int PIN_LORA_BUSY = 22;	 // LORA SPI BUSY
int PIN_LORA_DIO_1 = 21; // LORA DIO_1
int RADIO_TXEN = 26;	 // LORA ANTENNA TX ENABLE
int RADIO_RXEN = 27;	 // LORA ANTENNA RX ENABLE
#endif

// Define Helium or TTN OTAA keys. All msb (big endian).
uint8_t nodeDeviceEUI[8] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60};
uint8_t nodeAppEUI[8] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60};
uint8_t nodeAppKey[16] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x0F, 0xFD, 0x29, 0x8C, 0x51, 0x85, 0xF1, 0xB4};

// Foward declaration
/** LoRaWAN callback when join network finished */
static void lorawan_has_joined_handler(void);
/** LoRaWAN callback when join network failed */
static void lorawan_join_fail_handler(void);
/** LoRaWAN callback when data arrived */
static void lorawan_rx_handler(lmh_app_data_t *app_data);
/** LoRaWAN callback after class change request finished */
static void lorawan_confirm_class_handler(DeviceClass_t Class);
/** LoRaWAN callback after class change request finished */
static void lorawan_unconfirm_tx_finished(void);
/** LoRaWAN callback after class change request finished */
static void lorawan_confirm_tx_finished(bool result);
/** LoRaWAN Function to send a package */
static void send_lora_frame(void);
static uint32_t timers_init(void);

// APP_TIMER_DEF(lora_tx_timer_id);	 // LoRa tranfer timer instance.
TimerEvent_t appTimer;	 // LoRa tranfer timer instance.
static uint8_t m_lora_app_data_buffer[LORAWAN_APP_DATA_BUFF_SIZE]; // Lora user application data buffer.
static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0};	 // Lora user application data structure.

/**@brief Structure containing LoRaWan parameters, needed for lmh_init()
 */
static lmh_param_t lora_param_init = {LORAWAN_ADR_OFF, DR_3, LORAWAN_PUBLIC_NETWORK,
										JOINREQ_NBTRIALS, LORAWAN_DEFAULT_TX_POWER, LORAWAN_DUTYCYCLE_OFF};

static lmh_callback_t lora_callbacks = {BoardGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
										lorawan_rx_handler, lorawan_has_joined_handler, 
										lorawan_confirm_class_handler, lorawan_join_fail_handler,
										lorawan_unconfirm_tx_finished, lorawan_confirm_tx_finished};


// Check if the board has an LED port defined
#define LED_ON HIGH
#define LED_OFF LOW
#ifndef LED_BUILTIN
#ifdef ESP32
#define LED_BUILTIN 2
#endif
#endif
	
void ledOff(void)
{
	digitalWrite(LED_BUILTIN, LED_OFF);
}

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	// Define the HW configuration between MCU and SX126x
	hwConfig.CHIP_TYPE = SX1262_CHIP;	// Example uses an eByte E22 module with an SX1262
	hwConfig.PIN_LORA_RESET = PIN_LORA_RESET; // LORA RESET
	hwConfig.PIN_LORA_NSS = PIN_LORA_NSS;	  // LORA SPI CS
	hwConfig.PIN_LORA_SCLK = PIN_LORA_SCLK;	  // LORA SPI CLK
	hwConfig.PIN_LORA_MISO = PIN_LORA_MISO;	  // LORA SPI MISO
	hwConfig.PIN_LORA_DIO_1 = PIN_LORA_DIO_1; // LORA DIO_1
	hwConfig.PIN_LORA_BUSY = PIN_LORA_BUSY;	  // LORA SPI BUSY
	hwConfig.PIN_LORA_MOSI = PIN_LORA_MOSI;	  // LORA SPI MOSI
	hwConfig.RADIO_TXEN = RADIO_TXEN;	// LORA ANTENNA TX ENABLE (e.g. eByte E22 module)
	hwConfig.RADIO_RXEN = RADIO_RXEN;	// LORA ANTENNA RX ENABLE (e.g. eByte E22 module)
	hwConfig.USE_DIO2_ANT_SWITCH = false;	// LORA DIO2 does not control antenna
	hwConfig.USE_DIO3_TCXO = true;	// LORA DIO3 controls oscillator voltage (e.g. eByte E22 module)
	hwConfig.USE_DIO3_ANT_SWITCH = false;	// LORA DIO3 does not control antenna


	// Initialize Serial for debug output
	Serial.begin(115200);

	Serial.println("=====================================");
	Serial.println("SX126x LoRaWan test");
	Serial.println("=====================================");

	// Initialize LoRa chip.
	uint32_t err_code = lora_hardware_init(hwConfig);
	if (err_code != 0)
	{
		Serial.printf("lora_hardware_init failed - %d\n", err_code);
	}

	// Initialize Scheduler and timer (Must be after lora_hardware_init)
	err_code = timers_init();
	if (err_code != 0)
	{
		Serial.printf("timers_init failed - %d\n", err_code);
	}

	// Setup the EUIs and Keys
	lmh_setDevEui(nodeDeviceEUI);
	lmh_setAppEui(nodeAppEUI);
	lmh_setAppKey(nodeAppKey);

	// Initialize LoRaWan
	// CLASS C works for esp32 and e22, US915 region works in america, other local frequencies can be found 
	// here https://docs.helium.com/lorawan-on-helium/frequency-plans/
	
	err_code = lmh_init(&lora_callbacks, lora_param_init, doOTAA, CLASS_C, LORAMAC_REGION_US915);
	if (err_code != 0)
	{
		Serial.printf("lmh_init failed - %d\n", err_code);
	}

	// For Helium and US915, you need as well to select subband 2 after you called lmh_init(), 
	// For US816 you need to use subband 1. Other subbands configurations can be found in
	// https://github.com/beegee-tokyo/SX126x-Arduino/blob/1c28c6e769cca2b7d699a773e737123fc74c47c7/src/mac/LoRaMacHelper.cpp

	lmh_setSubBandChannels(2);

	// Start Join procedure
	lmh_join();
}

void loop()
{
}

static void lorawan_join_fail_handler(void)
{
	Serial.println("OTAA joined failed");
	Serial.println("Check LPWAN credentials and if a gateway is in range");
	// Restart Join procedure
	Serial.println("Restart network join request");
}

/**@brief LoRa function for handling HasJoined event.
 */
static void lorawan_has_joined_handler(void)
{
#if (OVER_THE_AIR_ACTIVATION != 0)
	Serial.println("Network Joined");
#else
	Serial.println("OVER_THE_AIR_ACTIVATION != 0");

#endif
	lmh_class_request(CLASS_A);

	TimerSetValue(&appTimer, LORAWAN_APP_TX_DUTYCYCLE);
	TimerStart(&appTimer);
	// app_timer_start(lora_tx_timer_id, APP_TIMER_TICKS(LORAWAN_APP_TX_DUTYCYCLE), NULL);
	Serial.println("Sending frame");
	send_lora_frame();
}

/**@brief Function for handling LoRaWan received data from Gateway
 *
 * @param[in] app_data  Pointer to rx data
 */
static void lorawan_rx_handler(lmh_app_data_t *app_data)
{
	Serial.printf("LoRa Packet received on port %d, size:%d, rssi:%d, snr:%d\n",
				  app_data->port, app_data->buffsize, app_data->rssi, app_data->snr);

	for (int i = 0; i < app_data->buffsize; i++)
	{
		Serial.printf("%0X ", app_data->buffer[i]);
	}
	Serial.println("");

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
}

/**@brief Function to confirm LORaWan class switch.
 *
 * @param[in] Class  New device class
 */
static void lorawan_confirm_class_handler(DeviceClass_t Class)
{
	Serial.printf("switch to class %c done\n", "ABC"[Class]);

	// Informs the server that switch has occurred ASAP
	m_lora_app_data.buffsize = 0;
	m_lora_app_data.port = LORAWAN_APP_PORT;
	lmh_send(&m_lora_app_data, LMH_UNCONFIRMED_MSG);
}

/**
 * @brief Called after unconfirmed packet was sent
 * 
 */
static void lorawan_unconfirm_tx_finished(void)
{
	Serial.println("Uncomfirmed TX finished");
}

/**
 * @brief Called after confirmed packet was sent
 * @param result Result of sending true = ACK received false = No ACK
 */
static void lorawan_confirm_tx_finished(bool result)
{
	Serial.printf("Comfirmed TX finished with result %s", result ? "ACK" : "NAK");
}

/**@brief Function for sending a LoRa package.
 */
static void send_lora_frame(void)
{
	
	// Building the message to send
	int32_t chipTemp = 0;
	uint32_t i = 0;
	Ticker ledTicker;

	if (lmh_join_status_get() != LMH_SET)
	{
		// Not joined, try again later
		Serial.println("Did not join network, skip sending frame");
		return;
	}

	// Building the message to send

	char t100 = (char)(chipTemp / 100);
	char t10 = (char)((chipTemp - (t100 * 100)) / 10);
	char t1 = (char)((chipTemp - (t100 * 100) - (t10 * 10)) / 1);

	// Buffer contruction
	m_lora_app_data.port = LORAWAN_APP_PORT;

	m_lora_app_data.buffer[i++] = '{';
	m_lora_app_data.buffer[i++] = '"';
	m_lora_app_data.buffer[i++] = 'i';
	m_lora_app_data.buffer[i++] = '"';
	m_lora_app_data.buffer[i++] = ':';
	m_lora_app_data.buffer[i++] = ',';
	m_lora_app_data.buffer[i++] = '"';
	m_lora_app_data.buffer[i++] = 'n';
	m_lora_app_data.buffer[i++] = '"';
	m_lora_app_data.buffer[i++] = ':';

	m_lora_app_data.buffer[i++] = t100 + 0x30;
	m_lora_app_data.buffer[i++] = t10 + 0x30;
	m_lora_app_data.buffer[i++] = t1 + 0x30;
	m_lora_app_data.buffer[i++] = '}';
	m_lora_app_data.buffsize = i;

	Serial.print("Data: ");
	Serial.println((char *)m_lora_app_data.buffer);
	Serial.print("Size: ");
	Serial.println(m_lora_app_data.buffsize);
	Serial.print("Port: ");
	Serial.println(m_lora_app_data.port);

	chipTemp += 1;
	if (chipTemp >= 999)
		chipTemp = 0;

	lmh_error_status error = lmh_send(&m_lora_app_data, LMH_UNCONFIRMED_MSG);
	if (error == LMH_SUCCESS)
	{
	}

	Serial.printf("lmh_send result %d\n", error);
	digitalWrite(LED_BUILTIN, LED_ON);
	ledTicker.once(1, ledOff);
}

/**@brief Function for handling a LoRa tx timer timeout event.
 */
static void tx_lora_periodic_handler(void)
{
	TimerSetValue(&appTimer, LORAWAN_APP_TX_DUTYCYCLE);
	TimerStart(&appTimer);
	Serial.println("Sending frame");
	send_lora_frame();
}

static uint32_t timers_init(void)
{
	appTimer.timerNum = 3;
	TimerInit(&appTimer, tx_lora_periodic_handler);
	return 0;
}
