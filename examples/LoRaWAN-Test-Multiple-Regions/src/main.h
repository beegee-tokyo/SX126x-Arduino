/**
 * @file main.h
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Includes and global declarations
 * @version 0.1
 * @date 2021-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef MAIN_H
#define MAIN_H

#define SW_VERSION 0.01

// Debug output set to 0 to disable app debug output
#define MY_DEBUG 1

#if MY_DEBUG > 0
#define MYLOG(tag, ...)           \
	do                            \
	{                             \
		if (tag)                  \
			PRINTF("[%s] ", tag); \
		PRINTF(__VA_ARGS__);      \
		PRINTF("\n");             \
	} while (0)
#else
#define MYLOG(...)
#endif

#include <Arduino.h>
#include <nrf_nvic.h>

// Main loop stuff
void periodic_wakeup(TimerHandle_t unused);
extern SemaphoreHandle_t g_task_sem;
extern uint16_t g_task_event_type;
extern SoftwareTimer g_task_wakeup_timer;

/** Wake up events */
extern uint16_t NO_EVENT;
extern uint16_t STATUS;
extern uint16_t N_STATUS;
extern uint16_t BLE_DATA;
extern uint16_t N_BLE_DATA;
extern uint16_t BLE_CONFIG;
extern uint16_t N_BLE_CONFIG;
extern uint16_t LORA_DATA;
extern uint16_t N_LORA_DATA;
extern uint16_t LIGHT;
extern uint16_t N_LIGHT;
extern uint16_t PIR_TRIGGER;
extern uint16_t N_PIR_TRIGGER;
extern uint16_t BUTTON;
extern uint16_t N_BUTTON;

// BLE
#include <bluefruit.h>
void init_ble(void);
BLEService init_settings_characteristic(void);
extern BLECharacteristic lora_data;
extern BLEUart ble_uart;
extern bool ble_uart_is_connected;

// LoRa
#ifdef _VARIANT_ISP4520_
#include <LoRaWan-ISP4520.h>
#else
#include <LoRaWan-RAK4630.h>
#endif
int8_t init_lora(void);
bool send_lora_packet(uint8_t *data, uint8_t size);
extern bool lpwan_has_joined;

#define LORAWAN_DATA_MARKER 0x57
struct s_lorawan_settings
{
	uint8_t valid_mark_1 = 0xAA;				// Just a marker for the Flash
	uint8_t valid_mark_2 = LORAWAN_DATA_MARKER; // Just a marker for the Flash

	// Flag if node joins automatically after reboot
	bool auto_join = false;
	// Flag for OTAA or ABP
	bool otaa_enabled = true;
	// OTAA Device EUI MSB
	uint8_t node_device_eui[8] = {0x00, 0x0D, 0x75, 0xE6, 0x56, 0x4D, 0xC1, 0xF3};
	// OTAA Application EUI MSB
	uint8_t node_app_eui[8] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x02, 0x01, 0xE1};
	// OTAA Application Key MSB
	uint8_t node_app_key[16] = {0x2B, 0x84, 0xE0, 0xB0, 0x9B, 0x68, 0xE5, 0xCB, 0x42, 0x17, 0x6F, 0xE7, 0x53, 0xDC, 0xEE, 0x79};
	// ABP Network Session Key MSB
	uint8_t node_nws_key[16] = {0x32, 0x3D, 0x15, 0x5A, 0x00, 0x0D, 0xF3, 0x35, 0x30, 0x7A, 0x16, 0xDA, 0x0C, 0x9D, 0xF5, 0x3F};
	// ABP Application Session key MSB
	uint8_t node_apps_key[16] = {0x3F, 0x6A, 0x66, 0x45, 0x9D, 0x5E, 0xDC, 0xA6, 0x3C, 0xBC, 0x46, 0x19, 0xCD, 0x61, 0xA1, 0x1E};
	// ABP Device Address MSB
	uint32_t node_dev_addr = 0x26021FB4;
	// Send repeat time in milliseconds: 2 * 60 * 1000 => 2 minutes
	uint32_t send_repeat_time = 120000;
	// Flag for ADR on or off
	bool adr_enabled = false;
	// Flag for public or private network
	bool public_network = true;
	// Flag to enable duty cycle
	bool duty_cycle_enabled = false;
	// Number of join retries
	uint8_t join_trials = 5;
	// TX power 0 .. 15
	uint8_t tx_power = 0;
	// Data rate 0 .. 15 (validity depnends on Region)
	uint8_t data_rate = 3;
	// LoRaWAN class 0: A, 2: C, 1: B is not supported
	uint8_t lora_class = 0;
	// Subband channel selection 1 .. 9
	uint8_t subband_channels = 1;
	// Data port to send data
	uint8_t app_port = 2;
	// Flag to enable confirmed messages
	lmh_confirm confirmed_msg_enabled = LMH_UNCONFIRMED_MSG;
	// Command from BLE to reset device
	bool resetRequest = true;
	// LoRa region
	uint8_t lora_region = 0;
};

// int size = sizeof(s_lorawan_settings);
extern s_lorawan_settings g_lorawan_settings;
extern uint8_t g_rx_lora_data[];
extern uint8_t g_rx_data_len;
extern bool g_lorawan_initialized;

// Flash
void init_flash(void);
bool save_settings(void);
void log_settings(void);
void flash_reset(void);
void ble_log_settings(void);

#endif // MAIN_H