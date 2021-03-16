/**
 * @file settings-service.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Handler for LoRa settings service
 * @version 0.1
 * @date 2021-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "main.h"

/** LoRa P2P service 0xF0A0 */
BLEService lorap2p_service = BLEService(0xF0A0);
/** LoRa settings  characteristic 0xF0A1 */
BLECharacteristic lora_data = BLECharacteristic(0xF0A1);

// Command callback
void settings_rx_callback(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len);

/**
 * @brief Initialize the settings characteristic
 * 
 */
BLEService init_settings_characteristic(void)
{
	// Initialize the LoRa setting service
	lorap2p_service.begin();
	lora_data.setProperties(CHR_PROPS_NOTIFY | CHR_PROPS_READ | CHR_PROPS_WRITE);
	lora_data.setPermission(SECMODE_OPEN, SECMODE_OPEN);
	lora_data.setFixedLen(sizeof(s_lorawan_settings) + 1);
	lora_data.setWriteCallback(settings_rx_callback);

	lora_data.begin();

	lora_data.write((void *)&g_lorawan_settings, sizeof(s_lorawan_settings));

	return lorap2p_service;
}

/**
 * Callback if data has been sent from the connected client
 * @param conn_hdl
 * 		The connection handle
 * @param chr
 *      The called characteristic
 * @param data
 *      Pointer to received data
 * @param len
 *      Length of the received data
 */
void settings_rx_callback(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len)
{
	MYLOG("SETT", "Settings received");

	delay(1000);

	// Check the characteristic
	if (chr->uuid == lora_data.uuid)
	{
		if (len != sizeof(s_lorawan_settings))
		{
			MYLOG("SETT", "Received settings have wrong size %d", len);
			return;
		}

		s_lorawan_settings *rcvdSettings = (s_lorawan_settings *)data;
		if ((rcvdSettings->valid_mark_1 != 0xAA) || (rcvdSettings->valid_mark_2 != LORAWAN_DATA_MARKER))
		{
			MYLOG("SETT", "Received settings data do not have required markers");
			return;
		}

		// Save new LoRa settings
		memcpy((void *)&g_lorawan_settings, data, sizeof(s_lorawan_settings));

		// Save new settings
		save_settings();

		// Update settings
		lora_data.write((void *)&g_lorawan_settings, sizeof(s_lorawan_settings));

		// Inform connected device about new settings
		lora_data.notify((void *)&g_lorawan_settings, sizeof(s_lorawan_settings));

		if (g_lorawan_settings.resetRequest)
		{
			MYLOG("SETT", "Initiate reset");
			delay(1000);
			sd_nvic_SystemReset();
		}

		// Notify task about the event
		if (g_task_sem != NULL)
		{
			g_task_event_type |= BLE_CONFIG;
			MYLOG("SETT", "Waking up loop task");
			xSemaphoreGive(g_task_sem);
		}
	}
}