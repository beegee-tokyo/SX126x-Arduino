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
#if defined ARDUINO_RAKWIRELESS_RAK11300
#include "boards/mcu/board.h"

uint32_t BoardGetRandomSeed(void)
{
	return random(255);
}

void BoardGetUniqueId(uint8_t *id)
{
	pico_unique_board_id_t brd_id; // uint8_t[8]
	pico_get_unique_board_id(&brd_id);

	id[7] = brd_id.id[7];
	id[6] = brd_id.id[6];
	id[5] = brd_id.id[5];
	id[4] = brd_id.id[4];
	id[3] = brd_id.id[3];
	id[2] = brd_id.id[2];
	id[1] = brd_id.id[1];
	id[0] = brd_id.id[0];
}

uint8_t BoardGetBatteryLevel(void)
{
	uint8_t batteryLevel = 0;

	// TO BE IMPLEMENTED

	return batteryLevel;
}

void BoardDisableIrq(void)
{
}

void BoardEnableIrq(void)
{
}

#endif
