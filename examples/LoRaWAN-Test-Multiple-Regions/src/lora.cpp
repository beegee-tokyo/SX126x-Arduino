/**
 * @file lora.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief LoRa initialization & handler
 * @version 0.1
 * @date 2021-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "main.h"

#ifdef _VARIANT_ISP4520_
/** DIO1 GPIO pin for ISP4520 */
#define PIN_LORA_DIO_1 11
#else
/** DIO1 GPIO pin for RAK4631 */
#define PIN_LORA_DIO_1 47
#endif

/** LoRaWAN setting from flash */
s_lorawan_settings g_lorawan_settings;

/** Semaphore used by SX126x IRQ handler to wake up LoRaWAN task */
static SemaphoreHandle_t lora_sem = NULL;

/** LoRa task handle */
TaskHandle_t loraTaskHandle;
/** GPS reading task */
void lora_task(void *pvParameters);

/** Buffer for received LoRaWan data */
uint8_t g_rx_lora_data[256];
/** Length of received data */
uint8_t g_rx_data_len = 0;
/** Buffer for received LoRaWan data */
uint8_t g_tx_lora_data[256];
/** Length of received data */
uint8_t g_tx_data_len = 0;

/** Flag if LoRaWAN is initialized and started */
bool g_lorawan_initialized = false;

/**************************************************************/
/* LoRaWAN properties                                            */
/**************************************************************/
/** LoRaWAN application data buffer. */
static uint8_t m_lora_app_data_buffer[256];
/** Lora application data structure. */
static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0};

// LoRaWAN event handlers
/** LoRaWAN callback when join network finished */
static void lpwan_joined_handler(void);
/** LoRaWAN callback when join network failed */
static void lpwan_join_fail_handler(void);
/** LoRaWAN callback when data arrived */
static void lpwan_rx_handler(lmh_app_data_t *app_data);
/** LoRaWAN callback after class change request finished */
static void lpwan_class_confirm_handler(DeviceClass_t Class);
/** LoRaWAN Function to send a package */
bool send_lpwan_packet(void);

/**@brief Structure containing LoRaWAN parameters, needed for lmh_init()
 * 
 * Set structure members to
 * LORAWAN_ADR_ON or LORAWAN_ADR_OFF to enable or disable adaptive data rate
 * LORAWAN_DEFAULT_DATARATE OR DR_0 ... DR_5 for default data rate or specific data rate selection
 * LORAWAN_PUBLIC_NETWORK or LORAWAN_PRIVATE_NETWORK to select the use of a public or private network
 * JOINREQ_NBTRIALS or a specific number to set the number of trials to join the network
 * LORAWAN_DEFAULT_TX_POWER or a specific number to set the TX power used
 * LORAWAN_DUTYCYCLE_ON or LORAWAN_DUTYCYCLE_OFF to enable or disable duty cycles
 *                   Please note that ETSI mandates duty cycled transmissions. 
 */
static lmh_param_t lora_param_init;

/** Structure containing LoRaWan callback functions, needed for lmh_init() */
static lmh_callback_t lora_callbacks = {BoardGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
										lpwan_rx_handler, lpwan_joined_handler,
										lpwan_class_confirm_handler, lpwan_join_fail_handler};

bool lpwan_has_joined = false;

String regionValues[] = {"AS923", "AU915", "CN470", "CN779", "EU433", "EU868", "KR920", "IN865", "US915", "US915H"};

/**
 * @brief SX126x interrupt handler
 * Called when DIO1 is set by SX126x
 * Gives lora_sem semaphore to wake up LoRaWan handler task
 * 
 */
void lora_interrupt_handler(void)
{
	// SX126x set IRQ
	if (lora_sem != NULL)
	{
		// Wake up LoRa task
		xSemaphoreGive(lora_sem);
	}
}

/**
 * @brief Initialize LoRa HW and LoRaWan MAC layer
 * 
 * @return int8_t result
 *  0 => OK
 * -1 => SX126x HW init failure
 * -2 => LoRaWan MAC initialization failure
 * -3 => Subband selection failure
 * -4 => LoRaWan handler task start failure
 */
