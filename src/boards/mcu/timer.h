/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Timer objects and scheduling management

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/

/******************************************************************************
 * @file    timer.h
 * @author  Insight SiP
 * @version V1.0.0
 * @date    02-mars-2018
 * @brief   timer header functions for LORA.
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

#ifndef __TIMER_H__
#define __TIMER_H__

#include "stdint.h"
#include "stdbool.h"
#if defined(ESP32) || defined(ESP8266)
#include <Ticker.h>
#endif
#include "sx126x-debug.h"

#define ROUNDED_DIV(A, B) (((A) + ((B) / 2)) / (B))

typedef void (*callbackType)(void);

/**@brief Timer object description
 */
typedef struct TimerEvent_s
{
	uint8_t timerNum;			  /**< Used with ESP32 MCU 1 for TX, 2 for RX*/
	bool oneShot = true;		  /**< True if it is a one shot timer */
	uint32_t Timestamp;			  /**< Current timer value */
	uint32_t ReloadValue = 10000; /**< Timer delay value	*/
	bool IsRunning;				  /**< Is the timer currently running	*/
	void (*Callback)(void);		  /**< Timer IRQ callback function	*/
	struct TimerEvent_s *Next;	  /**< Pointer to the next Timer object.	*/
} TimerEvent_t;

/**@brief Timer time variable definition
 */
#ifndef TimerTime_t
typedef uint32_t TimerTime_t;
#endif

/**@brief Initializes the RTC2 timer
 *
 * @details Set prescaler to 31 in order to have Fs=1kHz
 *			Enable CC interrupt
 *			Start RTC2
 */
void TimerConfig(void);

/**@brief Initializes the timer object
 *
 * @remark TimerSetValue function must be called before starting the timer.
 *         this function initializes timestamp and reload value at 0.
 *
 * @param  obj          Structure containing the timer object parameters
 * @param  callback     Function callback called at the end of the timeout
 */
void TimerInit(TimerEvent_t *obj, void (*callback)(void));

/**@brief Starts and adds the timer object to the list of timer events
 *
 * @param  obj Structure containing the timer object parameters
 */
void TimerStart(TimerEvent_t *obj);

/**@brief Stops and removes the timer object from the list of timer events
 *
 * @param  obj Structure containing the timer object parameters
 */
void TimerStop(TimerEvent_t *obj);

/**@brief Resets the timer object
 *
 * @param  obj Structure containing the timer object parameters
 */
void TimerReset(TimerEvent_t *obj);

/**@brief Set timer new timeout value
 *
 * @param  obj   Structure containing the timer object parameters
 *
 * @param  value New timer timeout value in ms
 */
void TimerSetValue(TimerEvent_t *obj, uint32_t value);

/**@brief Return the Time elapsed since a fix moment in Time
 *
 * @param  savedTime    fix moment in Time
 *
 * @retval time             returns elapsed time in ms
 */
TimerTime_t TimerGetElapsedTime(TimerTime_t savedTime);

/**@brief Read the current time ellapsed since the start (or restart) of RTC2
 *
 * @retval current time in ms
 */
TimerTime_t TimerGetCurrentTime(void);

void TimerHandleEvents(void);

#endif // __TIMER_H__
