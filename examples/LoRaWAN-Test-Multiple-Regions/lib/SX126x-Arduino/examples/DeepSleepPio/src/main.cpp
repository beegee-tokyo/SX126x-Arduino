#include <Arduino.h>
#include <SPI.h>
#include <Ticker.h>
#include <rom/rtc.h>
#include <driver/rtc_io.h>
#include <SX126x-Arduino.h>

#define LOG_ON

// ESP32 Feather - SX126x pin configuration
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

/** LED on level */
#define LED_ON HIGH
/** LED off level */
#define LED_OFF LOW

// Define LoRa parameters
#define RF_FREQUENCY 915000000  // Hz
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

/** LoRa HW configuration */
hw_config hwConfig;

/** CAD repeat counter */
uint8_t cadRepeat;

// Event declarations
/** LoRa transmit success */
void OnTxDone(void);
/** LoRa receive success */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
/** LoRa transmit timeout */
void OnTxTimeout(void);
/** LoRa receive timeout */
void OnRxTimeout(void);
/** LoRa receive error */
void OnRxError(void);
/** LoRa CAD finished */
void OnCadDone(bool cadResult);

/** Print reset reason */
void print_reset_reason(RESET_REASON reason);

/** Node ID */
uint8_t deviceId[8];

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
	Radio.SetRxDutyCycle(2 * 1024 * 1000 * 15.625, 10 * 1024 * 15.625);

	// Go back to bed
#ifdef LOG_ON
	Serial.println("Start sleeping");
#endif
	// Make sure the DIO1, RESET and NSS GPIOs are hold on required levels during deep sleep
	rtc_gpio_pulldown_en((gpio_num_t)PIN_LORA_DIO_1);
	rtc_gpio_pullup_en((gpio_num_t)PIN_LORA_RESET);
	rtc_gpio_pullup_en((gpio_num_t)PIN_LORA_NSS);
	// Setup deep sleep with wakeup by external source
	esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_LORA_DIO_1, RISING);
	// Finally set ESp32 into sleep
	esp_deep_sleep_start();
}

/**
 * ESP32 startup
 */
void setup()
{
#ifdef LOG_ON
	// Start serial communication
	Serial.begin(115200);
#endif

	// Check the reasons the CPU's are started. We need this later to decide if a full initialization
	// of the SX126x is required or if the chip just woke up triggered by an interrupt from the SX126x
	RESET_REASON cpu0WakeupReason = rtc_get_reset_reason(0);
	RESET_REASON cpu1WakeupReason = rtc_get_reset_reason(1);

#ifdef LOG_ON
	Serial.println("CPU0 reset reason: ");
	print_reset_reason(cpu0WakeupReason);

	Serial.println("CPU1 reset reason: ");
	print_reset_reason(cpu1WakeupReason);
#endif

	// Show we are awake
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LED_ON);

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
#ifdef LOG_ON
		Serial.println("Starting lora_hardware_re_init");
