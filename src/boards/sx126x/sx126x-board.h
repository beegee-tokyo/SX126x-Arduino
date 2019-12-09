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
 * @file    sx126x-board.h
 * @author  Insight SiP
 * @version V2.0.0
 * @date    30-january-2019
 * @brief   SX126x header compatible with LoraWan/semtech drivers.
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

#ifndef __SX126x_ARCH_H__
#define __SX126x_ARCH_H__

#include <Arduino.h>

extern "C"
{
	/**@brief Initializes the radio I/Os pins interface
 */
	void SX126xIoInit(void);

	/**@brief Initializes the radio I/Os pins interface after deep sleep wake
 */
	void SX126xIoReInit(void);

	/**@brief Initializes DIO IRQ handlers
 *
 * \param [IN] irqHandlers Array containing the IRQ callback functions
 */
	void SX126xIoIrqInit(DioIrqHandler dioIrq);

	/**@brief De-initializes the radio I/Os pins interface.
 *
 * \remark Useful when going in MCU low power modes
 */
	void SX126xIoDeInit(void);

	/**@brief HW Reset of the radio
 */
	void SX126xReset(void);

	/**@brief Blocking loop to wait while the Busy pin in high
 */
	void SX126xWaitOnBusy(void);

	/**@brief Wakes up the radio
 */
	void SX126xWakeup(void);

	/**@brief Send a command that write data to the radio
 *
 * \param [in]  opcode        Opcode of the command
 * \param [in]  buffer        Buffer to be send to the radio
 * \param [in]  size          Size of the buffer to send
 */
	void SX126xWriteCommand(RadioCommands_t opcode, uint8_t *buffer, uint16_t size);

	/**@brief Send a command that read data from the radio
 *
 * \param [in]  opcode        Opcode of the command
 * \param [out] buffer        Buffer holding data from the radio
 * \param [in]  size          Size of the buffer
 */
	void SX126xReadCommand(RadioCommands_t opcode, uint8_t *buffer, uint16_t size);

	/**@brief Write a single byte of data to the radio memory
 *
 * \param [in]  address       The address of the first byte to write in the radio
 * \param [in]  value         The data to be written in radio's memory
 */
	void SX126xWriteRegister(uint16_t address, uint8_t value);

	/**@brief Read a single byte of data from the radio memory
 *
 * \param [in]  address       The address of the first byte to write in the radio
 *
 * \retval      value         The value of the byte at the given address in radio's memory
 */
	uint8_t SX126xReadRegister(uint16_t address);

	/**@brief Sets the radio output power.
 *
 * \param [IN] power Sets the RF output power
 */
	void SX126xSetRfTxPower(int8_t power);

	/**@brief Gets the board PA selection configuration
 *
 * \param [IN] channel Channel frequency in Hz
 * \retval PaSelect RegPaConfig PaSelect value
 */
	uint8_t SX126xGetPaSelect(uint32_t channel);

	/**@brief Initializes the RF Switch I/Os pins interface
 */
	void SX126xAntSwOn(void);

	/**@brief De-initializes the RF Switch I/Os pins interface
 *
 * \remark Needed to decrease the power consumption in MCU low power modes
 */
	void SX126xAntSwOff(void);

	/**@brief Set the RF antenna switch to receiving mode
 *
 * \remark Used only on some modules e.g. eByte E22
 */
	void SX126xRXena(void);

	/**@brief Set the RF antenna switch to transmitting mode
 *
 * \remark Used only on some modules e.g. eByte E22
 */
	void SX126xTXena(void);

	/**@brief Checks if the given RF frequency is supported by the hardware
 *
 * \param [IN] frequency RF frequency to be checked
 * \retval isSupported [true: supported, false: unsupported]
 */
	bool SX126xCheckRfFrequency(uint32_t frequency);

	/**@brief Gets info on the number of packets received
 *
 * \param [OUT] nb_pkt_received     Number of received packets with CRC OK
 * \param [OUT] nb_pkt_crc_error    Number of received packets with CRC error
 * \param [OUT] nb_pkt_length_error Number of received packets with length error
 */
	void SX126xGetStats(uint16_t *nb_pkt_received, uint16_t *nb_pkt_crc_error, uint16_t *nb_pkt_length_error);

	/**@brief Resets values read by GetStats
 */
	void SX126xResetStats(void);

	/**@brief Radio hardware and global parameters
 */
	extern SX126x_t SX126x;
};
#endif // __SX126x_ARCH_H__
