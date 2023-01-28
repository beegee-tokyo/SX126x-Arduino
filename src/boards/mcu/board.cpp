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
 * @file    board.c
 * @author  Insight SiP
 * @version V2.0.0
 * @date    30-january-2019
 * @brief   Board (module) specific functions implementation.
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
#include "board.h"

#if defined NRF52_SERIES || defined ESP32
/** Semaphore used by SX126x IRQ handler to wake up LoRaWAN task */
SemaphoreHandle_t _lora_sem = NULL;

/** LoRa task handle */
TaskHandle_t _loraTaskHandle;
/** GPS reading task */
void _lora_task(void *pvParameters);
#endif

hw_config _hwConfig;

/**@brief Unique Devices IDs register set (nRF52)
 */
#define ID1 (0x10000060)
#define ID2 (0x10000064)

uint32_t lora_hardware_init(hw_config hwConfig)
{
	_hwConfig.CHIP_TYPE = hwConfig.CHIP_TYPE;					  // Chip type, SX1261 or SX1262
	_hwConfig.PIN_LORA_RESET = hwConfig.PIN_LORA_RESET;			  // LORA RESET
	_hwConfig.PIN_LORA_NSS = hwConfig.PIN_LORA_NSS;				  // LORA SPI CS
	_hwConfig.PIN_LORA_SCLK = hwConfig.PIN_LORA_SCLK;			  // LORA SPI CLK
	_hwConfig.PIN_LORA_MISO = hwConfig.PIN_LORA_MISO;			  // LORA SPI MISO
	_hwConfig.PIN_LORA_DIO_1 = hwConfig.PIN_LORA_DIO_1;			  // LORA DIO_1
	_hwConfig.PIN_LORA_BUSY = hwConfig.PIN_LORA_BUSY;			  // LORA SPI BUSY
	_hwConfig.PIN_LORA_MOSI = hwConfig.PIN_LORA_MOSI;			  // LORA SPI MOSI
	_hwConfig.RADIO_TXEN = hwConfig.RADIO_TXEN;					  // LORA ANTENNA TX ENABLE (e.g. eByte E22 module)
	_hwConfig.RADIO_RXEN = hwConfig.RADIO_RXEN;					  // LORA ANTENNA RX ENABLE (e.g. eByte E22 module)
	_hwConfig.USE_DIO2_ANT_SWITCH = hwConfig.USE_DIO2_ANT_SWITCH; // LORA DIO2 controls antenna
	_hwConfig.USE_DIO3_TCXO = hwConfig.USE_DIO3_TCXO;			  // LORA DIO3 controls oscillator voltage (e.g. eByte E22 module)
	_hwConfig.USE_DIO3_ANT_SWITCH = hwConfig.USE_DIO3_ANT_SWITCH; // LORA DIO3 controls antenna (e.g. Insight SIP ISP4520 module)
	_hwConfig.USE_LDO = hwConfig.USE_LDO;						  // LORA usage of LDO or DCDC power regulator (defaults to DCDC)
	_hwConfig.USE_RXEN_ANT_PWR = hwConfig.USE_RXEN_ANT_PWR;		  // RXEN used as power for antenna switch
	_hwConfig.TCXO_CTRL_VOLTAGE = hwConfig.TCXO_CTRL_VOLTAGE;

	TimerConfig();

	SX126xIoInit();

	// After power on the sync word should be 2414. 4434 could be possible on a restart
	// If we got something else, something is wrong.
	uint16_t readSyncWord = 0;
	SX126xReadRegisters(REG_LR_SYNCWORD, (uint8_t *)&readSyncWord, 2);

	LOG_LIB("BRD", "SyncWord = %04X", readSyncWord);

	if ((readSyncWord == 0x2414) || (readSyncWord == 0x4434))
	{
#if defined NRF52_SERIES || defined ESP32 || ARDUINO_ARCH_RP2040
		if (start_lora_task())
		{
			return 0;
		}
		else
		{
			return 1;
		}
#else
		return 0;
#endif
	}
	return 1;
}

