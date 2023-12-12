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
// #define ARDUINO_ARCH_RP2040
#if defined ARDUINO_ARCH_RP2040 && not defined ARDUINO_RAKWIRELESS_RAK11300

#include "boards/mcu/timer.h"
#include "boards/mcu/board.h"

#include <mbed.h>
#include <rtos.h>
#include <cmsis_os.h>
#include <pico/time.h>

using namespace std::chrono_literals;
using namespace std::chrono;
using namespace rtos;
using namespace mbed;

/** The event handler thread */
Thread thread_handle_timers(osPriorityAboveNormal, 4096);

/** Timer structure */
struct s_timer
{
	int32_t signal = 0;
	bool in_use = false;
	bool active = false;
	time_t duration;
	time_t start_time;
	void (*callback)();
};

/** Array to hold the timers */
s_timer timer[10];
mbed::Ticker timerTickers[10];

// Timer callbacks
void cb_timer_1(void);
void cb_timer_2(void);
void cb_timer_3(void);
void cb_timer_4(void);
void cb_timer_5(void);
void cb_timer_6(void);
void cb_timer_7(void);
void cb_timer_8(void);
void cb_timer_9(void);
void cb_timer_10(void);

void (*cb_callback[])() = {cb_timer_1, cb_timer_2, cb_timer_3, cb_timer_4, cb_timer_5, cb_timer_6, cb_timer_7, cb_timer_8, cb_timer_9, cb_timer_10};

/** Thread id for timer thread */
osThreadId timer_event_thread = NULL;

/**
 * @brief Thread to handle timer events
 * Called every 500 us to check if any timer
 * has to be handled
 *
 */
void handle_timer_events(void)
{
	timer_event_thread = osThreadGetId();
	while (true)
	{
		// Wait for event
		osEvent event = osSignalWait(0, osWaitForever);

		if (event.status == osEventSignal)
		{
			// Serial.printf("Signal %02X\n", event.value.signals);
			uint16_t mask = 0x01;
			for (int idx = 0; idx < 10; idx++)
			{
				if ((event.value.signals & (mask << idx)) == (mask << idx))
				{
					// Serial.printf("Callback %d", idx);
					timer[idx].callback();
				}
			}
		}
		yield();
	}
}

/**
 * @brief Hardware timer callbacks
 */
void cb_timer_1(void)
{
	if (timer_event_thread != NULL)
	{
		osSignalSet(timer_event_thread, 0x01);
	}
}
void cb_timer_2(void)
{
	if (timer_event_thread != NULL)
	{
		osSignalSet(timer_event_thread, 0x02);
	}
}
void cb_timer_3(void)
{
	if (timer_event_thread != NULL)
	{
		osSignalSet(timer_event_thread, 0x04);
	}
}
void cb_timer_4(void)
{
	if (timer_event_thread != NULL)
	{
		osSignalSet(timer_event_thread, 0x08);
	}
}
void cb_timer_5(void)
{
	if (timer_event_thread != NULL)
	{
		osSignalSet(timer_event_thread, 0x10);
	}
}
void cb_timer_6(void)
{
	if (timer_event_thread != NULL)
	{
		osSignalSet(timer_event_thread, 0x20);
	}
}
void cb_timer_7(void)
{
	if (timer_event_thread != NULL)
	{
		osSignalSet(timer_event_thread, 0x40);
	}
}
void cb_timer_8(void)
{
	if (timer_event_thread != NULL)
	{
		osSignalSet(timer_event_thread, 0x80);
	}
}
void cb_timer_9(void)
{
	if (timer_event_thread != NULL)
	{
		osSignalSet(timer_event_thread, 0x100);
	}
}
void cb_timer_10(void)
{
	if (timer_event_thread != NULL)
	{
		osSignalSet(timer_event_thread, 0x200);
	}
}

/**
 * @brief Configure the RP2040 timers
 * Starts the background thread to handle timer events
 * Starts the Hardware timer to wakeup the handler thread
 *
 */
void TimerConfig(void)
{
	for (int idx = 0; idx < 10; idx++)
	{
		timer[idx].in_use = false;
	}

	// LOG_LIB("TIM", "threads start");
	thread_handle_timers.start(handle_timer_events);
	thread_handle_timers.set_priority(osPriorityAboveNormal);
}

/**
 * @brief Initialize a new timer
 * Checks for available timer slot (limited to 10)
 *
 * @param obj structure with timer settings
 * @param callback callback that the timer should call
 */
void TimerInit(TimerEvent_t *obj, void (*callback)(void))
{
	// Look for an available Ticker
	for (int idx = 0; idx < 10; idx++)
	{
		if (timer[idx].in_use == false)
		{
			timer[idx].signal = 1 << idx; // + 1;
			timer[idx].in_use = true;
			timer[idx].active = false;
			timer[idx].callback = callback;
			timer[idx].duration = obj->ReloadValue * 1000;
			obj->timerNum = idx;
			obj->Callback = callback;
			LOG_LIB("TIM", "Timer %d assigned", idx);
			return;
		}
	}
	LOG_LIB("TIM", "No more timers available!");
}

/**
 * @brief Activate a timer
 * CAUTION requires the timer was initialized before
 *
 * @param obj structure with timer settings
 */
void TimerStart(TimerEvent_t *obj)
{
	int idx = obj->timerNum;

	// t_attach(idx);
	timerTickers[idx].attach(cb_callback[idx], (microseconds)timer[idx].duration);

	// LOG_LIB("TIM", "Timer %d started with %d ms", idx, timer[idx].duration);
}

/**
 * @brief Deactivate a timer
 * CAUTION requires the timer was initialized before
 *
 * @param obj structure with timer settings
 */
void TimerStop(TimerEvent_t *obj)
{
	int idx = obj->timerNum;

	timerTickers[idx].detach();

	// LOG_LIB("TIM", "Timer %d stopped", idx);
}

/**
 * @brief Restart a timer
 * CAUTION requires the timer was initialized before
 *
 * @param obj structure with timer settings
 */
void TimerReset(TimerEvent_t *obj)
{
	int idx = obj->timerNum;

	timerTickers[idx].detach();
	timerTickers[idx].attach(cb_callback[idx], (microseconds)timer[idx].duration);

	// LOG_LIB("TIM", "Timer %d reset with %d ms", idx, timerTimes[idx]);
}

/**
 * @brief Set the duration time of a timer
 * CAUTION requires the timer was initialized before
 *
 * @param obj structure with timer settings
 * @param value duration time in milliseconds
 */
void TimerSetValue(TimerEvent_t *obj, uint32_t value)
{
	int idx = obj->timerNum;
	timer[idx].duration = value * 1000;

	// LOG_LIB("TIM", "Timer %d setup to %d ms", idx, value);
}

/**
 * @brief Get current time in milliseconds
 *
 * @return TimerTime_t time in milliseconds
 */
TimerTime_t TimerGetCurrentTime(void)
{
	return millis();
}

/**
 * @brief Get timers elapsed time
 *
 * @param past last time the check was done
 * @return TimerTime_t difference between now and last check
 */
TimerTime_t TimerGetElapsedTime(TimerTime_t past)
{
	uint32_t nowInTicks = millis();
	uint32_t pastInTicks = past;
	TimerTime_t diff = nowInTicks - pastInTicks;

	return diff;
}
#endif // ARDUINO_ARCH_RP2040
