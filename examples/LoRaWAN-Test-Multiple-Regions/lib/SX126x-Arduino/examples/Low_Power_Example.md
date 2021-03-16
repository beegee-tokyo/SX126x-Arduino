# SX1262 - nRF52 Low Power Example

## Overview
In battery LPWAN applications reducing the power consumption has a high priority. Reduction of power consumption is done by keeping the MCU and where possible, as well the peripherals in sleep mode. They should ony wakeup on an external event or after a specified time to transmit data. 

The Arduino sketch is in the folder [DeepSleep-nRF52](DeepSleep-nRF52).
The PlatformIO project is in the folder [DeepSleep-nRF52Pio](DeepSleep-nRF52Pio).

When using the Arduino framework for the nRF52 modules it is not obvious how to send the MCU into deep-sleep. But there are methods made available from the underlaying FreeRTOS OS that can be used to send a task into sleep mode. And when all tasks are in sleep mode, the MCU is going into low power sleep mode. 
It is not as perfect implemented as for example on the ESP32 MCU's where specific sleep functions can be called.
But there are several methods, and here we will have a look into two of them. One is using the `delay` function, the other one is to use `semaphores` in your application.

### `delay(milliseconds)`
This command will send the task into sleep for x milliseconds. This sounds easy to use, however, is not very practical. Because while in the `delay()` function, the task cannot receive any information about external events, like an interrupt from a sensor or from a 9DOF sensor. So for most scenarios the `delay` is not a good solution.

