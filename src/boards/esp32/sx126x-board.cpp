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
#include "board.h"
//#include "radio.h" // Not needed
#include "radio/sx126x/sx126x.h"
#include "sx126x-board.h"

extern "C"
{
	SPISettings spiSettings = SPISettings(2000000, MSBFIRST, SPI_MODE0);

	void SX126xIoInit(void)
	{
		pinMode(PIN_LORA_NSS, OUTPUT);
		digitalWrite(PIN_LORA_NSS, HIGH);
		pinMode(PIN_LORA_BUSY, INPUT);
		pinMode(PIN_LORA_DIO_1, INPUT);
		pinMode(PIN_LORA_RESET, OUTPUT);
		digitalWrite(PIN_LORA_RESET, HIGH);

		// GpioInit(&SX126x.Spi.Nss,	PIN_LORA_NSS,		PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1);
		// GpioInit(&SX126x.BUSY, 	PIN_LORA_BUSY,		PIN_INPUT,  PIN_PUSH_PULL, PIN_NO_PULL, 0);
		// GpioInit(&SX126x.DIO1,	PIN_LORA_DIO_1, 	PIN_INPUT,  PIN_PUSH_PULL, PIN_NO_PULL, 0);
	}

	void SX126xIoIrqInit(DioIrqHandler dioIrq)
	{
		attachInterrupt(PIN_LORA_DIO_1, dioIrq, RISING);
		// GpioSetInterrupt(&SX126x.DIO1, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, dioIrq);
	}

	void SX126xIoDeInit(void)
	{
		detachInterrupt(PIN_LORA_DIO_1);
		pinMode(PIN_LORA_NSS, INPUT);
		pinMode(PIN_LORA_BUSY, INPUT);
		pinMode(PIN_LORA_DIO_1, INPUT);
		pinMode(PIN_LORA_RESET, INPUT);
		// GpioInit(&SX126x.Spi.Nss,	PIN_LORA_NSS,		PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
		// GpioInit(&SX126x.BUSY,	PIN_LORA_BUSY,		PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
		// GpioInit(&SX126x.DIO1,	PIN_LORA_DIO_1, 	PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
	}

	void SX126xReset(void)
	{
		DelayMs(10);
		digitalWrite(PIN_LORA_RESET, LOW);
		// GpioInit(&SX126x.Reset, PIN_LORA_RESET, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
		DelayMs(20);
		digitalWrite(PIN_LORA_RESET, HIGH);
		// GpioInit(&SX126x.Reset, PIN_LORA_RESET, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0); // internal pull-up
		DelayMs(10);
	}

	void SX126xWaitOnBusy(void)
	{
		// TODO this can lead to an endless loop !!!! Better to add a timeout
		//  while (digitalRead(PIN_LORA_BUSY) == HIGH);

		// TODO solution with timeout, waits 1000 x 1ms for BUSY to go low
		int timeout = 10000;
		while (digitalRead(PIN_LORA_BUSY) == HIGH)
		{
			DelayMs(1);
			timeout -= 1;
			if (timeout < 0)
			{
				log_e("Timeout waiting for BUSY low");
				return;
			}
		}
	}

	void SX126xWakeup(void)
	{
		BoardDisableIrq();

		digitalWrite(PIN_LORA_NSS, LOW);
		// GpioWrite(&SX126x.Spi.Nss, 0);

		SPI.beginTransaction(spiSettings);
		// SpiInOut(&SX126x.Spi, RADIO_GET_STATUS);
		// SpiInOut(&SX126x.Spi, 0x00);
		SPI.endTransaction();
		digitalWrite(PIN_LORA_NSS, HIGH);
		// GpioWrite(&SX126x.Spi.Nss, 1);

		// Wait for chip to be ready.
		SX126xWaitOnBusy();

		BoardEnableIrq();
	}

	void SX126xWriteCommand(RadioCommands_t command, uint8_t *buffer, uint16_t size)
	{
		SX126xCheckDeviceReady();

		digitalWrite(PIN_LORA_NSS, LOW);
		// GpioWrite(&SX126x.Spi.Nss, 0);

		SPI.beginTransaction(spiSettings);
		SPI.transfer((uint8_t)command);
		// SpiInOut(&SX126x.Spi, (uint8_t)command);

		for (uint16_t i = 0; i < size; i++)
		{
			SPI.transfer(buffer[i]);
			// SpiInOut(&SX126x.Spi, buffer[i]);
		}

		SPI.endTransaction();
		digitalWrite(PIN_LORA_NSS, HIGH);
		// GpioWrite(&SX126x.Spi.Nss, 1);

		if (command != RADIO_SET_SLEEP)
		{
			SX126xWaitOnBusy();
		}
	}

	void SX126xReadCommand(RadioCommands_t command, uint8_t *buffer, uint16_t size)
	{
		SX126xCheckDeviceReady();

		digitalWrite(PIN_LORA_NSS, LOW);
		// GpioWrite(&SX126x.Spi.Nss, 0);

		SPI.beginTransaction(spiSettings);
		SPI.transfer((uint8_t)command);
		SPI.transfer(0x00);
		// SpiInOut(&SX126x.Spi, (uint8_t)command);
		// SpiInOut(&SX126x.Spi, 0x00);
		for (uint16_t i = 0; i < size; i++)
		{
			buffer[i] = SPI.transfer(0x00);
			// buffer[i] = SpiInOut(&SX126x.Spi, 0);
		}

		SPI.endTransaction();
		digitalWrite(PIN_LORA_NSS, HIGH);
		// GpioWrite(&SX126x.Spi.Nss, 1);

		SX126xWaitOnBusy();
	}

	void SX126xWriteRegisters(uint16_t address, uint8_t *buffer, uint16_t size)
	{
		SX126xCheckDeviceReady();

		digitalWrite(PIN_LORA_NSS, LOW);
		// GpioWrite(&SX126x.Spi.Nss, 0);

		SPI.beginTransaction(spiSettings);
		SPI.transfer(RADIO_WRITE_REGISTER);
		SPI.transfer((address & 0xFF00) >> 8);
		SPI.transfer(address & 0x00FF);
		// SpiInOut(&SX126x.Spi, RADIO_WRITE_REGISTER);
		// SpiInOut(&SX126x.Spi, (address & 0xFF00) >> 8);
		// SpiInOut(&SX126x.Spi, address & 0x00FF);

		for (uint16_t i = 0; i < size; i++)
		{
			SPI.transfer(buffer[i]);
			// SpiInOut(&SX126x.Spi, buffer[i]);
		}

		SPI.endTransaction();
		digitalWrite(PIN_LORA_NSS, HIGH);
		// GpioWrite (&SX126x.Spi.Nss, 1);

		SX126xWaitOnBusy();
	}

	void SX126xWriteRegister(uint16_t address, uint8_t value)
	{
		SX126xWriteRegisters(address, &value, 1);
	}

	void SX126xReadRegisters(uint16_t address, uint8_t *buffer, uint16_t size)
	{
		SX126xCheckDeviceReady();

		digitalWrite(PIN_LORA_NSS, LOW);
		// GpioWrite(&SX126x.Spi.Nss, 0);

		SPI.beginTransaction(spiSettings);
		SPI.transfer(RADIO_READ_REGISTER);
		SPI.transfer((address & 0xFF00) >> 8);
		SPI.transfer(address & 0x00FF);
		SPI.transfer(0x00);
		// SpiInOut(&SX126x.Spi, RADIO_READ_REGISTER);
		// SpiInOut(&SX126x.Spi, (address & 0xFF00) >> 8);
		// SpiInOut(&SX126x.Spi, address & 0x00FF);
		// SpiInOut(&SX126x.Spi, 0);
		for (uint16_t i = 0; i < size; i++)
		{
			buffer[i] = SPI.transfer(0x00);
			// buffer[i] = SpiInOut(&SX126x.Spi, 0);
		}
		SPI.endTransaction();
		digitalWrite(PIN_LORA_NSS, HIGH);
		// GpioWrite(&SX126x.Spi.Nss, 1);

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

		digitalWrite(PIN_LORA_NSS, LOW);
		// GpioWrite(&SX126x.Spi.Nss, 0);

		SPI.beginTransaction(spiSettings);
		SPI.transfer(RADIO_WRITE_BUFFER);
		SPI.transfer(offset);
		// SpiInOut(&SX126x.Spi, RADIO_WRITE_BUFFER);
		// SpiInOut(&SX126x.Spi, offset);
		for (uint16_t i = 0; i < size; i++)
		{
			SPI.transfer(buffer[i]);
			// SpiInOut(&SX126x.Spi, buffer[i]);
		}
		SPI.endTransaction();
		digitalWrite(PIN_LORA_NSS, HIGH);
		// GpioWrite(&SX126x.Spi.Nss, 1);

		SX126xWaitOnBusy();
	}

	void SX126xReadBuffer(uint8_t offset, uint8_t *buffer, uint8_t size)
	{
		SX126xCheckDeviceReady();

		digitalWrite(PIN_LORA_NSS, LOW);
		// GpioWrite(&SX126x.Spi.Nss, 0);

		SPI.beginTransaction(spiSettings);
		SPI.transfer(RADIO_READ_BUFFER);
		SPI.transfer(offset);
		SPI.transfer(0x00);
		// SpiInOut(&SX126x.Spi, RADIO_READ_BUFFER);
		// SpiInOut(&SX126x.Spi, offset);
		// SpiInOut(&SX126x.Spi, 0);
		for (uint16_t i = 0; i < size; i++)
		{
			buffer[i] = SPI.transfer(0x00);
			// buffer[i] = SpiInOut(&SX126x.Spi, 0);
		}
		SPI.endTransaction();
		digitalWrite(PIN_LORA_NSS, HIGH);
		// GpioWrite(&SX126x.Spi.Nss, 1);

		SX126xWaitOnBusy();
	}

	void SX126xSetRfTxPower(int8_t power)
	{
		SX126xSetTxParams(power, RADIO_RAMP_40_US);
	}

	uint8_t SX126xGetPaSelect(uint32_t channel)
	{
#if defined(SX1261_CHIP)
		return SX1261;
#elif defined(SX1262_CHIP)
		return SX1262;
#endif
		return SX1262;
	}

	static void SX126xDio3Control(bool state)
	{
		uint8_t reg_0x0580;
		uint8_t reg_0x0583;
		uint8_t reg_0x0584;
		uint8_t reg_0x0585;
		uint8_t reg_0x0920;

		if (state)
		{
			// Configure DIO3 as output

			// Read 0x0580
			// reg_0x0580 = SX126xReadRegister(0x0580);
			SX126xWaitOnBusy();
			digitalWrite(PIN_LORA_NSS, LOW);
			SPI.beginTransaction(spiSettings);
			SPI.transfer(RADIO_READ_REGISTER);
			SPI.transfer((0x0580 & 0xFF00) >> 8);
			SPI.transfer(0x0580 & 0x00FF);
			SPI.transfer(0x00);
			reg_0x0580 = SPI.transfer(0x00);
			SPI.endTransaction();
			digitalWrite(PIN_LORA_NSS, HIGH);

			// Read 0x0583
			// reg_0x0583 = SX126xReadRegister(0x0583);
			SX126xWaitOnBusy();
			digitalWrite(PIN_LORA_NSS, LOW);
			SPI.beginTransaction(spiSettings);
			SPI.transfer(RADIO_READ_REGISTER);
			SPI.transfer((0x0583 & 0xFF00) >> 8);
			SPI.transfer(0x0583 & 0x00FF);
			SPI.transfer(0x00);
			reg_0x0583 = SPI.transfer(0x00);
			SPI.endTransaction();
			digitalWrite(PIN_LORA_NSS, HIGH);

			// Read 0x0584
			// reg_0x0584 = SX126xReadRegister(0x0584);
			SX126xWaitOnBusy();
			digitalWrite(PIN_LORA_NSS, LOW);
			SPI.beginTransaction(spiSettings);
			SPI.transfer(RADIO_READ_REGISTER);
			SPI.transfer((0x0584 & 0xFF00) >> 8);
			SPI.transfer(0x0584 & 0x00FF);
			SPI.transfer(0x00);
			reg_0x0584 = SPI.transfer(0x00);
			SPI.endTransaction();
			digitalWrite(PIN_LORA_NSS, HIGH);

			// Read 0x0585
			// reg_0x0585 = SX126xReadRegister(0x0585);
			SX126xWaitOnBusy();
			digitalWrite(PIN_LORA_NSS, LOW);
			SPI.beginTransaction(spiSettings);
			SPI.transfer(RADIO_READ_REGISTER);
			SPI.transfer((0x0585 & 0xFF00) >> 8);
			SPI.transfer(0x0585 & 0x00FF);
			SPI.transfer(0x00);
			reg_0x0585 = SPI.transfer(0x00);
			SPI.endTransaction();
			digitalWrite(PIN_LORA_NSS, HIGH);

			// Write 0x0580
			// SX126xWriteRegister(0x0580, reg_0x0580 | 0x08);
			SX126xWaitOnBusy();
			digitalWrite(PIN_LORA_NSS, LOW);
			SPI.beginTransaction(spiSettings);
			SPI.transfer(RADIO_WRITE_REGISTER);
			SPI.transfer((0x0580 & 0xFF00) >> 8);
			SPI.transfer(0x0580 & 0x00FF);
			SPI.transfer(reg_0x0580 | 0x08);
			SPI.endTransaction();
			digitalWrite(PIN_LORA_NSS, HIGH);

			// Write 0x0583
			// SX126xWriteRegister(0x0583, reg_0x0583 & ~0x08);
			SX126xWaitOnBusy();
			digitalWrite(PIN_LORA_NSS, LOW);
			SPI.beginTransaction(spiSettings);
			SPI.transfer(RADIO_WRITE_REGISTER);
			SPI.transfer((0x0583 & 0xFF00) >> 8);
			SPI.transfer(0x0583 & 0x00FF);
			SPI.transfer(reg_0x0583 & ~0x08);
			SPI.endTransaction();
			digitalWrite(PIN_LORA_NSS, HIGH);

			// Write 0x0584
			// SX126xWriteRegister(0x0584, reg_0x0584 & ~0x08);
			SX126xWaitOnBusy();
			digitalWrite(PIN_LORA_NSS, LOW);
			SPI.beginTransaction(spiSettings);
			SPI.transfer(RADIO_WRITE_REGISTER);
			SPI.transfer((0x0584 & 0xFF00) >> 8);
			SPI.transfer(0x0584 & 0x00FF);
			SPI.transfer(reg_0x0584 & ~0x08);
			SPI.endTransaction();
			digitalWrite(PIN_LORA_NSS, HIGH);

			// Write 0x0585
			// SX126xWriteRegister(0x0585, reg_0x0585 & ~0x08);
			SX126xWaitOnBusy();
			digitalWrite(PIN_LORA_NSS, LOW);
			SPI.beginTransaction(spiSettings);
			SPI.transfer(RADIO_WRITE_REGISTER);
			SPI.transfer((0x0585 & 0xFF00) >> 8);
			SPI.transfer(0x0585 & 0x00FF);
			SPI.transfer(reg_0x0585 & ~0x08);
			SPI.endTransaction();
			digitalWrite(PIN_LORA_NSS, HIGH);

			// Write 0x0920
			// SX126xWriteRegister(0x0920, 0x06);
			SX126xWaitOnBusy();
			digitalWrite(PIN_LORA_NSS, LOW);
			SPI.beginTransaction(spiSettings);
			SPI.transfer(RADIO_WRITE_REGISTER);
			SPI.transfer((0x0920 & 0xFF00) >> 8);
			SPI.transfer(0x0920 & 0x00FF);
			SPI.transfer(0x06);
			SPI.endTransaction();
			digitalWrite(PIN_LORA_NSS, HIGH);

			// Set DIO3 High
			// reg_0x0920 = SX126xReadRegister(0x0920);
			SX126xWaitOnBusy();
			digitalWrite(PIN_LORA_NSS, LOW);
			SPI.beginTransaction(spiSettings);
			SPI.transfer(RADIO_READ_REGISTER);
			SPI.transfer((0x0920 & 0xFF00) >> 8);
			SPI.transfer(0x0920 & 0x00FF);
			SPI.transfer(0x00);
			reg_0x0920 = SPI.transfer(0x00);
			SPI.endTransaction();
			digitalWrite(PIN_LORA_NSS, HIGH);

			// SX126xWriteRegister(0x0920, reg_0x0920 | 0x08);
			SX126xWaitOnBusy();
			digitalWrite(PIN_LORA_NSS, LOW);
			SPI.beginTransaction(spiSettings);
			SPI.transfer(RADIO_WRITE_REGISTER);
			SPI.transfer((0x0920 & 0xFF00) >> 8);
			SPI.transfer(0x0920 & 0x00FF);
			SPI.transfer(reg_0x0920 | 0x08);
			SPI.endTransaction();
			digitalWrite(PIN_LORA_NSS, HIGH);
		}
		else
		{
			// Set DIO3 Low
			// reg_0x0920 = SX126xReadRegister(0x0920);
			SX126xWaitOnBusy();
			digitalWrite(PIN_LORA_NSS, LOW);
			SPI.beginTransaction(spiSettings);
			SPI.transfer(RADIO_READ_REGISTER);
			SPI.transfer((0x0920 & 0xFF00) >> 8);
			SPI.transfer(0x0920 & 0x00FF);
			SPI.transfer(0x00);
			reg_0x0920 = SPI.transfer(0x00);
			SPI.endTransaction();
			digitalWrite(PIN_LORA_NSS, HIGH);

			// SX126xWriteRegister(0x0920, reg_0x0920 & ~0x08);
			SX126xWaitOnBusy();
			digitalWrite(PIN_LORA_NSS, LOW);
			SPI.beginTransaction(spiSettings);
			SPI.transfer(RADIO_WRITE_REGISTER);
			SPI.transfer((0x0920 & 0xFF00) >> 8);
			SPI.transfer(0x0920 & 0x00FF);
			SPI.transfer(reg_0x0920 & ~0x08);
			SPI.endTransaction();
			digitalWrite(PIN_LORA_NSS, HIGH);
		}
	}

	void SX126xAntSwOn(void)
	{
#ifdef DSP4520
		SX126xDio3Control(true);
#endif
	}

	void SX126xAntSwOff(void)
	{
#ifdef DSP4520
		SX126xDio3Control(false);
#endif
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
};