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
// #define ARDUINO_RAKWIRELESS_RAK11300
#if defined ARDUINO_RAKWIRELESS_RAK11300

#include "boards/mcu/timer.h"
#include "boards/mcu/board.h"

#include <RPi_Pico_TimerInterrupt.h>
#include "SimpleTimer.h"
#include <pico/time.h>

/** Timer task handle */
TaskHandle_t _timerTaskHandle;

/** Wake signal for timer task */
QueueHandle_t _timer_queue = NULL;

// Init SimpleTimer
SimpleTimer _simpleTimer;

BaseType_t _xHigherPriorityTaskWoken = pdFALSE;

/** RPi ISR timer 0 */
RPI_PICO_Timer _ITimer0(0);
/** RPi ISR timer 1 */
RPI_PICO_Timer _ITimer1(1);
/** RPi ISR timer 2 */
RPI_PICO_Timer _ITimer2(2);

/** Timer structure */
struct s_timer
{
	time_t duration = 0;
	void (*callback)();
};

/** Skip ISR timer check call */
volatile bool skip_timer = false;

/**
 * @brief ISR function to check if any SimpleTimer needs attention
 *
 * @param t unused
 * @return true keep timer running
 * @return false stop timer (maybe, not tried)
 */
bool TimerHandler(struct repeating_timer *t)
{
	(void)t;

	/** Dummy value, needed for FreeRTOS queue */
	int value = 0;
	if (!skip_timer)
	{
		xQueueSendFromISR(_timer_queue, &value, &_xHigherPriorityTaskWoken);
		value++;
	}
	return true;
}

/**
 * @brief Task to handle the SimpleTimer tickers. Called by ISR handler through FreeRTOS queue
 *
 * @param pvParameters unused
 */
void _timer_task(void *pvParameters)
{
	/** Dummy value, needed for FreeRTOS queue */
	int val;
	LOG_LIB("TIM_T", "Timer task started");
	while (1)
	{
		// Read from queue (wait max time if queue is empty)
		if (xQueueReceive(_timer_queue, &val, portMAX_DELAY) == pdTRUE)
		{
			_simpleTimer.run();
		}
	}
}

/**
 * @brief Configure the RP2040 timers
 * Starts the background task to handle timer events
 * Starts the Hardware timer to wakeup the handler thread
 *
 */
void TimerConfig(void)
{
	// Create the timer event queue
	_timer_queue = xQueueCreate(1000, sizeof(int));
	if (_timer_queue == NULL)
	{
		LOG_LIB("TIM", "Unable to create queue");
	}

	// Initialize the timer task
	if (xTaskCreate(_timer_task, "TIMER", 4096, NULL, 8, &_timerTaskHandle))
	{
		LOG_LIB("TIM", "Timer task start success");
	}

	// Initialize one of the three ISR timers to 1 ms interval
	bool timer_ok = false;
	int8_t try_timer = 2;
	do
	{
		if (try_timer == 0)
		{
			if (_ITimer0.attachInterruptInterval(500.0, TimerHandler))
			{
				LOG_LIB("TIM", "Starting ITimer0 OK");
				timer_ok = true;
			}
			else
			{
				try_timer--;
				if (try_timer == -1)
				{
					LOG_LIB("TIM", "FATAL ERROR, NO HARDWARE TIMER AVAILABLE");
					return;
				}
			}
		}
		if (try_timer == 1)
		{
			if (_ITimer1.attachInterruptInterval(500.0, TimerHandler))
			{
				LOG_LIB("TIM", "Starting ITimer1 OK");
				timer_ok = true;
			}
			else
			{
				try_timer--;
				if (try_timer == -1)
				{
					LOG_LIB("TIM", "FATAL ERROR, NO HARDWARE TIMER AVAILABLE");
					return;
				}
			}
		}
		if (try_timer == 2)
		{
			if (_ITimer2.attachInterruptInterval(500.0, TimerHandler))
			{
				LOG_LIB("TIM", "Starting ITimer2 OK");
				timer_ok = true;
			}
			else
			{
				try_timer--;
				if (try_timer == -1)
				{
					LOG_LIB("TIM", "FATAL ERROR, NO HARDWARE TIMER AVAILABLE");
					return;
				}
			}
		}
	} while (!timer_ok);

	return;
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
	// Save the callback pointer
	obj->Callback = callback;
	// Timers are assigned during start
	// Timers are deleted once expired
}

/**
 * @brief Activate a timer
 * CAUTION requires the timer was initialized before
 *
 * @param obj structure with timer settings
 */
void TimerStart(TimerEvent_t *obj)
{
	skip_timer = true;
	int idx;
	if (obj->oneShot)
	{
		idx = _simpleTimer.setTimer(obj->ReloadValue, obj->Callback, 1);
	}
	else
	{
		idx = _simpleTimer.setTimer(obj->ReloadValue, obj->Callback, 0);
	}
	obj->timerNum = idx;
	LOG_LIB("TIM", "Timer %d started as %s with %d ms", idx, obj->oneShot ? "OneShot" : "Recurring", obj->ReloadValue);
	skip_timer = false;
}

/**
 * @brief Deactivate a timer
 * CAUTION requires the timer was initialized before
 *
 * @param obj structure with timer settings
 */
void TimerStop(TimerEvent_t *obj)
{
	skip_timer = true;
	int idx = obj->timerNum;
	_simpleTimer.deleteTimer(idx);
	LOG_LIB("TIM", "Timer %d stopped manually", idx);
	skip_timer = false;
}

/**
 * @brief Restart a timer
 * CAUTION requires the timer was initialized before
 *
 * @param obj structure with timer settings
 */
void TimerReset(TimerEvent_t *obj)
{
	skip_timer = true;
	int idx = obj->timerNum;
	_simpleTimer.deleteTimer(idx);
	if (obj->oneShot)
	{
		idx = _simpleTimer.setTimer(obj->ReloadValue, obj->Callback, 1);
	}
	else
	{
		idx = _simpleTimer.setTimer(obj->ReloadValue, obj->Callback, 0);
	}
	obj->timerNum = idx;
	// LOG_LIB("TIM", "Timer %d reset with %d ms as %s", idx, obj->ReloadValue, obj->oneShot ? "Oneshot" : "Interval");
	skip_timer = false;
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
	skip_timer = true;
	int idx = obj->timerNum;
	obj->ReloadValue = value;

	_simpleTimer.changeTime(idx, value);
	// LOG_LIB("TIM", "Timer %d set to %d ms", idx, obj->ReloadValue);
	skip_timer = false;
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

#endif // ARDUINO_RAKWIRELESS_RAK11300
