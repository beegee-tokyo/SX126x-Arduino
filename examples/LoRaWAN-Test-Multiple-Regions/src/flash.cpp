/**
 * @file flash.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialize, read and write parameters from/to internal flash memory
 * @version 0.1
 * @date 2021-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "main.h"

s_lorawan_settings g_flash_content;

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;

static const char settings_name[] = "RAK";

File file(InternalFS);

void flash_reset(void);

/**
 * @brief Initialize access to nRF52 internal file system
 * 
 */
void init_flash(void)
{
	// Initialize Internal File System
	InternalFS.begin();

	// Check if file exists
	file.open(settings_name, FILE_O_READ);
	if (!file)
	{
		MYLOG("FLASH", "File doesn't exist, force format");
		delay(100);
		flash_reset();
		return;
	}
	file.read((uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings));
	file.close();
	// Check if it is LoRaWAN settings
	if ((g_lorawan_settings.valid_mark_1 != 0xAA) || (g_lorawan_settings.valid_mark_2 != LORAWAN_DATA_MARKER))
	{
		// Data is not valid, reset to defaults
		MYLOG("FLASH", "Invalid data set, deleting and restart node");
		InternalFS.format();
		delay(1000);
		sd_nvic_SystemReset();
	}
	log_settings();
}

/**
 * @brief Save changed settings if required
 * 
 * @return boolean 
 * 			result of saving
 */
boolean save_settings(void)
{
	bool result = true;
	// Read saved content
	file.open(settings_name, FILE_O_READ);
	if (!file)
	{
		MYLOG("FLASH", "File doesn't exist, force format");
		delay(100);
		flash_reset();
	}
	file.read((uint8_t *)&g_flash_content, sizeof(s_lorawan_settings));
	file.close();
	if (memcmp((void *)&g_flash_content, (void *)&g_lorawan_settings, sizeof(s_lorawan_settings)) != 0)
	{
		MYLOG("FLASH", "Flash content changed, writing new data");
		delay(100);

		InternalFS.remove(settings_name);

		if (file.open(settings_name, FILE_O_WRITE))
		{
			file.write((uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings));
			file.flush();
		}
		else
		{
			result = false;
		}
		file.close();
	}
	log_settings();
	return result;
}

/**
 * @brief Reset content of the filesystem
 * 
 */
void flash_reset(void)
{
	InternalFS.format();
	if (file.open(settings_name, FILE_O_WRITE))
	{
		file.write((uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings));
		file.flush();
		file.close();
	}
}

/**
 * @brief Printout of all settings
 * 
 */
