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
#ifdef NRF52
#include "boards/mcu/timer.h"
#include "boards/mcu/board.h"
#include "app_util.h"

extern "C"
{
#define TIMER_RTC2_PRESCALER 31     /**< Prescaler for the RTC Timer */
#define TIMER_RTC2_CLOCK_FREQ 32768 /**< Clock frequency of the RTC timer. */

/**@brief Convert ticks to timer milliseconds.  (tick * 32000 / 32768)
 *
 * @param[in]  ticks
 *
 * @return     milliseconds.
 */
#define TICKS_TO_MS(TICKS)                                         \
    ((uint32_t)ROUNDED_DIV(                                        \
        (TICKS) * ((uint64_t)(1000 * (TIMER_RTC2_PRESCALER + 1))), \
        (uint64_t)TIMER_RTC2_CLOCK_FREQ))

/**@brief Convert timer milliseconds to ticks.  (tick * 32768 / 32000)
 *
 * @param[in]  milliseconds
 *
 * @return     ticks.
 */
#define MS_TO_TICKS(MS)                           \
    ((uint32_t)ROUNDED_DIV(                       \
        (MS) * ((uint64_t)TIMER_RTC2_CLOCK_FREQ), \
        1000 * (TIMER_RTC2_PRESCALER + 1)))

    TimerTime_t m_rtc_reference_time; /**< Reference time */

    /** Timers list head pointer
 */
    static TimerEvent_t *TimerListHead = NULL;

    /**@brief returns the wake up time in ticks
 * @retval wake up time in ticks
 */
    static uint32_t RTC2_GetMinimumTimeout(void)
    {
        return 3;
    }

    /**@brief Get the RTC timer elapsed time since the reference time
 *
 * @retval RTC Elapsed time in ticks
 */
    static uint32_t RTC2_GetTimerElapsedTime(void)
    {
        TimerTime_t now_in_ticks = NRF_RTC2->COUNTER;
        return (now_in_ticks - m_rtc_reference_time);
    }

    /**@brief Get the RTC2 Counter value
 *
 * @retval RTC2 COUNTER
 */
    static uint32_t RTC2_GetCounterReg(void)
    {
        return NRF_RTC2->COUNTER;
    }

    /**@brief Set value in the CC[0] register
 *
 * @details The CC[0] is set at now (read in this funtion) + timeout, once the COUNTER
 *			reaches this value, an interrupt is triggered
 *
 * @param timeout Duration of the Timer ticks
 */
    static void RTC2_SetCompareReg(uint32_t timeout)
    {
        TimerTime_t now = NRF_RTC2->COUNTER;
        NRF_RTC2->CC[0] = now + timeout;
    }

    /**@brief Check if the input timer object exists
 *
 * @param [IN] obj Structure containing the timer object parameters
 *
 * @retval true is it exists, false otherwise
 */
    static bool TimerExists(TimerEvent_t *obj)
    {
        TimerEvent_t *cur = TimerListHead;

        while (cur != NULL)
        {
            if (cur == obj)
            {
                return true;
            }
            cur = cur->Next;
        }
        return false;
    }

    /**@brief Sets a timeout of an timer obj
 * 
 * @param [IN] obj Structure containing the timer object parameters
 */
    static void TimerSetTimeout(TimerEvent_t *obj)
    {
        int32_t minTicks = RTC2_GetMinimumTimeout();
        obj->IsRunning = true;

        //in case deadline too soon
        if (obj->Timestamp < (RTC2_GetTimerElapsedTime() + minTicks))
        {
            obj->Timestamp = RTC2_GetTimerElapsedTime() + minTicks;
        }
        RTC2_SetCompareReg(obj->Timestamp);
    }

    /**@brief Adds or replace the head timer of the list.
 *
 * @remark The list is automatically sorted. The list head always contains the next timer to expire.
 *
 * @param [IN]  obj Timer object to be become the new head
 */
    static void TimerInsertNewHeadTimer(TimerEvent_t *obj)
    {
        TimerEvent_t *cur = TimerListHead;

        if (cur != NULL)
        {
            cur->IsRunning = false;
        }

        obj->Next = cur;
        TimerListHead = obj;
        TimerSetTimeout(TimerListHead);
    }

    /**@brief Adds a timer to the list.
 *
 * @remark The list is automatically sorted. The list head always contains the next timer to expire.
 *
 * @param [IN]  obj Timer object to be added to the list
 */
    static void TimerInsertTimer(TimerEvent_t *obj)
    {
        TimerEvent_t *cur = TimerListHead;
        TimerEvent_t *next = TimerListHead->Next;

        while (cur->Next != NULL)
        {
            if (obj->Timestamp > next->Timestamp)
            {
                cur = next;
                next = next->Next;
            }
            else
            {
                cur->Next = obj;
                obj->Next = next;
                return;
            }
        }
        cur->Next = obj;
        obj->Next = NULL;
    }

    /**@brief RTC2 IRQ
 *
 */
    void RTC2_IRQHandler(void)
    {
        // Clear all events (also unexpected ones)
        NRF_RTC2->EVENTS_COMPARE[0] = 0;
        NRF_RTC2->EVENTS_COMPARE[1] = 0;
        NRF_RTC2->EVENTS_COMPARE[2] = 0;
        NRF_RTC2->EVENTS_COMPARE[3] = 0;
        NRF_RTC2->EVENTS_TICK = 0;
        NRF_RTC2->EVENTS_OVRFLW = 0;

        TimerEvent_t *cur;
        TimerEvent_t *next;

        uint32_t old = m_rtc_reference_time;
        uint32_t now = RTC2_GetCounterReg();
        m_rtc_reference_time = now;
        uint32_t delta = now - old;

        /* update timeStamp based upon new Time Reference*/
        if (TimerListHead != NULL)
        {
            for (cur = TimerListHead; cur->Next != NULL; cur = cur->Next)
            {
                next = cur->Next;
                if (next->Timestamp > delta)
                {
                    next->Timestamp -= delta;
                }
                else
                {
                    next->Timestamp = 0;
                }
            }
        }

        /* execute imediately the callback */
        if (TimerListHead != NULL)
        {
            cur = TimerListHead;
            TimerListHead = TimerListHead->Next;
            if (cur->Callback != NULL)
            {
                cur->Callback();
            }
        }

        // remove all the expired object from the list
        while ((TimerListHead != NULL) && (TimerListHead->Timestamp < RTC2_GetTimerElapsedTime()))
        {
            cur = TimerListHead;
            TimerListHead = TimerListHead->Next;
            if (cur->Callback != NULL)
            {
                cur->Callback();
            }
        }

        /* start the next TimerListHead if it exists AND NOT running */
        if ((TimerListHead != NULL) && (TimerListHead->IsRunning == false))
        {
            TimerSetTimeout(TimerListHead);
        }
    }

    // External functions

    void TimerConfig(void)
    {
        NRF_RTC2->PRESCALER = TIMER_RTC2_PRESCALER;
        NVIC_SetPriority(RTC2_IRQn, 5);

        NRF_RTC2->EVTENSET = RTC_EVTEN_COMPARE0_Disabled;
        NRF_RTC2->INTENSET = RTC_INTENSET_COMPARE0_Disabled;

        NVIC_ClearPendingIRQ(RTC2_IRQn);
        NVIC_EnableIRQ(RTC2_IRQn);

        NRF_RTC2->TASKS_START = 1;
    }

    void TimerInit(TimerEvent_t *obj, void (*callback)(void))
    {
        obj->Timestamp = 0;
        obj->ReloadValue = 0;
        obj->IsRunning = false;
        obj->Callback = callback;
        obj->Next = NULL;
    }

    void TimerStart(TimerEvent_t *obj)
    {
        uint32_t elapsedTime = 0;

        if ((obj == NULL) || (TimerExists(obj) == true))
        {
            return;
        }

        // CRITICAL_REGION_ENTER();

        obj->Timestamp = obj->ReloadValue;
        obj->IsRunning = false;

        // First obj in the list
        if (TimerListHead == NULL)
        {
            // enable RTC2 CC[0] interrupts
            NRF_RTC2->EVTENSET = RTC_EVTEN_COMPARE0_Msk;
            NRF_RTC2->INTENSET = RTC_INTENSET_COMPARE0_Msk;
            // Get reference time
            m_rtc_reference_time = RTC2_GetCounterReg();

            // insert a timeout at reference time + obj->Timestamp
            TimerInsertNewHeadTimer(obj);
        }
        // Add obj to the list
        else
        {
            elapsedTime = RTC2_GetTimerElapsedTime();
            obj->Timestamp += elapsedTime;

            if (obj->Timestamp < TimerListHead->Timestamp)
            {
                TimerInsertNewHeadTimer(obj);
            }
            else
            {
                TimerInsertTimer(obj);
            }
        }

        // CRITICAL_REGION_EXIT();
    }

    void TimerStop(TimerEvent_t *obj)
    {
        TimerEvent_t *prev = TimerListHead;
        TimerEvent_t *cur = TimerListHead;

        // List is empty or the Obj to stop does not exist
        if ((TimerListHead == NULL) || (obj == NULL))
        {
            return;
        }

        // CRITICAL_REGION_ENTER();

        // Stop the Head
        if (TimerListHead == obj)
        {
            // The head is already running
            if (TimerListHead->IsRunning == true)
            {
                // If another obj is registered we switch to it
                if (TimerListHead->Next != NULL)
                {
                    TimerListHead->IsRunning = false;
                    TimerListHead = TimerListHead->Next;
                    TimerSetTimeout(TimerListHead);
                }
                // No other obj registered: we can disable interrupts
                else
                {
                    // Disable RTC2 CC[0] interrupt
                    NRF_RTC2->EVTENSET = RTC_EVTEN_COMPARE0_Disabled;
                    NRF_RTC2->INTENSET = RTC_INTENSET_COMPARE0_Disabled;
                    TimerListHead = NULL;
                }
            }
            // Stop the head before it is started
            else
            {
                if (TimerListHead->Next != NULL)
                {
                    TimerListHead = TimerListHead->Next;
                }
                else
                {
                    TimerListHead = NULL;
                }
            }
        }
        // Stop an object within the list
        else
        {
            while (cur != NULL)
            {
                if (cur == obj)
                {
                    if (cur->Next != NULL)
                    {
                        cur = cur->Next;
                        prev->Next = cur;
                    }
                    else
                    {
                        cur = NULL;
                        prev->Next = cur;
                    }
                    break;
                }
                else
                {
                    prev = cur;
                    cur = cur->Next;
                }
            }
        }

        // CRITICAL_REGION_EXIT();
    }

    void TimerReset(TimerEvent_t *obj)
    {
        TimerStop(obj);
        TimerStart(obj);
    }

    void TimerSetValue(TimerEvent_t *obj, uint32_t value)
    {
        uint32_t minValue = 0;
        uint32_t ticks = MS_TO_TICKS(value);

        // APP_ERROR_CHECK_BOOL(obj!=NULL);

        TimerStop(obj);

        minValue = RTC2_GetMinimumTimeout();
        if (ticks < minValue)
        {
            ticks = minValue;
        }

        obj->Timestamp = ticks;
        obj->ReloadValue = ticks;
    }

    TimerTime_t TimerGetCurrentTime(void)
    {
        uint32_t now = RTC2_GetCounterReg();
        return TICKS_TO_MS(now);
    }

    TimerTime_t TimerGetElapsedTime(TimerTime_t past)
    {
        uint32_t nowInTicks = RTC2_GetCounterReg();
        uint32_t pastInTicks = MS_TO_TICKS(past);
        TimerTime_t diff = TICKS_TO_MS((nowInTicks - pastInTicks) & 0x00FFFFFF);

        return diff;
    }
};
#endif