int8_t init_lora(void)
{
	// Create the LoRaWan event semaphore
	lora_sem = xSemaphoreCreateBinary();
	// Initialize semaphore
	xSemaphoreGive(lora_sem);

	// Initialize LoRa chip.
#ifdef _VARIANT_ISP4520_
	if (lora_isp4520_init(SX1262) != 0)
#else
	if (lora_rak4630_init() != 0)
#endif
	{
		MYLOG("LORA", "Failed to initialize SX1262");
		return -1;
	}

	// Setup the EUIs and Keys
	lmh_setDevEui(g_lorawan_settings.node_device_eui);
	lmh_setAppEui(g_lorawan_settings.node_app_eui);
	lmh_setAppKey(g_lorawan_settings.node_app_key);
	lmh_setNwkSKey(g_lorawan_settings.node_nws_key);
	lmh_setAppSKey(g_lorawan_settings.node_apps_key);
	lmh_setDevAddr(g_lorawan_settings.node_dev_addr);

	// Setup the LoRaWan init structure
	lora_param_init.adr_enable = g_lorawan_settings.adr_enabled;
	lora_param_init.tx_data_rate = g_lorawan_settings.data_rate;
	lora_param_init.enable_public_network = g_lorawan_settings.public_network;
	lora_param_init.nb_trials = g_lorawan_settings.join_trials;
	lora_param_init.tx_power = g_lorawan_settings.tx_power;
	lora_param_init.duty_cycle = g_lorawan_settings.duty_cycle_enabled;

	// Initialize LoRaWan
	MYLOG("LORA", "Initialize for region %s", regionValues[g_lorawan_settings.lora_region].c_str());
	if (lmh_init(&lora_callbacks, lora_param_init, g_lorawan_settings.otaa_enabled, CLASS_A, g_lorawan_settings.lora_region) != 0)
	{
		MYLOG("LORA", "Failed to initialize LoRaWAN");
		return -2;
	}

	// For some regions we might need to define the sub band the gateway is listening to
	// This must be called AFTER lmh_init()
	if (!lmh_setSubBandChannels(g_lorawan_settings.subband_channels))
	{
		MYLOG("LORA", "lmh_setSubBandChannels failed. Wrong sub band requested?");
		return -3;
	}

	// Start the task that will handle the LoRaWan events
	MYLOG("LORA", "Starting LoRaWan task");
#ifdef _VARIANT_ISP4520_
	if (!xTaskCreate(lora_task, "LORA", 2048, NULL, TASK_PRIO_LOW, &loraTaskHandle))
#else
	if (!xTaskCreate(lora_task, "LORA", 4096, NULL, TASK_PRIO_LOW, &loraTaskHandle))
#endif
	{
		MYLOG("LORA", "Failed to start LoRaWAN task");
		return -4;
	}

	g_lorawan_initialized = true;
	return 0;
}

/**
 * @brief Independent task to handle LoRa events
 * 
 * @param pvParameters Unused
 */
void lora_task(void *pvParameters)
{
	// Start Join procedure
	MYLOG("LORA", "Start network join request");
	delay(200);
	lmh_join();

	while (1)
	{
		if (!lpwan_has_joined)
		{
			Radio.IrqProcess();
			delay(10);
			if (lpwan_has_joined)
			{
				// In deep sleep we need to hijack the SX126x IRQ to trigger a wakeup of the nRF52
				attachInterrupt(PIN_LORA_DIO_1, lora_interrupt_handler, RISING);
			}
		}
		else
		{
			// Switch off the indicator lights
			digitalWrite(LED_BUILTIN, LOW);
			// Only if semaphore is available we need to handle LoRa events.
			// Otherwise we sleep here until an event occurs
			if (xSemaphoreTake(lora_sem, portMAX_DELAY) == pdTRUE)
			{
				// Switch off the indicator lights
				digitalWrite(LED_BUILTIN, HIGH);
				// Handle Radio events with special process command!!!!
				Radio.IrqProcessAfterDeepSleep();
			}
		}
	}
}