uint32_t lora_hardware_re_init(hw_config hwConfig)
{
	_hwConfig.CHIP_TYPE = hwConfig.CHIP_TYPE;					  // Chip type, SX1261 or SX1262
	_hwConfig.PIN_LORA_RESET = hwConfig.PIN_LORA_RESET;			  // LORA RESET
	_hwConfig.PIN_LORA_NSS = hwConfig.PIN_LORA_NSS;				  // LORA SPI CS
	_hwConfig.PIN_LORA_SCLK = hwConfig.PIN_LORA_SCLK;			  // LORA SPI CLK
	_hwConfig.PIN_LORA_MISO = hwConfig.PIN_LORA_MISO;			  // LORA SPI MISO
	_hwConfig.PIN_LORA_DIO_1 = hwConfig.PIN_LORA_DIO_1;			  // LORA DIO_1
	_hwConfig.PIN_LORA_BUSY = hwConfig.PIN_LORA_BUSY;			  // LORA SPI BUSY
	_hwConfig.PIN_LORA_MOSI = hwConfig.PIN_LORA_MOSI;			  // LORA SPI MOSI
	_hwConfig.RADIO_TXEN = hwConfig.RADIO_TXEN;					  // LORA ANTENNA TX ENABLE (e.g. eByte E22 module)
	_hwConfig.RADIO_RXEN = hwConfig.RADIO_RXEN;					  // LORA ANTENNA RX ENABLE (e.g. eByte E22 module)
	_hwConfig.USE_DIO2_ANT_SWITCH = hwConfig.USE_DIO2_ANT_SWITCH; // LORA DIO2 controls antenna
	_hwConfig.USE_DIO3_TCXO = hwConfig.USE_DIO3_TCXO;			  // LORA DIO3 controls oscillator voltage (e.g. eByte E22 module)
	_hwConfig.USE_DIO3_ANT_SWITCH = hwConfig.USE_DIO3_ANT_SWITCH; // LORA DIO3 controls antenna (e.g. Insight SIP ISP4520 module)
	_hwConfig.USE_RXEN_ANT_PWR = hwConfig.USE_RXEN_ANT_PWR;		  // RXEN used as power for antenna switch

	TimerConfig();

	SX126xIoReInit();

	// After power on the sync word should be 2414. 4434 could be possible on a restart
	// If we got something else, something is wrong.
	uint16_t readSyncWord = 0;
	SX126xReadRegisters(REG_LR_SYNCWORD, (uint8_t *)&readSyncWord, 2);

	LOG_LIB("BRD", "SyncWord = %04X", readSyncWord);

	if ((readSyncWord == 0x2414) || (readSyncWord == 0x4434))
	{
#if defined NRF52_SERIES || defined ESP32 || ARDUINO_ARCH_RP2040
		if (start_lora_task())
		{
			return 0;
		}
		else
		{
			return 1;
		}
#else
		return 0;
#endif
	}
	return 1;
}

