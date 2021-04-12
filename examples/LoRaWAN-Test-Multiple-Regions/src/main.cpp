/**
 * @file main.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief LoRa configuration over BLE
 * @version 0.1
 * @date 2021-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "main.h"

/** Semaphore used by events to wake up loop task */
SemaphoreHandle_t g_task_sem = NULL;

/** Timer to wakeup task frequently and send message */
SoftwareTimer g_task_wakeup_timer;

/** Wake up events */
uint16_t NO_EVENT = 0;
uint16_t STATUS = 0b0000000000000001;
uint16_t N_STATUS = 0b1111111111111110;
uint16_t BLE_CONFIG = 0b0000000000000010;
uint16_t N_BLE_CONFIG = 0b1111111111111101;
uint16_t BLE_DATA = 0b0000000000000100;
uint16_t N_BLE_DATA = 0b1111111111111011;
uint16_t LORA_DATA = 0b0000000000001000;
uint16_t N_LORA_DATA = 0b1111111111110111;
uint16_t LIGHT = 0b0000000000010000;
uint16_t N_LIGHT = 0b1111111111101111;
uint16_t PIR_TRIGGER = 0b0000000000100000;
uint16_t N_PIR_TRIGGER = 0b1111111111011111;
uint16_t BUTTON = 0b0000000001000000;
uint16_t N_BUTTON = 0b1111111110111111;

/** Flag for the event type */
uint16_t g_task_event_type = NO_EVENT;

/**
 * @brief Timer event that wakes up the loop task frequently
 * 
 * @param unused 
 */
void periodic_wakeup(TimerHandle_t unused)
{
	// Switch on blue LED to show we are awake
	digitalWrite(LED_BUILTIN, HIGH);
	g_task_event_type |= STATUS;
	xSemaphoreGiveFromISR(g_task_sem, pdFALSE);
}

/**
 * @brief Arduino setup function. Called once after power-up or reset
 * 
 */
void setup()
{
	// Create the task event semaphore
	g_task_sem = xSemaphoreCreateBinary();
	// Initialize semaphore
	xSemaphoreGive(g_task_sem);

	// Initialize the built in LED
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	// Initialize the connection status LED
	pinMode(LED_CONN, OUTPUT);
	digitalWrite(LED_CONN, HIGH);

#if MY_DEBUG > 0
	// Initialize Serial for debug output
	Serial.begin(115200);

	time_t serial_timeout = millis();
	// On nRF52840 the USB serial is not available immediately
	while (!Serial)
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
			digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
		}
		else
		{
			break;
		}
	}
#endif

	digitalWrite(LED_BUILTIN, HIGH);

	MYLOG("APP", "=====================================");
	MYLOG("APP", "RAK4631 LoRa BLE Config Test");
	MYLOG("APP", "=====================================");

	// Get LoRa parameter
	init_flash();

	// Init BLE
	init_ble();

	// Check if auto join is enabled
	if (g_lorawan_settings.auto_join)
	{
		MYLOG("APP", "Auto join is enabled, start LoRa and join");
		// Initialize LoRa and start join request
		int8_t lora_init_result = init_lora();

		if (lora_init_result != 0)
		{
			MYLOG("APP", "Init LoRa failed");

			// Without working LoRa we just stop here
			while (1)
			{
				MYLOG("APP", "Nothing I can do, just loving you");
				digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
				delay(5000);
			}
		}
		MYLOG("APP", "LoRa init success");
	}
	else
	{
		MYLOG("APP", "Auto join is disabled, waiting for connect command");
		delay(100);
	}

	// Take the semaphore so the loop will go to sleep until an event happens
	xSemaphoreTake(g_task_sem, 10);
}

/**
 * @brief Arduino loop task. Called in a loop from the FreeRTOS task handler
 * 
 */