#endif
		lora_hardware_re_init(hwConfig);
	}
	else
	{
		// Other wake up reasons mean we need to do a complete initialization of the SX126x
#ifdef LOG_ON
		Serial.println("Starting lora_hardware_init");
#endif
		lora_hardware_init(hwConfig);
	}

	// Initialize the Radio callbacks
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.RxDone = OnRxDone;
	RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.RxTimeout = OnRxTimeout;
	RadioEvents.RxError = OnRxError;
	RadioEvents.CadDone = OnCadDone;

	if ((cpu0WakeupReason == DEEPSLEEP_RESET) || (cpu1WakeupReason == DEEPSLEEP_RESET))
	{
		// Wake up reason was a DEEPSLEEP_RESET, just re-initialize the callbacks
#ifdef LOG_ON
		Serial.println("Trying to handle SX1262 event after deep sleep wakeup");
#endif
		// Initialize the Radio
		Radio.ReInit(&RadioEvents);

		// Handle LoRa events to find out what the reason for the wake up was and handle the data
		Radio.IrqProcessAfterDeepSleep();
	}
	else
	{
#ifdef LOG_ON
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

/**@brief Function to be executed on Radio Tx Done event
 */
void OnTxDone(void)
{
#ifdef LOG_ON
	Serial.println("Transmit finished");
#endif
	// LoRa TX finished, go back to bed
	goToSleep();
}

/**@brief Function to be executed on Radio Rx Done event
 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
#ifdef LOG_ON
	Serial.println("Receive finished");
#endif
	char debugOutput[1024];
	for (int idx = 0; idx < size; idx++)
	{
		sprintf(&debugOutput[(idx * 2)], "%02X", payload[idx]);
	}
	Serial.printf("Data: %s\n", debugOutput);

	// Now its up to YOU to do something with the data

	// LoRa receive finished, go back to bed
	goToSleep();
}

/**@brief Function to be executed on Radio Tx Timeout event
 */
void OnTxTimeout(void)
{
#ifdef LOG_ON
	Serial.println("Transmit timeout");
#endif
	// LoRa TX failed, go back to bed
	goToSleep();
}

/**@brief Function to be executed on Radio Rx Timeout event
 */
void OnRxTimeout(void)
{
#ifdef LOG_ON
	Serial.println("Receive timeout");
#endif
	// LoRa RX failed, go back to bed
	goToSleep();
}

/**@brief Function to be executed on Radio Rx Error event
 */
void OnRxError(void)
{
#ifdef LOG_ON
	Serial.println("Receive error");
#endif
	// LoRa RX failed, go back to bed
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
#ifdef LOG_ON
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
#ifdef LOG_ON
		Serial.printf("CAD returned channel free after %d times\n", cadRepeat);
#endif
		// If we need to send something, it should be done here
		// after CAD found the channel available
	}
}

/**
 * Print the reset reason.
 * Just for better understanding.
 * Not required for the function of the app
 */
void print_reset_reason(RESET_REASON reason)
{
	switch (reason)
	{
	case 1:
		Serial.println("POWERON_RESET");
		break; /**<1, Vbat power on reset*/
	case 3:
		Serial.println("SW_RESET");
		break; /**<3, Software reset digital core*/
	case 4:
		Serial.println("OWDT_RESET");
		break; /**<4, Legacy watch dog reset digital core*/
	case 5:
		Serial.println("DEEPSLEEP_RESET");
		break; /**<5, Deep Sleep reset digital core*/
	case 6:
		Serial.println("SDIO_RESET");
		break; /**<6, Reset by SLC module, reset digital core*/
	case 7:
		Serial.println("TG0WDT_SYS_RESET");
		break; /**<7, Timer Group0 Watch dog reset digital core*/
	case 8:
		Serial.println("TG1WDT_SYS_RESET");
		break; /**<8, Timer Group1 Watch dog reset digital core*/
	case 9:
		Serial.println("RTCWDT_SYS_RESET");
		break; /**<9, RTC Watch dog Reset digital core*/
	case 10:
		Serial.println("INTRUSION_RESET");
		break; /**<10, Instrusion tested to reset CPU*/
	case 11:
		Serial.println("TGWDT_CPU_RESET");
		break; /**<11, Time Group reset CPU*/
	case 12:
		Serial.println("SW_CPU_RESET");
		break; /**<12, Software reset CPU*/
	case 13:
		Serial.println("RTCWDT_CPU_RESET");
		break; /**<13, RTC Watch dog Reset CPU*/
	case 14:
		Serial.println("EXT_CPU_RESET");
		break; /**<14, for APP CPU, reseted by PRO CPU*/
	case 15:
		Serial.println("RTCWDT_BROWN_OUT_RESET");
		break; /**<15, Reset when the vdd voltage is not stable*/
	case 16:
		Serial.println("RTCWDT_RTC_RESET");
		break; /**<16, RTC Watch dog reset digital core and rtc module*/
	default:
		Serial.println("NO_MEAN");
	}
}
