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
#ifdef ESP32
#include "boards/mcu/board.h"
extern "C"
{
	uint32_t BoardGetRandomSeed(void)
	{
		return random(255);
	}

	void BoardGetUniqueId(uint8_t *id)
	{
		//TO BE IMPLEMENTED
		id[7] = 8;
		id[6] = 7;
		id[5] = 6;
		id[4] = 5;
		id[3] = 4;
		id[2] = 3;
		id[1] = 2;
		id[0] = 1;
	}

	uint8_t BoardGetBatteryLevel(void)
	{
		uint8_t batteryLevel = 0;

		//TO BE IMPLEMENTED

		return batteryLevel;
	}

	portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

	void BoardDisableIrq(void)
	{
		portENTER_CRITICAL(&mux);
	}

	void BoardEnableIrq(void)
	{
		portEXIT_CRITICAL(&mux);
	}
};
#endif