/**************************************************************/
/* LoRaWAN callback functions                                            */
/**************************************************************/
/**
   @brief LoRa function when join has failed
*/
void lpwan_join_fail_handler(void)
{
	MYLOG("LORA", "OTAA joined failed");
	MYLOG("LORA", "Check LPWAN credentials and if a gateway is in range");
	// Restart Join procedure
	MYLOG("LORA", "Restart network join request");
	lmh_join();
}

/**
 * @brief LoRa function for handling HasJoined event.
 */
static void lpwan_joined_handler(void)
{
	digitalWrite(LED_BUILTIN, LOW);

	if (g_lorawan_settings.otaa_enabled)
	{
		uint32_t otaaDevAddr = lmh_getDevAddr();
		MYLOG("LORA", "OTAA joined and got dev address %08lX", otaaDevAddr);
	}
	else
	{
		MYLOG("LORA", "ABP joined");
	}

	delay(100); // Just to enable the serial port to send the message

	// Class A is default in the LoRaWAN lib. If app needs different class, request change here
	if (g_lorawan_settings.lora_class != CLASS_A)
	{
		// Switch to configured class
		lmh_class_request((DeviceClass_t)g_lorawan_settings.lora_class);
	}
	else
	{
		lpwan_has_joined = true;
		// Wake up task to send initial packet
		g_task_event_type |= STATUS;
		// Notify task about the event
		if (g_task_sem != NULL)
		{
			MYLOG("LORA", "Waking up loop task");
			delay(100); // Just to enable the serial port to send the message
			xSemaphoreGive(g_task_sem);
		}

		lpwan_has_joined = true;
	}

	if (g_lorawan_settings.send_repeat_time != 0)
	{
	// Now we are connected, start the timer that will wakeup the loop frequently
	g_task_wakeup_timer.begin(g_lorawan_settings.send_repeat_time, periodic_wakeup);
	g_task_wakeup_timer.start();
}
}

/**
 * @brief Function for handling LoRaWan received data from Gateway
 *
 * @param app_data  Pointer to rx data
 */
static void lpwan_rx_handler(lmh_app_data_t *app_data)
{
	MYLOG("LORA", "LoRa Packet received on port %d, size:%d, rssi:%d, snr:%d",
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
				MYLOG("LORA", "Request to switch to class A");
				break;

			case 1:
				lmh_class_request(CLASS_B);
				MYLOG("LORA", "Request to switch to class B");
				break;

			case 2:
				lmh_class_request(CLASS_C);
				MYLOG("LORA", "Request to switch to class C");
				break;

			default:
				break;
			}
		}
		break;
	case LORAWAN_APP_PORT:
		// Copy the data into loop data buffer
		memcpy(g_rx_lora_data, app_data->buffer, app_data->buffsize);
		g_rx_data_len = app_data->buffsize;
		g_task_event_type |= LORA_DATA;
		// Notify task about the event
		if (g_task_sem != NULL)
		{
			MYLOG("LORA", "Waking up loop task");
			xSemaphoreGive(g_task_sem);
		}
	}
}

/**
 * @brief Callback for class switch confirmation
 * 
 * @param Class The new class
 */
static void lpwan_class_confirm_handler(DeviceClass_t Class)
{
	MYLOG("LORA", "switch to class %c done", "ABC"[Class]);

	// Wake up task to send initial packet
	g_task_event_type |= LORA_DATA;
	// Notify task about the event
	if (g_task_sem != NULL)
	{
		MYLOG("LORA", "Waking up loop task");
		xSemaphoreGive(g_task_sem);
	}
	lpwan_has_joined = true;
}

/**
 * @brief Send a LoRaWan package
 * 
 * @return result of send request
 */
bool send_lora_packet(uint8_t *data, uint8_t size)
{
	if (lmh_join_status_get() != LMH_SET)
	{
		//Not joined, try again later
		MYLOG("LORA", "Did not join network, skip sending frame");
		return false;
	}

	m_lora_app_data.port = LORAWAN_APP_PORT;

	m_lora_app_data.buffsize = size;

	memcpy(m_lora_app_data_buffer, data, size);

	lmh_error_status error = lmh_send(&m_lora_app_data, g_lorawan_settings.confirmed_msg_enabled);

	return (error == 0);
}