void log_settings(void)
{
	MYLOG("FLASH", "Saved settings:");
	MYLOG("FLASH", "000 Marks: %02X %02X", g_lorawan_settings.valid_mark_1, g_lorawan_settings.valid_mark_2);
	MYLOG("FLASH", "002 Auto join %s", g_lorawan_settings.auto_join ? "enabled" : "disabled");
	MYLOG("FLASH", "003 OTAA %s", g_lorawan_settings.otaa_enabled ? "enabled" : "disabled");
	MYLOG("FLASH", "004 Dev EUI %02X %02X %02X %02X %02X %02X %02X %02X", g_lorawan_settings.node_device_eui[0], g_lorawan_settings.node_device_eui[1],
		  g_lorawan_settings.node_device_eui[2], g_lorawan_settings.node_device_eui[3],
		  g_lorawan_settings.node_device_eui[4], g_lorawan_settings.node_device_eui[5],
		  g_lorawan_settings.node_device_eui[6], g_lorawan_settings.node_device_eui[7]);
	MYLOG("FLASH", "012 App EUI %02X %02X %02X %02X %02X %02X %02X %02X", g_lorawan_settings.node_app_eui[0], g_lorawan_settings.node_app_eui[1],
		  g_lorawan_settings.node_app_eui[2], g_lorawan_settings.node_app_eui[3],
		  g_lorawan_settings.node_app_eui[4], g_lorawan_settings.node_app_eui[5],
		  g_lorawan_settings.node_app_eui[6], g_lorawan_settings.node_app_eui[7]);
	MYLOG("FLASH", "020 App Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		  g_lorawan_settings.node_app_key[0], g_lorawan_settings.node_app_key[1],
		  g_lorawan_settings.node_app_key[2], g_lorawan_settings.node_app_key[3],
		  g_lorawan_settings.node_app_key[4], g_lorawan_settings.node_app_key[5],
		  g_lorawan_settings.node_app_key[6], g_lorawan_settings.node_app_key[7],
		  g_lorawan_settings.node_app_key[8], g_lorawan_settings.node_app_key[9],
		  g_lorawan_settings.node_app_key[10], g_lorawan_settings.node_app_key[11],
		  g_lorawan_settings.node_app_key[12], g_lorawan_settings.node_app_key[13],
		  g_lorawan_settings.node_app_key[14], g_lorawan_settings.node_app_key[15]);
	MYLOG("FLASH", "036 NWS Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		  g_lorawan_settings.node_nws_key[0], g_lorawan_settings.node_nws_key[1],
		  g_lorawan_settings.node_nws_key[2], g_lorawan_settings.node_nws_key[3],
		  g_lorawan_settings.node_nws_key[4], g_lorawan_settings.node_nws_key[5],
		  g_lorawan_settings.node_nws_key[6], g_lorawan_settings.node_nws_key[7],
		  g_lorawan_settings.node_nws_key[8], g_lorawan_settings.node_nws_key[9],
		  g_lorawan_settings.node_nws_key[10], g_lorawan_settings.node_nws_key[11],
		  g_lorawan_settings.node_nws_key[12], g_lorawan_settings.node_nws_key[13],
		  g_lorawan_settings.node_nws_key[14], g_lorawan_settings.node_nws_key[15]);
	MYLOG("FLASH", "052 Apps Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		  g_lorawan_settings.node_apps_key[0], g_lorawan_settings.node_apps_key[1],
		  g_lorawan_settings.node_apps_key[2], g_lorawan_settings.node_apps_key[3],
		  g_lorawan_settings.node_apps_key[4], g_lorawan_settings.node_apps_key[5],
		  g_lorawan_settings.node_apps_key[6], g_lorawan_settings.node_apps_key[7],
		  g_lorawan_settings.node_apps_key[8], g_lorawan_settings.node_apps_key[9],
		  g_lorawan_settings.node_apps_key[10], g_lorawan_settings.node_apps_key[11],
		  g_lorawan_settings.node_apps_key[12], g_lorawan_settings.node_apps_key[13],
		  g_lorawan_settings.node_apps_key[14], g_lorawan_settings.node_apps_key[15]);
	MYLOG("FLASH", "068 Dev Addr %08lX", g_lorawan_settings.node_dev_addr);
	MYLOG("FLASH", "072 Repeat time %ld", g_lorawan_settings.send_repeat_time);
	MYLOG("FLASH", "076 ADR %s", g_lorawan_settings.adr_enabled ? "enabled" : "disabled");
	MYLOG("FLASH", "077 %s Network", g_lorawan_settings.public_network ? "Public" : "Private");
	MYLOG("FLASH", "078 Dutycycle %s", g_lorawan_settings.duty_cycle_enabled ? "enabled" : "disabled");
	MYLOG("FLASH", "079 Join trials %d", g_lorawan_settings.join_trials);
	MYLOG("FLASH", "080 TX Power %d", g_lorawan_settings.tx_power);
	MYLOG("FLASH", "081 DR %d", g_lorawan_settings.data_rate);
	MYLOG("FLASH", "082 Class %d", g_lorawan_settings.lora_class);
	MYLOG("FLASH", "083 Subband %d", g_lorawan_settings.subband_channels);
	MYLOG("FLASH", "084 Fport %d", g_lorawan_settings.app_port);
	MYLOG("FLASH", "085 %s Message", g_lorawan_settings.confirmed_msg_enabled ? "Confirmed" : "Unconfirmed");
	MYLOG("FLASH", "087 Region %d", g_lorawan_settings.lora_region);
}

/**
 * @brief Printout of all settings
 * 
 */
