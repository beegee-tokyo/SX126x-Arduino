/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Generic lora driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis, Gregory Cristian and Wael Guibene
*/

/******************************************************************************
 * @file    timer.c
 * @author  Insight SiP
 * @version V1.0.0
 * @date    02-mars-2018
 * @brief   timer implementation functions for LORA.
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
#include "boards/mcu/timer.h"
#include "boards/mcu/board.h"

extern "C"
{

	Ticker txTimeoutTick;
	Ticker rxTimeoutTick;
	uint32_t txTimeOutTime;
	uint32_t rxTimeOutTime;

	// External functions

	void TimerConfig(void)
	{
		/// \todo Nothing to do here for ESP32
	}

	void TimerInit(TimerEvent_t *obj, void (*callback)(void))
	{
		obj->Callback = callback;
		// Nothing to do here for the ESP32
	}

	void timerCallback(TimerEvent_t *obj)
	{
		// Nothing to do here for the ESP32
	}

	void TimerStart(TimerEvent_t *obj)
	{
		if (obj->timerNum == 1)
		{
			// TX timer
			txTimeoutTick.once_ms(txTimeOutTime, obj->Callback);
		}
		else if (obj->timerNum == 2)
		{
			// RX timer
			rxTimeoutTick.once_ms(rxTimeOutTime, obj->Callback);
		}
	}

	void TimerStop(TimerEvent_t *obj)
	{
		if (obj->timerNum == 1)
		{
			// TX timer
			txTimeoutTick.detach();
		}
		else if (obj->timerNum == 2)
		{
			// RX timer
			rxTimeoutTick.detach();
		}
	}

	void TimerReset(TimerEvent_t *obj)
	{
		if (obj->timerNum == 1)
		{
			// TX timer
			txTimeoutTick.detach();
			txTimeoutTick.once_ms(txTimeOutTime, obj->Callback);
		}
		else if (obj->timerNum == 2)
		{
			// RX timer
			rxTimeoutTick.detach();
			rxTimeoutTick.once_ms(rxTimeOutTime, obj->Callback);
		}
	}

	void TimerSetValue(TimerEvent_t *obj, uint32_t value)
	{
		if (obj->timerNum == 1)
		{
			// TX timer value
			txTimeOutTime = value;
		}
		else if (obj->timerNum == 2)
		{
			// RX timer value
			rxTimeOutTime = value;
		}
	}

	TimerTime_t TimerGetCurrentTime(void)
	{
		return millis();
	}

	TimerTime_t TimerGetElapsedTime(TimerTime_t past)
	{
		uint32_t nowInTicks = millis();
		uint32_t pastInTicks = past;
		TimerTime_t diff = nowInTicks - pastInTicks;

		return diff;
	}
};
#endif