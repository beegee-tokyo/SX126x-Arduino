/**
 * @file ble.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief BLE initialization & device configuration
 * @version 0.1
 * @date 2021-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "main.h"

/** OTA DFU service */
BLEDfu ble_dfu;
/** BLE UART service */
BLEUart ble_uart;
/** Device information service */
BLEDis ble_dis;

extern BLEService lorap2p_service;

/** Flag if BLE UART is connected */
bool ble_uart_is_connected = false;

// Connect callback
void connect_callback(uint16_t conn_handle);
// Disconnect callback
void disconnect_callback(uint16_t conn_handle, uint8_t reason);
// Uart RX callback
void bleuart_rx_callback(uint16_t conn_handle);

/**
 * @brief Initialize BLE and start advertising
 * 
 */
void init_ble(void)
{
	// Config the peripheral connection with maximum bandwidth
	// more SRAM required by SoftDevice
	// Note: All config***() function must be called before begin()
	Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
	Bluefruit.configPrphConn(sizeof(s_lorawan_settings) + 3, BLE_GAP_EVENT_LENGTH_MIN, 16, 16);

	// Start BLE
	Bluefruit.begin(1, 0);

	// Set max power. Accepted values are: (min) -40, -20, -16, -12, -8, -4, 0, 2, 3, 4, 5, 6, 7, 8 (max)
	Bluefruit.setTxPower(8);

	// Create device name
	char helper_string[256] = {0};

	uint32_t addr_high = ((*((uint32_t *)(0x100000a8))) & 0x0000ffff) | 0x0000c000;
	uint32_t addr_low = *((uint32_t *)(0x100000a4));
#ifdef _VARIANT_ISP4520_
	/** Device name for ISP4520 */
	sprintf(helper_string, "ISP-%02X%02X%02X%02X%02X%02X",
			(uint8_t)(addr_high), (uint8_t)(addr_high >> 8), (uint8_t)(addr_low),
			(uint8_t)(addr_low >> 8), (uint8_t)(addr_low >> 16), (uint8_t)(addr_low >> 24));
#else
	/** Device name for RAK4631 */
	sprintf(helper_string, "RAK-%02X%02X%02X%02X%02X%02X",
			(uint8_t)(addr_high), (uint8_t)(addr_high >> 8), (uint8_t)(addr_low),
			(uint8_t)(addr_low >> 8), (uint8_t)(addr_low >> 16), (uint8_t)(addr_low >> 24));
#endif

	Bluefruit.setName(helper_string);

	// Set connection/disconnect callbacks
	Bluefruit.Periph.setConnectCallback(connect_callback);
	Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

	// Configure and Start Device Information Service
#ifdef _VARIANT_ISP4520_
	ble_dis.setManufacturer("Insigh_SIP");

	ble_dis.setModel("ISP4520");
#else
	ble_dis.setManufacturer("RAKwireless");

	ble_dis.setModel("RAK4631");
#endif

	sprintf(helper_string, "%f", SW_VERSION);
	ble_dis.setSoftwareRev(helper_string);

	ble_dis.setHardwareRev("52840");

	ble_dis.begin();

	// Start the DFU service
	ble_dfu.begin();

	// Start the UART service
	ble_uart.begin();
	ble_uart.setRxCallback(bleuart_rx_callback);

	// Initialize the LoRa setting service
	BLEService sett_service = init_settings_characteristic();

	// Advertising packet
	Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE); //
	Bluefruit.Advertising.addService(sett_service);
	Bluefruit.Advertising.addName();
	Bluefruit.Advertising.addTxPower();

	/* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds 
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
	Bluefruit.Advertising.restartOnDisconnect(true);
	Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
	Bluefruit.Advertising.setFastTimeout(15);	// number of seconds in fast mode
	Bluefruit.Advertising.start(0);				// 0 = Don't stop advertising
}

/**
 * @brief  Callback when client connects
 * @param  conn_handle: Connection handle id
 */
void connect_callback(uint16_t conn_handle)
{
	(void)conn_handle;
	ble_uart_is_connected = true;
}

/**
 * @brief  Callback invoked when a connection is dropped
 * @param  conn_handle: connection handle id
 * @param  reason: disconnect reason
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
	(void)conn_handle;
	(void)reason;
	ble_uart_is_connected = false;
}

/**
 * Callback if data has been sent from the connected client
 * @param conn_handle
 * 		The connection handle
 */
void bleuart_rx_callback(uint16_t conn_handle)
{
	(void)conn_handle;

	g_task_event_type |= BLE_DATA;
	xSemaphoreGiveFromISR(g_task_sem, pdFALSE);
}
