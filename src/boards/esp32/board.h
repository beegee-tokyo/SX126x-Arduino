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
#include <SPI.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "system/delay.h"
// #include "gpio-board.h"
// #include "spi-board.h"

#include "radio/radio.h"
#include "radio/sx126x/sx126x.h"
#include "sx126x-board.h"
#include "system/timer.h"

// #define LORA_RESET 	PIN_LORA_RESET
// #define SPI_INSTANCE 	0

#define SX1262_CHIP
#define REGION_EU868
#define USE_TCXO 1

// ESP32 - SX126x pin configuration
#define PIN_LORA_RESET 4  // LORA RESET
#define PIN_LORA_NSS 5	// LORA SPI CS
#define PIN_LORA_SCLK 18  // LORA SPI CLK
#define PIN_LORA_MISO 19  // LORA SPI MISO
#define PIN_LORA_DIO_1 21 // LORA DIO_1
#define PIN_LORA_BUSY 22  // LORA SPI BUSY
#define PIN_LORA_MOSI 23  // LORA SPI MOSI
#define RADIO_TXEN 26	 // LORA ANTENNA TX ENABLE
#define RADIO_RXEN 27	 // LORA ANTENNA RX ENABLE

extern "C"
{

	/**@brief Initializes the target board peripherals.
 */
	uint32_t lora_hardware_init(void);

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
 * @param [IN] id Pointer to an array that will contain the Unique ID
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
};
#endif // __BOARD_H__