uint32_t lora_isp4520_init(int chipType)
{
	_hwConfig.CHIP_TYPE = chipType;		  // Chip type, SX1261 or SX1262
	_hwConfig.PIN_LORA_RESET = 19;		  // LORA RESET
	_hwConfig.PIN_LORA_NSS = 24;		  // LORA SPI CS
	_hwConfig.PIN_LORA_SCLK = 23;		  // LORA SPI CLK
	_hwConfig.PIN_LORA_MISO = 25;		  // LORA SPI MISO
	_hwConfig.PIN_LORA_DIO_1 = 11;		  // LORA DIO_1
	_hwConfig.PIN_LORA_BUSY = 27;		  // LORA SPI BUSY
	_hwConfig.PIN_LORA_MOSI = 26;		  // LORA SPI MOSI
	_hwConfig.RADIO_TXEN = -1;			  // LORA ANTENNA TX ENABLE (e.g. eByte E22 module)
	_hwConfig.RADIO_RXEN = -1;			  // LORA ANTENNA RX ENABLE (e.g. eByte E22 module)
	_hwConfig.USE_DIO2_ANT_SWITCH = true; // LORA DIO2 controls antenna
	_hwConfig.USE_DIO3_TCXO = false;	  // LORA DIO3 does not controls oscillator voltage (e.g. eByte E22 module)
	_hwConfig.USE_DIO3_ANT_SWITCH = true; // LORA DIO3 controls antenna (e.g. Insight SIP ISP4520 module)
	_hwConfig.USE_RXEN_ANT_PWR = false;	  // RXEN is not used as power for antenna switch
	TimerConfig();

	SX126xIoInit();

	// After power on the sync word should be 2414. 4434 could be possible on a restart
	// If we got something else, something is wrong.
	uint16_t readSyncWord = 0;
	SX126xReadRegisters(REG_LR_SYNCWORD, (uint8_t *)&readSyncWord, 2);

	LOG_LIB("BRD", "SyncWord = %04X", readSyncWord);

	if ((readSyncWord == 0x2414) || (readSyncWord == 0x4434))
	{
#if defined NRF52_SERIES || defined ESP32 || ARDUINO_ARCH_RP2040
		if (start_lora_task())
		{
			return 0;
		}
		else
		{
			return 1;
		}
#else
		return 0;
#endif
	}
	return 1;
}

uint32_t lora_rak4630_init(void)
{
	_hwConfig.CHIP_TYPE = SX1262;		   // Chip type, SX1261 or SX1262
	_hwConfig.PIN_LORA_RESET = 38;		   // LORA RESET
	_hwConfig.PIN_LORA_NSS = 42;		   // LORA SPI CS
	_hwConfig.PIN_LORA_SCLK = 43;		   // LORA SPI CLK
	_hwConfig.PIN_LORA_MISO = 45;		   // LORA SPI MISO
	_hwConfig.PIN_LORA_DIO_1 = 47;		   // LORA DIO_1
	_hwConfig.PIN_LORA_BUSY = 46;		   // LORA SPI BUSY
	_hwConfig.PIN_LORA_MOSI = 44;		   // LORA SPI MOSI
	_hwConfig.RADIO_TXEN = -1;			   // LORA ANTENNA TX ENABLE
	_hwConfig.RADIO_RXEN = 37;			   // LORA ANTENNA RX ENABLE (power for antenna switch)
	_hwConfig.USE_DIO2_ANT_SWITCH = true;  // LORA DIO2 controls antenna
	_hwConfig.USE_DIO3_TCXO = true;		   // LORA DIO3 controls oscillator voltage
	_hwConfig.USE_DIO3_ANT_SWITCH = false; // LORA DIO3 controls antenna
	_hwConfig.USE_RXEN_ANT_PWR = true;	   // RXEN is used as power for antenna switch
	_hwConfig.USE_LDO = false;			   // LORA usage of LDO or DCDC power regulator (defaults to DCDC)

	TimerConfig();

	SX126xIoInit();

	// After power on the sync word should be 2414. 4434 could be possible on a restart
	// If we got something else, something is wrong.
	uint16_t readSyncWord = 0;
	SX126xReadRegisters(REG_LR_SYNCWORD, (uint8_t *)&readSyncWord, 2);

	LOG_LIB("BRD", "SyncWord = %04X", readSyncWord);

	if ((readSyncWord == 0x2414) || (readSyncWord == 0x4434))
	{
#if defined NRF52_SERIES || defined ESP32 || ARDUINO_ARCH_RP2040
		if (start_lora_task())
		{
			return 0;
		}
		else
		{
			return 1;
		}
#else
		return 0;
#endif
	}
	return 1;
}

