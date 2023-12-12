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
// #define ARDUINO_ARCH_RP2040
#if defined ARDUINO_ARCH_RP2040 && not defined ARDUINO_RAKWIRELESS_RAK11300
#include "boards/mcu/board.h"
// #include "pico/unique_id.h"

uint32_t BoardGetRandomSeed(void)
{
	/// \todo find matching function for RP2040
	// return random(255);
	return 128;
}

void BoardGetUniqueId(uint8_t *id)
{
	uint8_t name[32];
	getUniqueSerialNumber(name);

	// 	pico_unique_board_id_t brd_id;
	// pico_get_unique_board_id(&brd_id);

	id[7] = name[7] + name[15] + name[23] + name[31];
	id[6] = name[6] + name[14] + name[22] + name[30];
	id[5] = name[5] + name[13] + name[21] + name[29];
	id[4] = name[4] + name[12] + name[20] + name[28];
	id[3] = name[3] + name[11] + name[19] + name[27];
	id[2] = name[2] + name[10] + name[18] + name[26];
	id[1] = name[1] + name[9] + name[17] + name[25];
	id[0] = name[0] + name[8] + name[16] + name[24];
}

uint8_t BoardGetBatteryLevel(void)
{
	uint8_t batteryLevel = 0;

	//TO BE IMPLEMENTED

	return batteryLevel;
}

void BoardDisableIrq(void)
{
}

void BoardEnableIrq(void)
{
}

#endif
