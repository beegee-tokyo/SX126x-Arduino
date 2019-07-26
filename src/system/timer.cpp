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

#include "timer.h"
#include "boards/esp32/board.h"

extern "C"
{

	// External functions

	Ticker timeout1;
	Ticker timeout2;
	bool timeout1InUse = false;
	bool timeout2InUse = false;

	void TimerConfig(void)
	{
		/// \todo Nothing to do here for ESP32
	}

	void TimerInit(TimerEvent_t *obj, void (*callback)(void))
	{
		obj->Timestamp = millis();
		obj->ReloadValue = 0;
		obj->IsRunning = false;
		obj->Callback = callback;
		obj->Next = NULL;
		if (timeout1InUse == false)
		{
			obj->evtTicker = timeout1;
		}
		else if (timeout2InUse == false)
		{
			obj->evtTicker = timeout2;
		}
		else
		{
			/// \todo Check if how many timeouts we need. Only two TimerEvent_t objects are defined
		}
	}

	void timerCallback(TimerEvent_t *obj)
	{
		obj->IsRunning = false;
		obj->Callback();
	}

	void TimerStart(TimerEvent_t *obj)
	{
		// if (obj == NULL)
		// {
		//     return;
		// }
		// // if (obj->IsRunning)
		// // {
		// //     Serial.println("Stop Timer before Start Timer");
		// //     obj->evtTicker.detach();
		// // }
		// Serial.println("Start Timer with:");
		// Serial.printf("Timestamp %ld Reload %ld\nIsRunning %d\nCallback %ld, Ticker %ld", obj->Timestamp, obj->ReloadValue, obj->IsRunning, obj->Callback, obj->evtTicker);
		// obj->IsRunning = true;
		// obj->evtTicker.once_ms(obj->ReloadValue, obj->Callback);
	}

	void TimerStop(TimerEvent_t *obj)
	{
		// if (obj == NULL)
		// {
		//     return;
		// }
		// if (obj->IsRunning)
		// {
		//     obj->IsRunning = false;
		//     obj->evtTicker.detach();
		// }
	}

	void TimerReset(TimerEvent_t *obj)
	{
		obj->evtTicker.detach();
		obj->evtTicker.once_ms(obj->ReloadValue, obj->Callback);
	}

	void TimerSetValue(TimerEvent_t *obj, uint32_t value)
	{
		obj->ReloadValue = value;
		// obj->evtTicker.detach();
		// obj->evtTicker.attach(value, obj->Callback);
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
