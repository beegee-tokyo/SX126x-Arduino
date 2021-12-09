/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: SX126x driver specific target board functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/

/******************************************************************************
 * @file    sx126x-board.c
 * @author  Insight SiP
 * @version V2.0.0
 * @date    30-january-2019
 * @brief   SX126x implementation compatible with LoraWan/semtech drivers.
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

#include <Arduino.h>
#include "boards/mcu/board.h"
#include "boards/mcu/spi_board.h"
#include "radio/sx126x/sx126x.h"
#include "sx126x-board.h"

SPISettings spiSettings = SPISettings(2000000, MSBFIRST, SPI_MODE0);

// No need to initialize DIO3 as output everytime, do it once and remember it
bool dio3IsOutput = false;

void SX126xIoInit(void)
{

	initSPI();

	dio3IsOutput = false;

	pinMode(_hwConfig.PIN_LORA_NSS, OUTPUT);
	digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);
	pinMode(_hwConfig.PIN_LORA_BUSY, INPUT);
	pinMode(_hwConfig.PIN_LORA_DIO_1, INPUT);
	pinMode(_hwConfig.PIN_LORA_RESET, OUTPUT);
	digitalWrite(_hwConfig.PIN_LORA_RESET, HIGH);

	// Use RADIO_RXEN as power for the antenna switch
	if (_hwConfig.USE_RXEN_ANT_PWR)
	{
		if (_hwConfig.RADIO_TXEN != -1)
			pinMode(_hwConfig.RADIO_TXEN, INPUT);
		pinMode(_hwConfig.RADIO_RXEN, OUTPUT);
		digitalWrite(_hwConfig.RADIO_RXEN, LOW);
	}
	// If both RADIO_TXEN and RADIO_RXEN is defined they control the direction of the antenna switch
	else if ((_hwConfig.RADIO_TXEN != -1) && (_hwConfig.RADIO_RXEN != -1))
	{
		pinMode(_hwConfig.RADIO_TXEN, OUTPUT);
		pinMode(_hwConfig.RADIO_RXEN, OUTPUT);
		SX126xRXena();
	}

	SX126xReset();
}

void SX126xIoReInit(void)
{

	initSPI();

	dio3IsOutput = false;

	pinMode(_hwConfig.PIN_LORA_NSS, OUTPUT);
	digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);
	pinMode(_hwConfig.PIN_LORA_BUSY, INPUT);
	pinMode(_hwConfig.PIN_LORA_DIO_1, INPUT);
	// pinMode(_hwConfig.PIN_LORA_RESET, OUTPUT);
	// digitalWrite(_hwConfig.PIN_LORA_RESET, HIGH);

	// If only RADIO_RXEN is defined, it is the power control for the antenna switch
	if (_hwConfig.USE_RXEN_ANT_PWR)
	{
		if (_hwConfig.RADIO_TXEN != -1)
			pinMode(_hwConfig.RADIO_TXEN, INPUT);
		pinMode(_hwConfig.RADIO_RXEN, OUTPUT);
		digitalWrite(_hwConfig.RADIO_RXEN, LOW);
	}
	// If both RADIO_TXEN and RADIO_RXEN is defined they control the direction of the antenna switch
	else if ((_hwConfig.RADIO_TXEN != -1) && (_hwConfig.RADIO_RXEN != -1))
	{
		pinMode(_hwConfig.RADIO_TXEN, OUTPUT);
		pinMode(_hwConfig.RADIO_RXEN, OUTPUT);
		SX126xRXena();
	}
}

void SX126xIoIrqInit(DioIrqHandler dioIrq)
{
	attachInterrupt(_hwConfig.PIN_LORA_DIO_1, dioIrq, RISING);
}

void SX126xIoDeInit(void)
{
	dio3IsOutput = false;
	detachInterrupt(_hwConfig.PIN_LORA_DIO_1);
	pinMode(_hwConfig.PIN_LORA_NSS, INPUT);
	pinMode(_hwConfig.PIN_LORA_BUSY, INPUT);
	pinMode(_hwConfig.PIN_LORA_DIO_1, INPUT);
	pinMode(_hwConfig.PIN_LORA_RESET, INPUT);
}

void SX126xReset(void)
{
	pinMode(_hwConfig.PIN_LORA_RESET, OUTPUT);
	digitalWrite(_hwConfig.PIN_LORA_RESET, LOW);
	delay(10);
	digitalWrite(_hwConfig.PIN_LORA_RESET, HIGH);
	delay(20);
	dio3IsOutput = false;
}

void SX126xWaitOnBusy(void)
{
	int timeout = 1000;
	while (digitalRead(_hwConfig.PIN_LORA_BUSY) == HIGH)
	{
		delay(1);
		timeout -= 1;
		if (timeout < 0)
		{
			/// \todo This error should be reported to the main app
			LOG_LIB("LORA", "[SX126xWaitOnBusy] Timeout waiting for BUSY low");
			return;
		}
	}
}

void SX126xWakeup(void)
{
	dio3IsOutput = false;
	BoardDisableIrq();

	digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);

	SPI_LORA.beginTransaction(spiSettings);
	SPI_LORA.transfer(RADIO_GET_STATUS);
	SPI_LORA.transfer(0x00);
	SPI_LORA.endTransaction();
	digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

	// Wait for chip to be ready.
	SX126xWaitOnBusy();

	BoardEnableIrq();
}

void SX126xWriteCommand(RadioCommands_t command, uint8_t *buffer, uint16_t size)
{
	SX126xCheckDeviceReady();

	digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);

	SPI_LORA.beginTransaction(spiSettings);
	SPI_LORA.transfer((uint8_t)command);

	for (uint16_t i = 0; i < size; i++)
	{
		SPI_LORA.transfer(buffer[i]);
	}

	SPI_LORA.endTransaction();
	digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

	if (command != RADIO_SET_SLEEP)
	{
		SX126xWaitOnBusy();
	}
}

void SX126xReadCommand(RadioCommands_t command, uint8_t *buffer, uint16_t size)
{
	SX126xCheckDeviceReady();

	digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);

	SPI_LORA.beginTransaction(spiSettings);
	SPI_LORA.transfer((uint8_t)command);
	SPI_LORA.transfer(0x00);
	for (uint16_t i = 0; i < size; i++)
	{
		buffer[i] = SPI_LORA.transfer(0x00);
	}

	SPI_LORA.endTransaction();
	digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

	SX126xWaitOnBusy();
}

void SX126xWriteRegisters(uint16_t address, uint8_t *buffer, uint16_t size)
{
	SX126xCheckDeviceReady();

	digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);

	SPI_LORA.beginTransaction(spiSettings);
	SPI_LORA.transfer(RADIO_WRITE_REGISTER);
	SPI_LORA.transfer((address & 0xFF00) >> 8);
	SPI_LORA.transfer(address & 0x00FF);

	for (uint16_t i = 0; i < size; i++)
	{
		SPI_LORA.transfer(buffer[i]);
	}

	SPI_LORA.endTransaction();
	digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

	SX126xWaitOnBusy();
}

void SX126xWriteRegister(uint16_t address, uint8_t value)
{
	SX126xWriteRegisters(address, &value, 1);
}

void SX126xReadRegisters(uint16_t address, uint8_t *buffer, uint16_t size)
{
	SX126xCheckDeviceReady();

	digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);

	SPI_LORA.beginTransaction(spiSettings);
	SPI_LORA.transfer(RADIO_READ_REGISTER);
	SPI_LORA.transfer((address & 0xFF00) >> 8);
	SPI_LORA.transfer(address & 0x00FF);
	SPI_LORA.transfer(0x00);
	for (uint16_t i = 0; i < size; i++)
	{
		buffer[i] = SPI_LORA.transfer(0x00);
	}
	SPI_LORA.endTransaction();
	digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

	SX126xWaitOnBusy();
}

uint8_t SX126xReadRegister(uint16_t address)
{
	uint8_t data;
	SX126xReadRegisters(address, &data, 1);
	return data;
}

void SX126xWriteBuffer(uint8_t offset, uint8_t *buffer, uint8_t size)
{
	SX126xCheckDeviceReady();

	digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);

	SPI_LORA.beginTransaction(spiSettings);
	SPI_LORA.transfer(RADIO_WRITE_BUFFER);
	SPI_LORA.transfer(offset);
	for (uint16_t i = 0; i < size; i++)
	{
		SPI_LORA.transfer(buffer[i]);
	}
	SPI_LORA.endTransaction();
	digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

	SX126xWaitOnBusy();
}

void SX126xReadBuffer(uint8_t offset, uint8_t *buffer, uint8_t size)
{
	SX126xCheckDeviceReady();

	digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);

	SPI_LORA.beginTransaction(spiSettings);
	SPI_LORA.transfer(RADIO_READ_BUFFER);
	SPI_LORA.transfer(offset);
	SPI_LORA.transfer(0x00);
	for (uint16_t i = 0; i < size; i++)
	{
		buffer[i] = SPI_LORA.transfer(0x00);
	}
	SPI_LORA.endTransaction();
	digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

	SX126xWaitOnBusy();
}

void SX126xSetRfTxPower(int8_t power)
{
	SX126xSetTxParams(power, RADIO_RAMP_40_US);
}

uint8_t SX126xGetPaSelect(uint32_t channel)
{
	if (_hwConfig.CHIP_TYPE == SX1262_CHIP)
	{
		return SX1262;
	}
	else
	{
		return SX1261;
	}
}

static void SX126xDio3Control(bool state)
{
	uint8_t reg_0x0580;
	uint8_t reg_0x0583;
	uint8_t reg_0x0584;
	uint8_t reg_0x0585;
	uint8_t reg_0x0920;

	if (!dio3IsOutput)
	{
		// Configure DIO3 as output

		// Read 0x0580
		SX126xWaitOnBusy();
		digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);
		SPI_LORA.beginTransaction(spiSettings);
		SPI_LORA.transfer(RADIO_READ_REGISTER);
		SPI_LORA.transfer((0x0580 & 0xFF00) >> 8);
		SPI_LORA.transfer(0x0580 & 0x00FF);
		SPI_LORA.transfer(0x00);
		reg_0x0580 = SPI_LORA.transfer(0x00);
		SPI_LORA.endTransaction();
		digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

		// Read 0x0583
		SX126xWaitOnBusy();
		digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);
		SPI_LORA.beginTransaction(spiSettings);
		SPI_LORA.transfer(RADIO_READ_REGISTER);
		SPI_LORA.transfer((0x0583 & 0xFF00) >> 8);
		SPI_LORA.transfer(0x0583 & 0x00FF);
		SPI_LORA.transfer(0x00);
		reg_0x0583 = SPI_LORA.transfer(0x00);
		SPI_LORA.endTransaction();
		digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

		// Read 0x0584
		SX126xWaitOnBusy();
		digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);
		SPI_LORA.beginTransaction(spiSettings);
		SPI_LORA.transfer(RADIO_READ_REGISTER);
		SPI_LORA.transfer((0x0584 & 0xFF00) >> 8);
		SPI_LORA.transfer(0x0584 & 0x00FF);
		SPI_LORA.transfer(0x00);
		reg_0x0584 = SPI_LORA.transfer(0x00);
		SPI_LORA.endTransaction();
		digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

		// Read 0x0585
		SX126xWaitOnBusy();
		digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);
		SPI_LORA.beginTransaction(spiSettings);
		SPI_LORA.transfer(RADIO_READ_REGISTER);
		SPI_LORA.transfer((0x0585 & 0xFF00) >> 8);
		SPI_LORA.transfer(0x0585 & 0x00FF);
		SPI_LORA.transfer(0x00);
		reg_0x0585 = SPI_LORA.transfer(0x00);
		SPI_LORA.endTransaction();
		digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

		// Write 0x0580
		// SX126xWriteRegister(0x0580, reg_0x0580 | 0x08);
		SX126xWaitOnBusy();
		digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);
		SPI_LORA.beginTransaction(spiSettings);
		SPI_LORA.transfer(RADIO_WRITE_REGISTER);
		SPI_LORA.transfer((0x0580 & 0xFF00) >> 8);
		SPI_LORA.transfer(0x0580 & 0x00FF);
		SPI_LORA.transfer(reg_0x0580 | 0x08);
		SPI_LORA.endTransaction();
		digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

		// Write 0x0583
		SX126xWaitOnBusy();
		digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);
		SPI_LORA.beginTransaction(spiSettings);
		SPI_LORA.transfer(RADIO_WRITE_REGISTER);
		SPI_LORA.transfer((0x0583 & 0xFF00) >> 8);
		SPI_LORA.transfer(0x0583 & 0x00FF);
		SPI_LORA.transfer(reg_0x0583 & ~0x08);
		SPI_LORA.endTransaction();
		digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

		// Write 0x0584
		SX126xWaitOnBusy();
		digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);
		SPI_LORA.beginTransaction(spiSettings);
		SPI_LORA.transfer(RADIO_WRITE_REGISTER);
		SPI_LORA.transfer((0x0584 & 0xFF00) >> 8);
		SPI_LORA.transfer(0x0584 & 0x00FF);
		SPI_LORA.transfer(reg_0x0584 & ~0x08);
		SPI_LORA.endTransaction();
		digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

		// Write 0x0585
		SX126xWaitOnBusy();
		digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);
		SPI_LORA.beginTransaction(spiSettings);
		SPI_LORA.transfer(RADIO_WRITE_REGISTER);
		SPI_LORA.transfer((0x0585 & 0xFF00) >> 8);
		SPI_LORA.transfer(0x0585 & 0x00FF);
		SPI_LORA.transfer(reg_0x0585 & ~0x08);
		SPI_LORA.endTransaction();
		digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

		// Write 0x0920
		SX126xWaitOnBusy();
		digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);
		SPI_LORA.beginTransaction(spiSettings);
		SPI_LORA.transfer(RADIO_WRITE_REGISTER);
		SPI_LORA.transfer((0x0920 & 0xFF00) >> 8);
		SPI_LORA.transfer(0x0920 & 0x00FF);
		SPI_LORA.transfer(0x06);
		SPI_LORA.endTransaction();
		digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

		dio3IsOutput = true;
	}

	if (state)
	{
		// Set DIO3 High
		SX126xWaitOnBusy();
		digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);
		SPI_LORA.beginTransaction(spiSettings);
		SPI_LORA.transfer(RADIO_READ_REGISTER);
		SPI_LORA.transfer((0x0920 & 0xFF00) >> 8);
		SPI_LORA.transfer(0x0920 & 0x00FF);
		SPI_LORA.transfer(0x00);
		reg_0x0920 = SPI_LORA.transfer(0x00);
		SPI_LORA.endTransaction();
		digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

		SX126xWaitOnBusy();
		digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);
		SPI_LORA.beginTransaction(spiSettings);
		SPI_LORA.transfer(RADIO_WRITE_REGISTER);
		SPI_LORA.transfer((0x0920 & 0xFF00) >> 8);
		SPI_LORA.transfer(0x0920 & 0x00FF);
		SPI_LORA.transfer(reg_0x0920 | 0x08);
		SPI_LORA.endTransaction();
		digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);
	}
	else
	{
		// Set DIO3 Low
		SX126xWaitOnBusy();
		digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);
		SPI_LORA.beginTransaction(spiSettings);
		SPI_LORA.transfer(RADIO_READ_REGISTER);
		SPI_LORA.transfer((0x0920 & 0xFF00) >> 8);
		SPI_LORA.transfer(0x0920 & 0x00FF);
		SPI_LORA.transfer(0x00);
		reg_0x0920 = SPI_LORA.transfer(0x00);
		SPI_LORA.endTransaction();
		digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);

		SX126xWaitOnBusy();
		digitalWrite(_hwConfig.PIN_LORA_NSS, LOW);
		SPI_LORA.beginTransaction(spiSettings);
		SPI_LORA.transfer(RADIO_WRITE_REGISTER);
		SPI_LORA.transfer((0x0920 & 0xFF00) >> 8);
		SPI_LORA.transfer(0x0920 & 0x00FF);
		SPI_LORA.transfer(reg_0x0920 & ~0x08);
		SPI_LORA.endTransaction();
		digitalWrite(_hwConfig.PIN_LORA_NSS, HIGH);
	}
}

void SX126xAntSwOn(void)
{
	// Use if DIO3 is used as antenna switch power control
	if (_hwConfig.USE_DIO3_ANT_SWITCH)
	{
		SX126xDio3Control(true);
	}

	// Use if RADIO_RXEN is used as antenna switch power control
	if (_hwConfig.USE_RXEN_ANT_PWR)
	{
		digitalWrite(_hwConfig.RADIO_RXEN, HIGH);
	}
}

void SX126xAntSwOff(void)
{
	// Use if DIO3 is used as antenna switch power control
	if (_hwConfig.USE_DIO3_ANT_SWITCH)
	{
		SX126xDio3Control(false);
	}
	// Use if RADIO_RXEN is used as antenna switch power control
	if (_hwConfig.USE_RXEN_ANT_PWR)
	{
		digitalWrite(_hwConfig.RADIO_RXEN, LOW);
	}
}

void SX126xRXena(void)
{
	if (!_hwConfig.USE_RXEN_ANT_PWR)
	{
		if ((_hwConfig.RADIO_RXEN != -1) && (_hwConfig.RADIO_TXEN != -1))
		{
			digitalWrite(_hwConfig.RADIO_RXEN, HIGH);
			digitalWrite(_hwConfig.RADIO_TXEN, LOW);
		}
	}
	else
	{
		digitalWrite(_hwConfig.RADIO_RXEN, HIGH);
	}
}

void SX126xTXena(void)
{
	if (!_hwConfig.USE_RXEN_ANT_PWR)
	{
		if ((_hwConfig.RADIO_RXEN != -1) && (_hwConfig.RADIO_TXEN != -1))
		{
			digitalWrite(_hwConfig.RADIO_RXEN, LOW);
			digitalWrite(_hwConfig.RADIO_TXEN, HIGH);
		}
	}
	else
	{
		digitalWrite(_hwConfig.RADIO_RXEN, HIGH);
	}
}

bool SX126xCheckRfFrequency(uint32_t frequency)
{
	// Implement check. Currently all frequencies are supported
	return true;
}

void SX126xGetStats(uint16_t *nb_pkt_received, uint16_t *nb_pkt_crc_error, uint16_t *nb_pkt_length_error)
{
	uint8_t buf[6];

	SX126xReadCommand(RADIO_GET_STATS, buf, 6);

	*nb_pkt_received = (buf[0] << 8) | buf[1];
	*nb_pkt_crc_error = (buf[2] << 8) | buf[3];
	*nb_pkt_length_error = (buf[4] << 8) | buf[5];
}

void SX126xResetStats(void)
{
	uint8_t buf[6] = {0x00};

	SX126xWriteCommand(RADIO_RESET_STATS, buf, 6);
}