### `xSemaphoreTakeBinary(semphoreHandle, portMAX_DELAY)`
FreeRTOS provides `semaphores` to control task switches and let tasks "sleep" while waiting for an event. Looking into the [FreeRTOS documentation](https://www.freertos.org/a00113.html), you can see there are several types of semaphores, named `binary`, `counting`, `mutex` and `recursive mutex`. To keep things simple here, I will use the `binary` semaphore in this example.

### How do semaphores work? 
I am here citing the [FreeRTOS documentation](https://www.freertos.org/Embedded-RTOS-Binary-Semaphores.html):
>  Think of a binary semaphore as a queue that can only hold one item. The queue can therefore only be empty or full (hence binary). Tasks and interrupts using the queue don’t care what the queue holds – they only want to know if the queue is empty or full. This mechanism can be exploited to synchronise (for example) a task with an interrupt.
> 
> Consider the case where a task is used to service a peripheral. Polling the peripheral would be wasteful of CPU resources, and prevent other tasks from executing. It is therefore preferable that the task spends most of its time in the Blocked state (allowing other tasks to execute) and only execute itself when there is actually something for it to do. This is achieved using a binary semaphore by having the task Block while attempting to ‘take’ the semaphore. An interrupt routine is then written for the peripheral that just ‘gives’ the semaphore when the peripheral requires servicing. The task always ‘takes’ the semaphore (reads from the queue to make the queue empty), but never ‘gives’ it. The interrupt always ‘gives’ the semaphore (writes to the queue to make it full) but never takes it.

### How do we use semaphores in this low power example?
This example will use separate tasks. One is the Arduino `loop()` function (referred to in the following parts as `loopTask`), which is actually a task on FreeRTOS. A second task is created to handle all LoRa events. We call this second task `loraTask`.

For the `loopTask`, we create a semaphore called `taskEvent`, that is given by two different events:
- a timer event, which wakes up the `loopTask` every 2 minutes to send a status message to the LoRaWan server
- a LoRaWAN downlink event, that is triggered if a downlink package from the LoRaWAN server has arrived

And for the `loraTask`, we create a second semaphore, called `loraEvent`, that is given by only one event. This event is the interrupt signal of the SX1262 transceiver. Only if the SX1262 transceiver is setting the interrupt signal the `loraTask` has to wakeup and handle the event.

In the example code both `taskEvent` and the `loraEvent` semaphores are taken by the setup functions before the `loopTask` and the `loraTask` are started. Once the two tasks are started, they will call `xSemaphoreTake(semaphore, portMAX_DELAY)`. The first parameter is the semaphore the task wants to take, the second parameter is the time the task will wait for the semaphore to be available. `portMAX_DELAY` means that the function will not return until the semaphore is given ==> the task goes to sleep!

### Code explanation
#### Important to know
If you look into the source code, you can see that all "Serial.xxxx" calls are surrounded by
```cpp
#ifndef MAX_SAVE
	<some code here>
#endif
```
As we want to achieve maximum power savings, the Serial port **MUST NOT** be initialized. Because not only does the hardware of the Serial port consume energy, FreeRTOS is as well starting a task running in the background (and never sleeps), that prevents the MCU from sleeping.

----
However, while testing this application, you might want to get some debug output. To enable the debug output, comment out the line
```cpp
#define MAX_SAVE
```
in the file `main.h`.

----

#### The Arduino setup() function
The first thing in `setup()` is to create the `taskEvent` semaphore that will later hold the `loopTask` in sleep mode.    
```cpp
	// Create the LoRaWan event semaphore
	taskEvent = xSemaphoreCreateBinary();
	// Initialize semaphore
	xSemaphoreGive(taskEvent);
```
Then after some GPIO setup, we call the function that initializes the SX1262 transceiver. More about that in the next paragraph. At the end of the setup() function you can find the code lines, that take the `taskEvent` semaphore and therefor blocks the main loop from executing
```cpp
	// Take the semaphore so the loop will go to sleep until an event happens
	xSemaphoreTake(taskEvent, 10);
```

#### The LoRaWan setup function `initLoRaWan()`
In the `initLoRaWan()` function the first thing is to create the semaphore that will later hold the `loraTask` in sleep mode.
```cpp
	// Create the LoRaWan event semaphore
	loraEvent = xSemaphoreCreateBinary();
	// Initialize semaphore
	xSemaphoreGive(loraEvent);
```
Then the usual setup calls to initiate the LoRaWan library are done. Details about this can be found in the other examples. The more important parts are at the end of the function.
In normal mode, the SX1262 interrupt would call a function inside the SX126x-Arduino library. This interrupt handler sets a flag, that is used by the library function `Radio.IrqProcess` to handle different LoRa events. But we do not want to poll this interrupt handler flag in an endless loop, because that would prevent the MCU from going into sleep. The workaround here is to _**steal**_ the interrupt handler from the SX126x-Arduino library and point it to our own interrupt handler.
```cpp
	// In deep sleep we need to hijack the SX126x IRQ to trigger a wakeup of the nRF52
	attachInterrupt(PIN_LORA_DIO_1, loraIntHandler, RISING);
```
This interrupt handler `loraIntHandler` will be explained later.
Next is to start the `loraTask`. 
```cpp
	// Start the task that will handle the LoRaWan events
#ifndef MAX_SAVE
	Serial.println("Starting LoRaWan task");
#endif
	if (!xTaskCreate(loraTask, "LORA", 2048, NULL, TASK_PRIO_LOW, &loraTaskHandle))
	{
		return -4;
	}
```
And then we can start the LoRaWAN join process.
```cpp
	// Start Join procedure
#ifndef MAX_SAVE
	Serial.println("Start network join request");
#endif
	lmh_join();
```
As you might have seen, here we do not block the `loraTask` immediately by taking the `loraEvent` semaphore. This is because until the _**join network process**_ is finished, it is required to keep the `loraTask` running.

#### The function that wakes up the `loraTask`.
If the SX1262 sets its interrupt to inform the MCU about an event (can be RX, TX or error events), the function `loraIntHandler` is called.
This function gives (frees) the `loraEvent` semaphore, which wakes up the `loraTask` and lets the task handle the event.
```cpp
void loraIntHandler(void)
{
	// SX126x set IRQ
	if (loraEvent != NULL)
	{
		// Wake up LoRa task
		xSemaphoreGive(loraEvent);
	}
}
```

#### The task `loraTask` to handle LoRa events
The task is quite simple. After starting the task it will run in an endless loop
```cpp
	while (1)
	{
	}
```
Just for some info, once the node has joined the LoRaWAN network, we switch off the blue LED
```cpp
		if (lmh_join_status_get() == LMH_SET)
		{ // Switch off the indicator lights
			digitalWrite(LED_CONN, LOW);
		}
```
And next the `loraTask` will go to sleep and wait for the `loraEvent` semaphore be given by the above `loraIntHandler`. Once the `loraEvent` semaphore is available we call `Radio.IrqProcessAfterDeepSleep()` to check what event the SX1262 has reported and to handle it.
```cpp
		// Only if semaphore is available we need to handle LoRa events.
		// Otherwise we sleep here until an event occurs
		if (xSemaphoreTake(loraEvent, portMAX_DELAY) == pdTRUE)
		{
			// Switch off the indicator lights
			digitalWrite(LED_CONN, HIGH);

			// Handle Radio events with special process command!!!!
			Radio.IrqProcessAfterDeepSleep();
		}
```
Handling the LoRa events is done in the functions `lorawan_has_joined_handler()`, `lorawan_rx_handler()`, `lorawan_confirm_class_handler()`, ... that are similar to the same functions in the "normal" LoRaWAN example.     
There are only a few differences:    
**lorawan_has_joined_handler()**    
After successfully joining the LoRaWAN network, we start the timer that will wake up the `loopTask` every 2 minutes to send a status messages.
```cpp
	// Now we are connected, start the timer that will wakeup the loop frequently
	taskWakeupTimer.begin(SLEEP_TIME, periodicWakeup);
	taskWakeupTimer.start();
```
**lorawan_rx_handler()**    
If we received a downlink package from the LoRaWAN server, we wake up the `taskLoop` that will handle the data package.
```cpp
		eventType = 0;
		// Notify task about the event
		if (taskEvent != NULL)
		{
#ifndef MAX_SAVE
			Serial.println("Waking up loop task");
#endif
			xSemaphoreGive(taskEvent);
		}
```
Beside of giving the `taskEvent` semaphore, we set as well the flag `eventType = 0;` which tells the `taskLoop` that it woke up because a data package has arrived.    
**All LoRa event handler functions**    
In every LoRa event handler function, you can find the lines
```cpp
	// Send LoRa handler back to sleep
	xSemaphoreTake(loraEvent, 10);
```
which takes the `loraEvent` semaphore. That causes the `loraTask` to go back to sleep again until the next event happens.

### The function that wakes up the `loopTask` frequently
As already mentioned, the `loopTask` sleeps until either a downlink package is received, or wakes up every 2 minutes to send a status package.    
After the node has joined the LoRaWAN network, a timer is initialized (see above), that calls every 2 minutes the function `periodicWakeup()`. In this function, the `taskEvent` semaphore is given, which will wake up the `loopTask`.
```cpp
void periodicWakeup(TimerHandle_t unused)
{
	// Switch on blue LED to show we are awake
	digitalWrite(LED_CONN, HIGH);
	eventType = 1;
	// Give the semaphore, so the loop task will wake up
	xSemaphoreGiveFromISR(taskEvent, pdFALSE);
}
```
Here in addition to give the semaphore, we set in addition a flag `eventType = 1;` to inform the `taskLoop` that it was woken up because of the periodic alarm.    

### The `loopTask` (or Arduino loop() function)
The loop task is quite simple. First we switch of the indicator LED to show that the nRF52 is going to sleep
```cpp
void loop(void)
{
	// Switch off blue LED to show we go to sleep
	digitalWrite(LED_BUILTIN, LOW);

```
Next we call `xSemaphoreTake()` with `portMAX_DELAY` which puts the task into sleep until the semaphore is available
```cpp
	// Sleep until we are woken up by an event
	if (xSemaphoreTake(taskEvent, portMAX_DELAY) == pdTRUE)
	{
```
If the semaphore is given either by the periodic alarm or the SX1262 has received a data package from the LoRaWAN server, the indicator LED is switched on
```cpp
		// Switch on blue LED to show we are awake
		digitalWrite(LED_BUILTIN, HIGH);
		delay(500); // Only so we can see the blue LED
```
Then the `eventType` is checked to take actions according to the event. In case a downlink package was received, the package should be handled. For simplicity I skipped that part in this example.
```cpp
		// Check the wake up reason
		switch (eventType)
		{
		case 0: // Wakeup reason is package downlink arrived
#ifndef MAX_SAVE
			Serial.println("Received package over LoRaWan");
#endif
			if (rcvdLoRaData[0] > 0x1F)
			{
#ifndef MAX_SAVE
				Serial.printf("%s\n", (char *)rcvdLoRaData);
#endif
			}
			else
			{
#ifndef MAX_SAVE
				for (int idx = 0; idx < rcvdDataLen; idx++)
				{
					Serial.printf("%X ", rcvdLoRaData[idx]);
				}
				Serial.println("");
#endif
			}

			break;
```
If the wake event was triggered by the periodic timer, a LoRaWan package is sent to the LoRaWAN server
```cpp
		case 1: // Wakeup reason is timer
#ifndef MAX_SAVE
			Serial.println("Timer wakeup");
#endif
			/// \todo read sensor or whatever you need to do frequently

			// Send the data package
			if (sendLoRaFrame())
			{
#ifndef MAX_SAVE
				Serial.println("LoRaWan package sent successfully");
#endif
			}
			else
			{
#ifndef MAX_SAVE
				Serial.println("LoRaWan package send failed");
				/// \todo maybe you need to retry here?
#endif
			}

			break;
```
After the event is handled, the `taskLoop` goes back to sleep.
```cpp
		// Go back to sleep
		xSemaphoreTake(taskEvent, 10);
	}
}
```

## Notes
This code is written as an example and is not perfect in all parts. The goal was to show how to minimize the power consumption of a system that is based on a nRF52 and a SX1262.