uint32_t lora_rak11300_init(void)
{
	_hwConfig.CHIP_TYPE = SX1262;		   // Chip type, SX1261 or SX1262
	_hwConfig.PIN_LORA_SCLK = 10;		   // LORA SPI CLK
	_hwConfig.PIN_LORA_MOSI = 11;		   // LORA SPI MOSI
	_hwConfig.PIN_LORA_MISO = 12;		   // LORA SPI MISO
	_hwConfig.PIN_LORA_NSS = 13;		   // LORA SPI CS
	_hwConfig.PIN_LORA_RESET = 14;		   // LORA RESET
	_hwConfig.PIN_LORA_BUSY = 15;		   // LORA SPI BUSY
	_hwConfig.RADIO_TXEN = -1;			   // LORA ANTENNA TX ENABLE (e.g. eByte E22 module)
	_hwConfig.RADIO_RXEN = 25;			   // LORA ANTENNA RX ENABLE (e.g. eByte E22 module)
	_hwConfig.USE_DIO2_ANT_SWITCH = true;  // LORA DIO2 controls antenna
	_hwConfig.USE_DIO3_TCXO = true;		   // LORA DIO3 controls oscillator voltage (e.g. eByte E22 module)
	_hwConfig.USE_DIO3_ANT_SWITCH = false; // LORA DIO3 controls antenna (e.g. Insight SIP ISP4520 module)
	_hwConfig.PIN_LORA_DIO_1 = 29;		   // LORA DIO_1
	_hwConfig.USE_RXEN_ANT_PWR = true;	   // RXEN is used as power for antenna switch
#ifdef RAK11310_PROTO
	_hwConfig.USE_LDO = true; // True on RAK11300 prototypes because of DCDC regulator problem
#else
	_hwConfig.USE_LDO = false;
#endif
	LOG_LIB("BRD", "TimerConfig()");
	TimerConfig();

	LOG_LIB("BRD", "SX126xIoInit()");
	SX126xIoInit();

	// After power on the sync word should be 2414. 4434 could be possible on a restart
	// If we got something else, something is wrong.
	uint16_t readSyncWord = 0;
	LOG_LIB("BRD", "SX126xReadRegisters()");
	SX126xReadRegisters(REG_LR_SYNCWORD, (uint8_t *)&readSyncWord, 2);

	LOG_LIB("BRD", "SyncWord = %04X", readSyncWord);

	if ((readSyncWord == 0x2414) || (readSyncWord == 0x4434))
	{
		// If we are compiling for ESP32, nRF52 or RP2040 we start background task
#if defined NRF52_SERIES || defined ESP32 || ARDUINO_ARCH_RP2040
		if (start_lora_task())
		{
			return 0;
		}
		else
		{
			return 1;
		}
#else
		return 0;
#endif
	}
	return 1;
}

#ifndef WB_IO3
#define WB_IO3 -1
#endif
#ifndef WB_IO4
#define WB_IO4 -1
#endif
#ifndef WB_IO5
#define WB_IO5 -1
#endif
#ifndef WB_IO6
#define WB_IO6 -1
#endif

