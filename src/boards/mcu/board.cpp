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
extern "C"
{

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

		TimerConfig();

		SX126xIoInit();

		return 0;
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
		_hwConfig.USE_DIO3_TCXO = false;	  // LORA DIO3 controls oscillator voltage (e.g. eByte E22 module)
		_hwConfig.USE_DIO3_ANT_SWITCH = true; // LORA DIO3 controls antenna (e.g. Insight SIP ISP4520 module)

		TimerConfig();

		SX126xIoInit();

		return 0;
	}

	void lora_hardware_uninit(void)
	{
		SX126xIoDeInit();
	}

	// Below functions are mcu specific and declared within the mcu folder
	uint32_t BoardGetRandomSeed(void);

	void BoardGetUniqueId(uint8_t *id);

	uint8_t BoardGetBatteryLevel(void);

	void BoardDisableIrq(void);

	void BoardEnableIrq(void);
};