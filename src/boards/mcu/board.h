/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/

/******************************************************************************
 * @file    board.h
 * @author  Insight SiP
 * @version V2.0.0
 * @date    30-january-2019
 * @brief   Board (module) specific functions declaration.
 *
 * @attention
 *	THIS SOFTWARE IS PROVIDED BY INSIGHT SIP "AS IS" AND ANY EXPRESS
 *	OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *	OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *	DISCLAIMED. IN NO EVENT SHALL INSIGHT SIP OR CONTRIBUTORS BE
 *	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *	GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 *	OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#ifndef __BOARD_H__
#define __BOARD_H__

#include <Arduino.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "radio/radio.h"
#include "radio/sx126x/sx126x.h"
#include "boards/sx126x/sx126x-board.h"
#include "timer.h"
#include "sx126x-debug.h"

// SX126x chip type
#define SX1261_CHIP 1
#define SX1262_CHIP 2
#define SX1268_CHIP 2

// Microcontroller - SX126x pin configuration
struct hw_config
{
	int CHIP_TYPE = SX1262_CHIP;	  // Module type, see defines above
	int PIN_LORA_RESET;				  // LORA RESET
	int PIN_LORA_NSS;				  // LORA SPI CS
	int PIN_LORA_SCLK;				  // LORA SPI CLK
	int PIN_LORA_MISO;				  // LORA SPI MISO
	int PIN_LORA_DIO_1;				  // LORA DIO_1
	int PIN_LORA_BUSY;				  // LORA SPI BUSY
	int PIN_LORA_MOSI;				  // LORA SPI MOSI
	int RADIO_TXEN = -1;			  // LORA ANTENNA TX ENABLE (eByte E22 module only)
	int RADIO_RXEN = -1;			  // LORA ANTENNA RX ENABLE (eByte E22 module only)
	bool USE_DIO2_ANT_SWITCH = false; // Whether DIO2 is used to control the antenna
	bool USE_DIO3_TCXO = false;		  // Whether DIO3 is used to control the oscillator
	bool USE_DIO3_ANT_SWITCH = false; // Whether DIO2 is used to control the antenna
	bool USE_LDO = false;			  // Whether SX126x uses LDO or DCDC power regulator
	bool USE_RXEN_ANT_PWR = false;	  // Whether RX_EN is used as antenna power
	RadioTcxoCtrlVoltage_t TCXO_CTRL_VOLTAGE = TCXO_CTRL_3_3V;
};

#ifdef ARDUINO_ARCH_RP2040
#include <mbed.h>
#include <rtos.h>
// Wake up LoRa event handler on RP2040
extern osThreadId _lora_task_thread;
#endif

extern hw_config _hwConfig;

/**@brief Initializes the target board peripherals.
 *
 * @param [hwConfig] hw_config describes the HW connection between the MCU and the SX126x
 */
uint32_t lora_hardware_init(hw_config hwConfig);

/**@brief Initializes the target board peripherals after deep sleep wake up.
 *
 * @param [hwConfig] hw_config describes the HW connection between the MCU and the SX126x
 */
uint32_t lora_hardware_re_init(hw_config hwConfig);

/**@brief Initializes the ISP4520 board peripherals.
 *
 * @param [chipType] chipType selects either SX1262/1268 or SX1261
 */
uint32_t lora_isp4520_init(int chipType);

/**@brief Initializes the RAK4630 board peripherals.
 */
uint32_t lora_rak4630_init(void);

/**@brief Initializes the RAK11300 board peripherals.
 */
uint32_t lora_rak11300_init(void);

/**@brief Initializes the RAK13300 board peripherals.
 */
uint32_t lora_rak13300_init(void);

/**@brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void lora_hardware_uninit(void);

/**@brief Returns a pseudo random seed generated using the MCU Unique ID
 *
 * @retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed(void);

/**@brief Gets the board 64 bits unique ID
 *
 * @param [id] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId(uint8_t *id);

/**@brief   Get batttery value TO BE IMPLEMENTED
 */
uint8_t BoardGetBatteryLevel(void);

/**@brief Disable interrupts
 *
 * @remark IRQ nesting is managed
 */
void BoardDisableIrq(void);

/**@brief Enable interrupts
 *
 * @remark IRQ nesting is managed
 */
void BoardEnableIrq(void);

/**@brief Initialize LoRa handler task (ESP32 & nRF52)
	 * 
	 * 
	 */
bool start_lora_task(void);

#endif // __BOARD_H__