void loop()
{
	// Sleep until we are woken up by an event
	if (xSemaphoreTake(g_task_sem, portMAX_DELAY) == pdTRUE)
	{
		// Switch on green LED to show we are awake
		digitalWrite(LED_BUILTIN, HIGH);
		while (g_task_event_type != NO_EVENT)
		{
			if ((g_task_event_type & STATUS) == STATUS)
			{
				g_task_event_type &= N_STATUS;
				MYLOG("APP", "Timer wakeup");
				delay(100); // Just to enable the serial port to send the message

				if (g_lorawan_settings.send_repeat_time == 0)
				{
					MYLOG("APP", "Repeat send is disabled");
					delay(100); // Just to enable the serial port to send the message
				}
				else
				{
					/**************************************************************/
					/**************************************************************/
					/// \todo read sensor or whatever you need to do frequently
					/// \todo write your data into a char array
					/// \todo call LoRa P2P send_lora_packet()
					/**************************************************************/
					/**************************************************************/

					uint8_t collected_data[8] = {0};
					uint8_t data_size = 0;
					collected_data[data_size++] = 'H';
					collected_data[data_size++] = 'e';
					collected_data[data_size++] = 'l';
					collected_data[data_size++] = 'l';
					collected_data[data_size++] = 'o';

					if (!send_lora_packet(collected_data, data_size))
					{
						MYLOG("APP", "LoRa package sending failed");
					}
					else
					{
						MYLOG("APP", "LoRa package sent");
					}
				}
				if ((g_task_event_type & BLE_CONFIG) == BLE_CONFIG)
				{
					g_task_event_type &= N_BLE_CONFIG;
					MYLOG("APP", "Config received over BLE");
					delay(100);

					// Inform connected device about new settings
					lora_data.write((void *)&g_lorawan_settings, sizeof(s_lorawan_settings));
					lora_data.notify((void *)&g_lorawan_settings, sizeof(s_lorawan_settings));

					// Check if auto connect is enabled
					if ((g_lorawan_settings.auto_join) && !g_lorawan_initialized)
					{
						init_lora();
					}
				}
				if ((g_task_event_type & BLE_DATA) == BLE_DATA)
				{
					g_task_event_type &= N_BLE_DATA;
					String uart_rx_buff = ble_uart.readStringUntil('\n');

					uart_rx_buff.toUpperCase();

					MYLOG("BLE", "BLE Received %s", uart_rx_buff.c_str());

					if (uart_rx_buff[0] == 'S')
					{
						ble_uart.printf("Alive and LoRaWAN %s", g_lorawan_initialized ? "initialized" : "not initialized");
						ble_log_settings();
					}
					if (uart_rx_buff[0] == 'E')
					{
						ble_uart.printf("Erasing Flash content");
						flash_reset();
						ble_uart.printf("Reset device");
						delay(5000);
						sd_nvic_SystemReset();
					}
					if (uart_rx_buff[0] == 'R')
					{
						ble_uart.printf("Reset device");
						delay(5000);
						sd_nvic_SystemReset();
					}
				}
				if ((g_task_event_type & LORA_DATA) == LORA_DATA)
				{
					g_task_event_type &= N_LORA_DATA;
					MYLOG("APP", "Received package over LoRa");
					if (g_rx_lora_data[0] > 0x1F)
					{
						MYLOG("APP", "%s", (char *)g_rx_lora_data);
					}
					else
					{
						char log_buff[g_rx_data_len * 3] = {0};
						uint8_t log_idx = 0;
						for (int idx = 0; idx < g_rx_data_len; idx++)
						{
							sprintf(&log_buff[log_idx], "%02X ", g_rx_lora_data[idx]);
							log_idx += 3;
						}
						MYLOG("APP", "%s", log_buff);
					}
					if (ble_uart_is_connected)
					{
						for (int idx = 0; idx < g_rx_data_len; idx++)
						{
							ble_uart.printf("%02X ", g_rx_lora_data[idx]);
						}
						ble_uart.println("");
					}
				}
			}
			MYLOG("APP", "Loop goes to sleep");
			g_task_event_type = 0;
			// Go back to sleep
			xSemaphoreTake(g_task_sem, 10);
			// Switch off blue LED to show we go to sleep
			digitalWrite(LED_BUILTIN, LOW);
			delay(10);
		}
	}
}