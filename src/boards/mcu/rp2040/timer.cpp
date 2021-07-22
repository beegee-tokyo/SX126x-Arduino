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
#ifdef ARDUINO_ARCH_RP2040
#include "boards/mcu/timer.h"
#include "boards/mcu/board.h"

#include <mbed.h>
#include <rtos.h>
#include <cmsis_os.h>
#include <pico/time.h>

using namespace rtos;
using namespace mbed;

#define USE_HW_TIMER

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

/** The hardware timer used to trigger the timer handle thread */
repeating_timer_t timer_500us;

/** Array to hold the timers */
s_timer timer[10];

// Timer callback
bool cb_timer(repeating_timer_t *rt);

/** Thread id for timer thread */
osThreadId timer_event_thread = NULL;

/** Alarm pool for low level timer function */
alarm_pool_t *my_alarm_pool;

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
#ifdef USE_HW_TIMER
		// Wait for event
		osSignalWait(0, osWaitForever);
#else
		delay(1);
#endif
		for (int idx = 0; idx < 10; idx++)
		{
			if (timer[idx].active)
			{
				if ((unsigned long)(micros() - (unsigned long)timer[idx].start_time) >= (unsigned long)timer[idx].duration)
				{
					if (idx == 0)
					{
						// digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
						// LOG_LIB("TIM", "Handling timer %d", idx);
					}
					timer[idx].callback();
					timer[idx].start_time = micros();
				}
			}
		}
		yield();
	}
}

/**
 * @brief Hardware timer callback
 * Triggers the thread to check all active timers
 * Called every 500 us
 * 
 * @param rt unused
 * @return true retrigger the timer
 * @return false cancel the timer
 */
bool cb_timer(repeating_timer_t *rt)
{
	if (timer_event_thread != NULL)
	{
		osSignalSet(timer_event_thread, 0x01);
	}
	// keep it running
	return true;
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

	// Create the default alarm pool (if not already created or disabled)
	my_alarm_pool = alarm_pool_create(1, 12);

	if (my_alarm_pool == NULL)
	{
		LOG_LIB("TIM", "Failed to create alarm pool");
	}

	// Start the 500us timer to handle the different alarm events
	if (!alarm_pool_add_repeating_timer_us(my_alarm_pool, 500, cb_timer, NULL, &timer_500us))
	{
		LOG_LIB("TIM", "Failed to add timer");
	}
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

	timer[idx].start_time = micros();
	timer[idx].active = true;
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
	timer[idx].active = false;
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
	timer[idx].active = false;
	timer[idx].start_time = micros();
	timer[idx].active = true;
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