uint32_t lora_rak13300_init(void)
{
	_hwConfig.CHIP_TYPE = SX1262;		   // Chip type, SX1261 or SX1262
	_hwConfig.PIN_LORA_RESET = WB_IO4;	   // LORA RESET
	_hwConfig.PIN_LORA_NSS = SS;		   // LORA SPI CS
	_hwConfig.PIN_LORA_SCLK = SCK;		   // LORA SPI CLK
	_hwConfig.PIN_LORA_MISO = MISO;		   // LORA SPI MISO
	_hwConfig.PIN_LORA_DIO_1 = WB_IO6;	   // LORA DIO_1
	_hwConfig.PIN_LORA_BUSY = WB_IO5;	   // LORA SPI BUSY
	_hwConfig.PIN_LORA_MOSI = MOSI;		   // LORA SPI MOSI
	_hwConfig.RADIO_TXEN = -1;			   // LORA ANTENNA TX ENABLE (e.g. eByte E22 module)
	_hwConfig.RADIO_RXEN = WB_IO3;		   // LORA ANTENNA RX ENABLE (e.g. eByte E22 module)
	_hwConfig.USE_DIO2_ANT_SWITCH = true;  // LORA DIO2 controls antenna
	_hwConfig.USE_DIO3_TCXO = true;		   // LORA DIO3 controls oscillator voltage (e.g. eByte E22 module)
	_hwConfig.USE_DIO3_ANT_SWITCH = false; // LORA DIO3 controls antenna (e.g. Insight SIP ISP4520 module)
	_hwConfig.USE_RXEN_ANT_PWR = true;	   // RXEN is used as power for antenna switch

	TimerConfig();

	SX126xIoInit();

	// After power on the sync word should be 2414. 4434 could be possible on a restart
	// If we got something else, something is wrong.
	uint16_t readSyncWord = 0;
	SX126xReadRegisters(REG_LR_SYNCWORD, (uint8_t *)&readSyncWord, 2);

	LOG_LIB("BRD", "SyncWord = %04X", readSyncWord);

	if ((readSyncWord == 0x2414) || (readSyncWord == 0x4434))
	{
#if defined NRF52_SERIES || defined ESP32 || ARDUINO_ARCH_RP2040
		if (start_lora_task())
		{
			return 0;
		}
		else
		{
			return 1;
		}
#else
		return 0;
#endif
	}
	return 1;
}

#if defined NRF52_SERIES || defined ESP32
void _lora_task(void *pvParameters)
{
	LOG_LIB("BRD", "LoRa Task started");

	while (1)
	{
		if (xSemaphoreTake(_lora_sem, portMAX_DELAY) == pdTRUE)
		{
			// Handle Radio events
			Radio.BgIrqProcess();
		}
	}
}

bool start_lora_task(void)
{
	// Create the LoRaWan event semaphore
	_lora_sem = xSemaphoreCreateBinary();
	// Initialize semaphore
	xSemaphoreGive(_lora_sem);

	xSemaphoreTake(_lora_sem, 10);
#ifdef NRF52_SERIES
	if (!xTaskCreate(_lora_task, "LORA", 4096, NULL, TASK_PRIO_NORMAL, &_loraTaskHandle))
#else
	if (!xTaskCreate(_lora_task, "LORA", 4096, NULL, 1, &_loraTaskHandle))
#endif
	{
		return false;
	}
	return true;
}
#endif

#ifdef ARDUINO_ARCH_RP2040
#include <mbed.h>
#include <rtos.h>
using namespace rtos;
using namespace mbed;

/** The event handler thread */
Thread _thread_handle_lora(osPriorityAboveNormal, 4096);

/** Thread id for lora event thread */
osThreadId _lora_task_thread = NULL;

// Task to handle timer events
void _lora_task()
{
	_lora_task_thread = osThreadGetId();
	while (true)
	{
		// Wait for event
		osSignalWait(0x1, osWaitForever);

		// LOG_LIB("TIM", "LoRa IRQ");
		// Handle Radio events
		Radio.BgIrqProcess();

		yield();
	}
}

bool start_lora_task(void)
{
	_thread_handle_lora.start(_lora_task);
	_thread_handle_lora.set_priority(osPriorityAboveNormal);

	/// \todo how to detect that the task is really created
	return true;
}
#endif

void lora_hardware_uninit(void)
{
#if defined NRF52_SERIES || defined ESP32
	vTaskSuspend(_loraTaskHandle);

#endif
	SX126xIoDeInit();
}

// Below functions are mcu specific and declared within the mcu folder
uint32_t BoardGetRandomSeed(void);

void BoardGetUniqueId(uint8_t *id);

uint8_t BoardGetBatteryLevel(void);

void BoardDisableIrq(void);

void BoardEnableIrq(void);