void ble_log_settings(void)
{
	ble_uart.printf("Saved settings:\n");
	delay(50);
	ble_uart.printf("000 Marks: %02X %02X\n", g_lorawan_settings.valid_mark_1, g_lorawan_settings.valid_mark_2);
	delay(50);
	ble_uart.printf("002 Auto join %s\n", g_lorawan_settings.auto_join ? "enabled" : "disabled");
	delay(50);
	ble_uart.printf("003 OTAA %s\n", g_lorawan_settings.otaa_enabled ? "enabled" : "disabled");
	delay(50);
	ble_uart.printf("004 Dev EUI %02X %02X %02X %02X %02X %02X %02X %02X\n", g_lorawan_settings.node_device_eui[0], g_lorawan_settings.node_device_eui[1],
					g_lorawan_settings.node_device_eui[2], g_lorawan_settings.node_device_eui[3],
					g_lorawan_settings.node_device_eui[4], g_lorawan_settings.node_device_eui[5],
					g_lorawan_settings.node_device_eui[6], g_lorawan_settings.node_device_eui[7]);
	delay(50);
	ble_uart.printf("012 App EUI %02X %02X %02X %02X %02X %02X %02X %02X\n", g_lorawan_settings.node_app_eui[0], g_lorawan_settings.node_app_eui[1],
					g_lorawan_settings.node_app_eui[2], g_lorawan_settings.node_app_eui[3],
					g_lorawan_settings.node_app_eui[4], g_lorawan_settings.node_app_eui[5],
					g_lorawan_settings.node_app_eui[6], g_lorawan_settings.node_app_eui[7]);
	delay(50);
	ble_uart.printf("020 App Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
					g_lorawan_settings.node_app_key[0], g_lorawan_settings.node_app_key[1],
					g_lorawan_settings.node_app_key[2], g_lorawan_settings.node_app_key[3],
					g_lorawan_settings.node_app_key[4], g_lorawan_settings.node_app_key[5],
					g_lorawan_settings.node_app_key[6], g_lorawan_settings.node_app_key[7],
					g_lorawan_settings.node_app_key[8], g_lorawan_settings.node_app_key[9],
					g_lorawan_settings.node_app_key[10], g_lorawan_settings.node_app_key[11],
					g_lorawan_settings.node_app_key[12], g_lorawan_settings.node_app_key[13],
					g_lorawan_settings.node_app_key[14], g_lorawan_settings.node_app_key[15]);
	delay(50);
	ble_uart.printf("036 NWS Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
					g_lorawan_settings.node_nws_key[0], g_lorawan_settings.node_nws_key[1],
					g_lorawan_settings.node_nws_key[2], g_lorawan_settings.node_nws_key[3],
					g_lorawan_settings.node_nws_key[4], g_lorawan_settings.node_nws_key[5],
					g_lorawan_settings.node_nws_key[6], g_lorawan_settings.node_nws_key[7],
					g_lorawan_settings.node_nws_key[8], g_lorawan_settings.node_nws_key[9],
					g_lorawan_settings.node_nws_key[10], g_lorawan_settings.node_nws_key[11],
					g_lorawan_settings.node_nws_key[12], g_lorawan_settings.node_nws_key[13],
					g_lorawan_settings.node_nws_key[14], g_lorawan_settings.node_nws_key[15]);
	delay(50);
	ble_uart.printf("052 Apps Key %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
					g_lorawan_settings.node_apps_key[0], g_lorawan_settings.node_apps_key[1],
					g_lorawan_settings.node_apps_key[2], g_lorawan_settings.node_apps_key[3],
					g_lorawan_settings.node_apps_key[4], g_lorawan_settings.node_apps_key[5],
					g_lorawan_settings.node_apps_key[6], g_lorawan_settings.node_apps_key[7],
					g_lorawan_settings.node_apps_key[8], g_lorawan_settings.node_apps_key[9],
					g_lorawan_settings.node_apps_key[10], g_lorawan_settings.node_apps_key[11],
					g_lorawan_settings.node_apps_key[12], g_lorawan_settings.node_apps_key[13],
					g_lorawan_settings.node_apps_key[14], g_lorawan_settings.node_apps_key[15]);
	delay(50);
	ble_uart.printf("068 Dev Addr %08lX\n", g_lorawan_settings.node_dev_addr);
	delay(50);
	ble_uart.printf("072 Repeat time %ld\n", g_lorawan_settings.send_repeat_time);
	delay(50);
	ble_uart.printf("076 ADR %s\n", g_lorawan_settings.adr_enabled ? "enabled" : "disabled");
	delay(50);
	ble_uart.printf("077 %s Network\n", g_lorawan_settings.public_network ? "Public" : "Private");
	delay(50);
	ble_uart.printf("078 Dutycycle %s\n", g_lorawan_settings.duty_cycle_enabled ? "enabled" : "disabled");
	delay(50);
	ble_uart.printf("079 Join trials %d\n", g_lorawan_settings.join_trials);
	delay(50);
	ble_uart.printf("080 TX Power %d\n", g_lorawan_settings.tx_power);
	delay(50);
	ble_uart.printf("081 DR %d\n", g_lorawan_settings.data_rate);
	delay(50);
	ble_uart.printf("082 Class %d\n", g_lorawan_settings.lora_class);
	delay(50);
	ble_uart.printf("083 Subband %d\n", g_lorawan_settings.subband_channels);
	delay(50);
	ble_uart.printf("084 Fport %d\n", g_lorawan_settings.app_port);
	delay(50);
	ble_uart.printf("085 %s Message\n", g_lorawan_settings.confirmed_msg_enabled ? "Confirmed" : "Unconfirmed");
	delay(50);
	ble_uart.printf("087 Region %d\n", g_lorawan_settings.lora_region);
}