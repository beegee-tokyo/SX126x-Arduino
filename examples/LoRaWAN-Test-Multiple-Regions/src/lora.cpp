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

/** LoRaWAN setting from flash */
s_lorawan_settings g_lorawan_settings;

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
	if (lmh_init(&lora_callbacks, lora_param_init, g_lorawan_settings.otaa_enabled, CLASS_A, (LoRaMacRegion_t)g_lorawan_settings.lora_region) != 0)
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

	// Start Join procedure
	MYLOG("LORA", "Start network join request");
	delay(200);
	lmh_join();

	g_lorawan_initialized = true;
	return 0;
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
